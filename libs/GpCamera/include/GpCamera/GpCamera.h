#ifndef GP_CAMERA_H
#define GP_CAMERA_H

#include <memory>
#include <vector>
#include <fcntl.h>
#include <cstring>

#include <gphoto2/gphoto2-camera.h>
#include "GpCommon.h"

#include <queue>
#include <mutex>
#include <future>

namespace CamDriver
{
    class GpCamera;
    class CamGPContext;

    using uptrCamContext = std::unique_ptr<CamGPContext>;
    using sptrCamContext = std::shared_ptr<CamGPContext>;

    using GpCamList = std::vector<GpCamera>;
    using GpCaptureReq = std::pair<eGpCapMode, std::pair<std::string, int>>;

    const int MAX_Q_SIZE = 50;
    const int CAMERA_TIMEOUT = 500;

    extern sptrCamContext gSharedCamContext;

    class GpCamera
    {
    private:
        std::string _name, _value;
        Camera *_cam;

        CameraAbilities _abilities;
        GPPortInfo _portInfo;
        CameraWidget *widget;

    public:
        GpCamera() : _name(""), _value("") {};
        GpCamera(std::string name, std::string value) : _name(name), _value(value) {};
        GpCamera(std::pair<std::string, std::string> args) : _name(args.first), _value(args.second) {};

        ~GpCamera() {}

        std::string getName() { return this->_name; };
        std::string getValue() { return this->_value; };
        Camera *getCamera() { return this->_cam; };

        eGpResult initCamera();
        eGpResult openCamera(GPContext *context, GPPortInfoList *portList, CameraAbilitiesList *abList);
    };

    class CamEventQueue
    {
    public:
        CamEventQueue(std::shared_ptr<std::queue<GpCaptureReq>> ptrQueue, std::mutex *mutex) : _mEventQ(ptrQueue) 
        {
            this->_masterMutex = mutex;
        }
        ~CamEventQueue() {}

        void queueInit();
        bool eventPush(GpCaptureReq req);
        GpCaptureReq getFront();
        bool isEmpty() { return this->_mEventQ->empty(); };

    private:
        std::mutex _locker;
        std::mutex *_masterMutex;
        std::condition_variable *_masterNotificator;

        std::shared_ptr<std::queue<GpCaptureReq>> _mEventQ;
    };

    class CamGPContext
    {
    private:
        CameraList *_camList;   //gphoto library list
        GPContext *_context;
        GpCamList _cams;

        GPPortInfoList *portInfoList;
        CameraAbilitiesList *abilities;

        CamEventQueue _requestQ;

        bool _asyncStop;
        std::mutex _contextMutex;
        std::condition_variable _contextController;

        void loadAbilitiesList();
        void loadPortInfoList();

    public:
        CamGPContext() : _requestQ(std::make_shared<std::queue<GpCaptureReq>>(), &this->_contextMutex)
        {
            this->_camList = nullptr;
            this->_context = nullptr;

            this->portInfoList = nullptr;
            this->abilities = nullptr;
        }
        ~CamGPContext() {}

        eGpResult initContext();

        int detectCameras();
        eGpResult createCameras(int camCnt);

        size_t getCamCount() { return _cams.size(); };
        void printCamerasInfo();
        bool requestCapture(GpCaptureReq request);

        std::future<eGpResult> runCaptureListener();
        void stopAsync();
    };

    eGpResult downloadFromMemory(char **data, unsigned long size, std::string fileName);
    int waitCamEvent(Camera *camera, int waitTime, GPContext *context);

};

#endif