#include <GpCamera/CubeEyeDriver.h>
#include <GpCamera/FrameParser.h>
#include <Util/Socket.h>
#include "DetectEvent.h"

#include <chrono>
#include <thread>
#include <future>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <string>

using namespace CamDriver;
namespace Lidar = CamDriver::CubeEye;

using FrameListDep = Lidar::FrameListDep;
using PixelInfoList = Lidar::PixelInfoList;

namespace CameraKiosk
{
    FrameListDep getRoiFrameData(Lidar::EventListener *ceListener, Lidar::ROIFrame *roi)
    {
        int callCnt = 0;

        FrameListDep avgDepthArr;
        while(1)
        {
            if(!ceListener->mFrameListQ.empty())
            {
                auto frames = std::move(ceListener->mFrameListQ.front());
                ceListener->mFrameListQ.pop();

                for(size_t idx = 0; idx < frames->size(); idx++)
                {
                    auto frame = frames->at(idx);
                    if(frame->frameType() == Lidar::FrameType::FrameType_Depth)
                    {
                        callCnt++;

                        auto sptrBasicFrame = Lidar::frame_cast_basic16u(frame);
                        auto frameData = sptrBasicFrame->frameData();
                        avgDepthArr = Lidar::calcDepthFrameAvg(avgDepthArr, frameData, roi, callCnt);
                    }
                }
            }
            else
            {
                //std::cout << "[CubeEye] queue is empty " << std::endl;
                break;
            }
        }

        return avgDepthArr;
    }

    uint16_t calcObjectDistance(Lidar::PixelInfoList pixels, double baseDepth, int direction)
    {
        auto allowRangeBottom = baseDepth - Lidar::AllowableDepthRange;
        auto allowRangeTop = baseDepth + Lidar::AllowableDepthRange;

        if(direction == 0)
        {
            // asix-Y sort
            std::sort(pixels.begin(), pixels.end(), [](Lidar::PixelInfo &a, Lidar::PixelInfo &b) {
                return a.coordY > b.coordY;
            });
        }else {
            // asix-X sort
            std::sort(pixels.begin(), pixels.end(), [](Lidar::PixelInfo &a, Lidar::PixelInfo &b) {
                return a.coordX > b.coordX;
            });
        }

        Lidar::PixelInfo tmp = pixels.front();
        uint16_t distance = 0;
        uint16_t counter = 0;
        
        for (auto it : pixels)
        {
            auto calbDep = Lidar::calibrationDedpth(it, Lidar::gCubeEyeContext->getFovScaleH(), Lidar::gCubeEyeContext->getFovScaleV());
            if(calbDep < allowRangeBottom) {
                continue;
            }

            if(calbDep > allowRangeTop) {
                continue;
            }

            counter++;
            if (direction == 0)
            {
                // calc asix-X distance
                if (it.coordY != tmp.coordY)
                {
                    tmp = it;
                    if (distance < counter)
                    {
                        distance = counter;
                    }
                    counter = 0;
                }
            }
            else
            {
                // calc asix-Y distance
                if (it.coordX != tmp.coordX)
                {
                    tmp = it;
                    if (distance < counter)
                    {
                        distance = counter;
                    }
                    counter = 0;
                }
            }
        }

        return distance;
    }

    void DetectEvent::notify(eKioskResult res, eKioskStates nextState = eKioskStates::NONE)
    {
        this->_listener->update(res, nextState);
    }

    void requestGPCapture(std::string fileName)
    {
        auto camCnt = CamDriver::gSharedCamContext->getCamCount();
        for (size_t i = 0; i < camCnt; i++)
        {
            // char buffer[128];
            std::string saveFile("Cam" + std::to_string(i));
            saveFile.append("-");
            // saveFile.insert(4, std::to_string(i));

            saveFile.append(fileName);
            CamDriver::gSharedCamContext->requestCapture({CamDriver::eGpCapMode::TO_MEMORY, {saveFile, i}});
        }
    }

    void DetectEvent::onEnter()
    {
        //clear json object
        this->infoJsonObj.clear();

        //CubeEye Lidar ReadFrame
        Lidar::gCubeEyeEvtListener->setReadFlag(true);

        //create capture info
        time_t currTime = time(nullptr);
        struct tm *locTime = localtime(&currTime);
        char fileName[128];

        strftime(fileName, sizeof(fileName), "%Y-%m-%d-%X", locTime);
        auto fGphoto = std::async(requestGPCapture, std::string(fileName).append(".jpg"));

        this->infoJsonObj["fileId"] = fileName;

        //저울 thread
        BalancerAND::setup();
        try
        {
            this->_fObjWeight = std::async(BalancerAND::getWeightValue);
        }
        catch(std::exception &e)
        {
            e.what();
        }

        notify(eKioskResult::CONTINUE, getId());
    }

    void DetectEvent::onEvent()
    {
        //make ROI
        Lidar::ROIFrame roi(Lidar::DefaultROI_X, Lidar::DefaultROI_Y);
        Lidar::PixelInfoList pixels;
        FrameListDep depthList;

        //get Fov Scales
        auto hScale = Lidar::gCubeEyeContext->getFovScaleH();
        auto vScale = Lidar::gCubeEyeContext->getFovScaleV();

        std::this_thread::sleep_for(std::chrono::milliseconds(750));

        // stop read frame
        Lidar::gCubeEyeEvtListener->setReadFlag(false);

        // make base pixel datas
        try
        {
            /* code */
            depthList = getRoiFrameData(Lidar::gCubeEyeEvtListener.get(), &roi);
            pixels = Lidar::convertDpethToPixelInfos(&depthList, &roi);
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            notify(eKioskResult::TRANSITION, eKioskStates::EXTRACT);
            return ;
        }
        
        /* Object Detect algorithm start */ 
        auto areaData = Lidar::getDetectArea(pixels.begin(), pixels.end());
        auto newFrameSt = std::get<0>(areaData);
        auto newFrameEd = std::get<1>(areaData);

        try
        {
            auto availablePixels = Lidar::getObjectInDetectArea(newFrameSt, newFrameEd);

            // depth loging
            // std::ofstream infoFile("depthLog.txt");
            // if (infoFile.is_open())
            // {
            //     for(auto it:availablePixels)
            //     {
            //         infoFile << "[X: " << std::to_string(it.coordX) << " Y: " << std::to_string(it.coordY) << "]\n";
            //         infoFile << "Depth: " << std::to_string(it.getDepth()) << " Calibration: " << std::to_string(Lidar::calibrationDedpth(it, hScale, vScale)) << "\n\n";
            //     }
            //     infoFile.close();
            // }

            //copy vector
            Lidar::PixelInfoList sorter;
            sorter.insert(sorter.begin(), availablePixels.begin(), availablePixels.end());

            std::sort(sorter.begin(), sorter.end(), [](Lidar::PixelInfo &a, Lidar::PixelInfo &b)
                      { return a.getDepth() > b.getDepth(); });
            auto medianIter = sorter.at(sorter.size() / 2);
            double calibMidDepth = Lidar::calibrationDedpth(medianIter, hScale, vScale);

            auto fDistX = std::async(calcObjectDistance, availablePixels, calibMidDepth, 0);
            auto fDistY = std::async(calcObjectDistance, availablePixels, calibMidDepth, 1);

            uint16_t distanceX = fDistX.get();
            uint16_t distanceY = fDistY.get();
            auto weight = _fObjWeight.get();

            double hDist = (hScale * calibMidDepth) / Lidar::CamWidth;
            double vDist = (vScale * calibMidDepth) / Lidar::CamHeight;

            Json::Value dist;
            dist["X"] = distanceX * hDist;
            dist["Y"] = distanceY * vDist;

            infoJsonObj["depth"] = Lidar::PropValDepthOffset - calibMidDepth;
            infoJsonObj["distance"] = dist;
            infoJsonObj["weight"] = weight;

            std::string jsonStr = infoJsonObj.toStyledString();
            if (!Util::gSocketHandler->sendReqeust(jsonStr, Util::ESocketMessageType::JSON))
            {
                JsonBuilder::writeJson(infoJsonObj, infoJsonObj["fileId"].asString().append(".json"));
                std::cout << "failed send message\n";
            }
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        notify(eKioskResult::TRANSITION, eKioskStates::EXTRACT);
    }
};
