#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include "CubeEyeWrapper.h"
#include "EcCubeEyeProps.h"
#include <tuple>

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    class PixelInfo;

    using Coordinate = std::pair<size_t, size_t>;
    using FrameListDep = std::vector<uint16_t>;
    using PixelInfoList = std::vector<PixelInfo>;

    const Coordinate DefaultROI_X = {350, 580};
    const Coordinate DefaultROI_Y = {200, 330};
    const Coordinate ZeroPoint = {320, 240}; // W x H (640, 480)
    const int AllowableDepthRange = 10;

    class ROIFrame
    {
    private:
        Coordinate axisX;
        Coordinate axisY;

    public:
        //default: Camera frame size
        ROIFrame() {
            this->axisX = {0, CamWidth};
            this->axisY = {0, CamHeight};
        }
        ROIFrame(Coordinate x, Coordinate y) : axisX(x), axisY(y) {}
        ~ROIFrame() {}

        // <0> = roiX, <1> = roiY
        std::tuple<Coordinate, Coordinate> getRoi();
        Coordinate getRoiX();
        Coordinate getRoiY();

        size_t getRoiPixelSize();
        size_t getRoiDistX();
        size_t getRoiDistY();
    };

    class PixelInfo
    {
    public:
        uint16_t depthVal;
        size_t coordX;
        size_t coordY;

        PixelInfo(uint16_t depth, Coordinate coord)
        {
            this->depthVal = depth;
            this->coordX = coord.first;
            this->coordY = coord.second;
        }
        PixelInfo() {}
        ~PixelInfo() {}

        Coordinate getCoord();
        uint16_t getDepth();
    };

    FrameListDep calcDepthFrameAvg(FrameListDep prevArr, CubeEyeList *newDataArr, ROIFrame *roi, int calcCnt);

    PixelInfoList convertDpethToPixelInfos(FrameListDep *orgArr, ROIFrame *roi);
    double calibrationDedpth(PixelInfo info, double horizScale, double verticalScale);
    std::tuple<PixelInfoList::iterator, PixelInfoList::iterator> getDetectArea(PixelInfoList::iterator first, PixelInfoList::iterator end);
    PixelInfoList getObjectInDetectArea(PixelInfoList::iterator areaSt, PixelInfoList::iterator areaEnd);

    NS_CUBEEYE_END
}

#endif