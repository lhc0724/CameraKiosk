#include <GpCamera/CubeEyeDriver.h>
#include "InitEvent.h"

#include <iostream>
#include <filesystem>
#include <thread>
#include <future>

namespace fs = std::filesystem;
namespace Lidar = CamDriver::CubeEye;

using namespace CamDriver;
namespace CameraKiosk
{
    Lidar::eCubeEyeRes setReadyCubeEye(Lidar::EventListener *listener)
    {
        Lidar::gCubeEyeContext = move(Lidar::processMakeContext());
        if(Lidar::gCubeEyeContext == nullptr)
        {
            return Lidar::eCubeEyeRes::fail;
        }

        return Lidar::gCubeEyeContext->PreparedCamera(listener);
    }

    int initGPhoto()
    {
        int camCnt = -1;

        try
        {
            /* code */
            std::cout << "\n[gPhoto] Library init... \n";
            auto sptrGp = CamDriver::gSharedCamContext;
            
            sptrGp->initContext();
            camCnt = sptrGp->detectCameras();

            if(camCnt > 0)
            {
                sptrGp->createCameras(camCnt);
                sptrGp->printCamerasInfo();
            }
        }
        catch(CamDriver::GpException& e)
        {
            std::cerr << e.what() << '\n';
            camCnt = -1;
        }

        return camCnt;
    }

    void initFilePaths()
    {
        auto projDir = fs::current_path().parent_path();

        std::string captureDir("");
        std::string jsonDir("");

        for (auto const &dirEntry : fs::directory_iterator{projDir})
        {
            if (!dirEntry.path().filename().compare("jsons"))
            {
                if (dirEntry.is_directory())
                {
                    jsonDir = dirEntry.path().string();
                }
                else
                {
                    fs::remove(dirEntry);
                }
            }

            if (!dirEntry.path().filename().compare("capture"))
            {
                if (dirEntry.is_directory())
                {
                    captureDir = dirEntry.path().string();
                }
                else
                {
                    fs::remove(dirEntry);
                }
            }
        }

        // if not found capture, jsons directory, create directory
        if (captureDir == "")
        {
            captureDir = projDir.string().append("/capture");
            fs::create_directory(captureDir);
        }

        if (jsonDir == "") 
        {
            jsonDir = projDir.string().append("/jsons");
            fs::create_directory(jsonDir);
        }
    }

    void InitEvent::notify(eKioskResult res, eKioskStates nextState = eKioskStates::NONE)
    {
        this->_listener->update(res, nextState);
    }

    void InitEvent::onEnter()
    {

        auto gpioRet = gpioSetup();
        auto serialRet = BalancerAND::setup();

        if(!gpioRet || serialRet < 0)
        {
            notify(eKioskResult::ERROR);
        }
        else
        {
            notify(eKioskResult::CONTINUE, getId());
        }

    }

    void InitEvent::onEvent()
    {
        Lidar::gCubeEyeEvtListener = std::make_unique<Lidar::EventListener>();

        int balRes = BalancerAND::setToZero();

        auto fCubeEyeRes = std::async(setReadyCubeEye, Lidar::gCubeEyeEvtListener.get());
        auto fGpRes = std::async(initGPhoto);
        initFilePaths();

        auto camCnt = fGpRes.get();
        auto ceRes = fCubeEyeRes.get();

        // std::cout << "gp: " << camCnt << ", cubeEye: " << ceRes << ", balancer: " << balRes << std::endl;

        if (camCnt > 0 && balRes == 0 && ceRes == Lidar::eCubeEyeRes::success)
        {
            Lidar::gCubeEyeEvtListener->setReadFlag(false);
            Lidar::gCubeEyeContext->CameraRun(Lidar::FrameType::FrameType_Depth);
            BalancerAND::balancerEnd();

            notify(eKioskResult::TRANSITION, eKioskStates::MOVE);
        }
        else if (camCnt < 0)
        {
            std::cout << "[gPhoto] Camera is not found" << std::endl;
            notify(eKioskResult::ERROR, getId());
        }
        else if (balRes < 0)
        {
            std::cout << "[AND Balancer] Serial setup failed" << std::endl;
            notify(eKioskResult::ERROR, getId());
        }
        else if (ceRes != Lidar::eCubeEyeRes::success)
        {
            std::cout << "[CubeEye] failed setup\n";
            notify(eKioskResult::ERROR, getId());
        }
    }
};