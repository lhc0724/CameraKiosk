#include "FrameParser.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    size_t ROIFrame::getRoiPixelSize()
    {
        return getRoiDistX() * getRoiDistY();
    }

    size_t ROIFrame::getRoiDistX()
    {
        auto [xBegin, xEnd] = this->axisX;

        return (xEnd - xBegin);
    }

    size_t ROIFrame::getRoiDistY()
    {
        auto [yBegin, yEnd] = this->axisY;

        return (yEnd - yBegin);
    }

    std::tuple<Coordinate, Coordinate> ROIFrame::getRoi()
    {
        return std::make_tuple(getRoiX(), getRoiY());
    }

    Coordinate ROIFrame::getRoiX()
    {
        return this->axisX;
    }

    Coordinate ROIFrame::getRoiY()
    {
        return this->axisY;
    }

    uint16_t PixelInfo::getDepth()
    {
        return this->depthVal;
    }

    std::pair<size_t, size_t> PixelInfo::getCoord()
    {
        return {this->coordX, this->coordY};
    }

    FrameListDep calcDepthFrameAvg(FrameListDep prevArr, CubeEyeList *newDataArr, ROIFrame *roi, int calcCnt)
    {

        FrameListDep newAvgArr;
        auto [xBegin, xEnd] = roi->getRoiX();
        auto [yBegin, yEnd] = roi->getRoiY();

        bool makeFlag = prevArr.empty();
        size_t idx = 0;

        // framedata[307200] (camWidth * camHeight), [W]: 0 ~ 639 [H]: 0 ~ 479
        // 좌표 찾기 ex) [10, 43] = data[W(640) * 43] + 10
        for (size_t j = yBegin; j < yEnd; j++)
        {
            auto pixelPoint = (CamWidth * j);

            for (size_t i = xBegin; i < xEnd; i++)
            {
                if (makeFlag)
                {
                    auto tmpVal = newDataArr->at(pixelPoint + i);
                    newAvgArr.push_back(tmpVal);
                }
                else
                {
                    uint16_t avgValue = 0;
                    if(newDataArr->at(pixelPoint + i) != 0)
                        avgValue = (uint16_t)(((prevArr.at(idx) * (calcCnt - 1)) + newDataArr->at(pixelPoint + i)) / calcCnt);
                        
                    newAvgArr.push_back(avgValue);
                }

                idx++;
            }
        }

        return newAvgArr;
    }

    PixelInfoList convertDpethToPixelInfos(FrameListDep *orgArr, ROIFrame *roi)
    {
        PixelInfoList infoList;
        auto [xBegin, xEnd] = roi->getRoiX();
        auto [yBegin, yEnd] = roi->getRoiY();

        size_t idx = 0;
        for(size_t y = yBegin; y < yEnd; y++)
        {
            for(size_t x = xBegin; x < xEnd; x++)
            {
                infoList.push_back(PixelInfo(orgArr->at(idx), {x, y}));
                idx++;
            }
        }

        return infoList;
    }

    double calibrationDedpth(PixelInfo info, double horizScale, double verticalScale)
    {
        if(info.getDepth() == 0) {
            return 0;
        }

        int xDiff = info.coordX - ZeroPoint.first;
        int yDiff = info.coordY - ZeroPoint.second;

        if (xDiff < 0)
            xDiff *= -1;
        if (yDiff < 0)
            yDiff *= -1;

        auto hDist = ((horizScale * PropValDepthOffset) / CamWidth) * xDiff;
        auto vDist = ((verticalScale * PropValDepthOffset) / CamHeight) * yDiff;

        double bottomLength = pow(hDist, 2) + pow(vDist, 2);
        double baseHypotenuse = sqrt(pow(PropValDepthOffset, 2) + bottomLength);

        double hypotenusRatio = info.getDepth() / baseHypotenuse;
        double calibDepth = PropValDepthOffset * hypotenusRatio;

        return calibDepth;
    }

    std::tuple<PixelInfoList::iterator, PixelInfoList::iterator> getDetectArea(PixelInfoList::iterator first, PixelInfoList::iterator end)
    {
        PixelInfoList::iterator areaStart = end;
        PixelInfoList::iterator areaEnd = end;

        for(auto iter = first; iter != end; ++iter)
        {
            if(iter->getDepth() == 0)
            {
                if(areaStart == end && areaEnd == end)
                {
                    areaStart = iter;
                    areaEnd = iter;

                    continue;
                }

                auto coordOper = iter->coordX * iter->coordY;
                if(areaStart->coordX * areaStart->coordY > coordOper)
                {
                    areaStart = iter;
                }

                if(areaEnd->coordX * areaEnd->coordY < coordOper)
                {
                    areaEnd = iter;
                }
            }
        }

        return std::make_tuple(areaStart, areaEnd);
    }

    PixelInfoList getObjectInDetectArea(PixelInfoList::iterator areaSt, PixelInfoList::iterator areaEnd)
    {
        PixelInfoList detectPixels;
        for (auto iter = areaSt; iter <= areaEnd; iter++)
        {
            if (iter->coordX < areaSt->coordX || iter->coordY < areaSt->coordY)
            {
                continue;
            }

            if (iter->coordX > areaEnd->coordX || iter->coordY > areaEnd->coordY)
            {
                continue;
            }

            auto pixelDepth = iter->getDepth();

            if(pixelDepth != 0)
            {
                detectPixels.push_back(*iter);
            }
        }

        return detectPixels;
    }

    NS_CUBEEYE_END
}