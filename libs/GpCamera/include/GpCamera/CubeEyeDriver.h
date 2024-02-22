#ifndef LIDAR_DRIVER_H
#define LIDAR_DRIVER_H

#include "CubeEyeWrapper.h"
#include "EcCubeEyeProps.h"

#include <thread>
#include <queue>
#include <mutex>

namespace CamDriver
{
    NS_CUBEEYE_BEGIN
    using namespace meere::sensor;

    class CubeEyeContext;
    class EventListener;

    using uptrCubeEyeContext = std::unique_ptr<CubeEyeContext>;
    using uptrCubeEyeEvtListener = std::unique_ptr<EventListener>;

    const long unsigned int MAX_FRAME_CNT = 20;
    const int FILTER_CNT = 10;

    extern uptrCubeEyeContext gCubeEyeContext;
    extern uptrCubeEyeEvtListener gCubeEyeEvtListener;

    class EventListener : public CubeEyeSink, public PreparedListener
    {
    public:
        // sink events listener
        virtual std::string name() const
        {
            return std::string("S100D-EventListener");
        };
        virtual void onCubeEyeCameraState(const ptrCamSource source, eCubeEyeState state);
        virtual void onCubeEyeCameraError(const ptrCamSource source, eCubeEyeError error);
        virtual void onCubeEyeFrameList(const ptrCamSource source, const sptrFrameList &frames);

    public:
        virtual void onCubeEyeCameraPrepared(const ptrCamera camera);

    protected:
        static void ReadFrameProc(EventListener *thiz);

    public:
        EventListener() = default;
        virtual ~EventListener() = default;
        std::queue<sptrFrameList> mFrameListQ;
        std::mutex queueLocker;

        void setReadFlag(bool value);

    protected:
        bool _readFlag = false;
        std::thread _readFrameThd;
        int _frameCnt;
    };

    class CubeEyeContext
    {
    private:
        sptrSourceList _sourceList;
        sptrCamSource _camSource;
        sptrCamera _camera;
        PropManager _propMgr;

        double _fovScaleH = 0;
        double _fovScaleV = 0;

        void CameraInitialize();

    public:
        CubeEyeContext(): _propMgr()
        {
            _sourceList.reset();
            _camSource.reset();
            _camera.reset();
        };
        ~CubeEyeContext() {
            if(_camera != nullptr)
            {
                _camera->release();
                destroy_camera(this->_camera);
            }
        };

        int SearchSources();
        void PrintSourceList();

        eCubeEyeRes SetupFoVScale();
        eCubeEyeRes SelectCamera(uint16_t camNum);
        eCubeEyeRes PreparedCamera(EventListener *listener);

        eCubeEyeRes CameraSetProp(sptrCamProp prop);
        eCubeEyeRes CameraRun(FrameType wantedFrame);

        eCubeEyeRes CameraStop();
        void CameraEnd();
        double getFovScaleH() { return this->_fovScaleH; };
        double getFovScaleV() { return this->_fovScaleV; };
    };

    uptrCubeEyeContext processMakeContext();
    NS_CUBEEYE_END;
}

#endif