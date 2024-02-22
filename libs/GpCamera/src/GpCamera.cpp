#include <filesystem>

#include "GpCamera.h"
#include <Util/Socket.h>

namespace fs = std::filesystem;
namespace CamDriver
{
    sptrCamContext gSharedCamContext;

    eGpResult captureToMemeory(Camera *camera, GPContext *context, const char **picture, unsigned long int *size);
    eGpResult captureToFile(Camera *camera, GPContext *context, std::string fileName);
    int waitCaptureEvent(Camera *camera, GPContext *context);

    /* CamEventQueue */
    void CamEventQueue::queueInit()
    {
        std::lock_guard(this->_locker);
        while(!this->_mEventQ->empty())
            _mEventQ->pop();
        
    }

    bool CamEventQueue::eventPush(GpCaptureReq req)
    {
        bool result = false;

        std::lock_guard(this->_locker);
        if(this->_mEventQ->size() < MAX_Q_SIZE)
        {
            this->_mEventQ->push(req);
            result = true;
        }

        return result;
    }

    GpCaptureReq CamEventQueue::getFront()
    {
        GpCaptureReq evtInfo = {eGpCapMode::NO_EVENT, {"", 0}};

        std::lock_guard(this->_locker);
        if(!this->_mEventQ->empty())
        {
            evtInfo = this->_mEventQ->front();
            this->_mEventQ->pop();
        }

        return evtInfo;
    }

    /* CamEventQueue End */

    /* GpCamera */
    eGpResult GpCamera::initCamera()
    {
        return static_cast<eGpResult>(gp_camera_new(&this->_cam));
    }

    eGpResult GpCamera::openCamera(GPContext *context, GPPortInfoList *portList, CameraAbilitiesList *abList)
    {
        GpExceptionThrower exThrower;

        if (this->_cam == nullptr)
        {
            return eGpResult::ERR_BAD_PARAM;
        }

        /* Camera lookup the model / driver / abilities setup */
        auto model = gp_abilities_list_lookup_model(abList, this->_name.c_str());
        exThrower.checkOnErrorOrg(model);
        exThrower.checkOnErrorOrg(gp_abilities_list_get_abilities(abList, model, &this->_abilities));
        exThrower.checkOnErrorOrg(gp_camera_set_abilities(this->_cam, this->_abilities));

        /* associate the camera with the specified port */
        auto port = gp_port_info_list_lookup_path(portList, this->_value.c_str());
        exThrower.checkOnErrorOrg(port);

        /* read port info and setup */
        exThrower.checkOnErrorOrg(gp_port_info_list_get_info(portList, port, &this->_portInfo));
        exThrower.checkOnErrorOrg(gp_camera_set_port_info(this->_cam, this->_portInfo));
        exThrower.checkOnErrorOrg(gp_camera_get_config(this->_cam, &this->widget, context));

        return eGpResult::SUCCESS;
    }
    /* GpCamera End */

    /* CamGPContext */

    /* this function that brings up the abilities of the camera */
    void CamGPContext::loadAbilitiesList()
    { 
        GpExceptionThrower exThrower;

        auto res = gp_abilities_list_new(&this->abilities);
        exThrower.checkOnErrorOrg(res);

        res = gp_abilities_list_load(this->abilities, this->_context);
        exThrower.checkOnErrorOrg(res);

    }

    /* this function that brings up the COM Port infomation of the camera */
    void CamGPContext::loadPortInfoList()
    {
        GpExceptionThrower exThrower;

        auto res = gp_port_info_list_new(&portInfoList);
        exThrower.checkOnErrorOrg(res);

        res = gp_port_info_list_load(portInfoList);
        exThrower.checkOnErrorOrg(res);

        res = gp_port_info_list_count(portInfoList);
        exThrower.checkOnErrorOrg(res);
    }

    eGpResult CamGPContext::initContext()
    {
        GpExceptionThrower exThrower;

        // create context, camera list object
        this->_context = gp_context_new();
        auto result = gp_list_new(&this->_camList);

        // Err check
        exThrower.checkOnErrorOrg(result);

        return eGpResult::SUCCESS;
    }

    int CamGPContext::detectCameras()
    {
        int camCnt = 0;

        if(this->_context == nullptr || this->_camList == nullptr)
        {
            return -1;
        }

        // Camera AutoDetecting
        gp_list_reset(this->_camList);
        camCnt = gp_camera_autodetect(this->_camList, this->_context);

        return camCnt;
    }

    eGpResult CamGPContext::createCameras(int camCnt)
    {
        const char *name, *value;
        GpExceptionThrower th;

        if(this->_context == nullptr || this->_camList == nullptr)
        {
            return eGpResult::ERR;
        }

        this->loadAbilitiesList();
        this->loadPortInfoList();

        for(int i = 0; i < camCnt; i++)
        {
            gp_list_get_name(this->_camList, i, &name);
            gp_list_get_value(this->_camList, i, &value);

            this->_cams.push_back(GpCamera(std::string(name), std::string(value)));
        }

        for (size_t i = 0; i < this->_cams.size(); i++)
        {
            auto res = this->_cams.at(i).initCamera();
            th.checkOnError(res);

            this->_cams.at(i).openCamera(this->_context, this->portInfoList, this->abilities);
        }

        return eGpResult::SUCCESS;
    }
    void CamGPContext::printCamerasInfo()
    {
        std::cout << "[gPhoto]" << std::endl;
        for(size_t i = 0; i < this->_cams.size(); i++)
        {
            std::cout << "        Device:" << this->_cams.at(i).getName() << std::endl;
            std::cout << "        PortInfo:" << this->_cams.at(i).getValue() << std::endl;
        }
    }

    bool CamGPContext::requestCapture(GpCaptureReq request)
    {
        auto result = this->_requestQ.eventPush(request);
        if (result)
        {
            std::unique_lock<std::mutex> lock(this->_contextMutex);
            this->_contextController.notify_one();
        }

        return result;
    }

    void CamGPContext::stopAsync()
    {
        this->requestCapture({eGpCapMode::EXIT, {"", 0}});
    }

    std::future<eGpResult> CamGPContext::runCaptureListener()
    {
        std::future<eGpResult> fRes;

        fRes = std::async([this]()
        {
            eGpResult res = eGpResult::ERR;

            char *pictureData;
            size_t picSize;

            while(1)
            {
                std::unique_lock<std::mutex> lock(this->_contextMutex);
                this->_contextController.wait(lock, [&]()
                                              { return !this->_requestQ.isEmpty(); });

                auto [evtType, evtInfo] = this->_requestQ.getFront();
                auto camera = this->_cams.at(evtInfo.second);

                if(evtType == eGpCapMode::EXIT || this->_asyncStop)
                {
                    res = eGpResult::SUCCESS;
                    break;
                }

                try
                {

                    switch (evtType)
                    {
                    case eGpCapMode::TO_FILE:
                        res = captureToFile(camera.getCamera(), this->_context, evtInfo.first);
                        if (res == eGpResult::SUCCESS)
                        {
                            std::cout << "[gPhoto] Success capture, Save as " << evtInfo.first << std::endl;
                        }
                        waitCaptureEvent(camera.getCamera(), this->_context);

                        break;
                    case eGpCapMode::TO_MEMORY:
                        res = captureToMemeory(camera.getCamera(), this->_context, (const char **)&pictureData, &picSize);

                        if (res != eGpResult::SUCCESS)
                        {
                            break;
                        }

                        // save to File
                        if (CameraKiosk::Util::gSocketHandler != nullptr)
                        {
                            auto reqResult = CameraKiosk::Util::gSocketHandler->sendReqeust(&pictureData, picSize, evtInfo.first);
                            if(!reqResult)
                            {
                                downloadFromMemory(&pictureData, picSize, evtInfo.first);
                            }
                        }

                        waitCaptureEvent(camera.getCamera(), this->_context);

                        break;
                    default:
                        break;
                    }
                }
                catch (GpException &e)
                {
                    std::cerr << "[gPhoto] Capture Error - " << e.what() << "\n";
                }
            }

            return res;
        });

        return fRes;
    }
    /* CamGPContext end */

    eGpResult downloadFromMemory(char **data, size_t size, std::string fileName)
    {
        eGpResult ret = eGpResult::SUCCESS;

        FILE *file;
        auto projDir = fs::current_path().parent_path();
        std::string captureDir("");

        // find capture directory
        for (auto const &dirEntry : fs::directory_iterator{projDir})
        {
            if (!dirEntry.path().filename().compare("capture"))
            {
                if (dirEntry.is_directory())
                {
                    captureDir = dirEntry.path().string() + "/";
                }
                break;
            }
        }

        if (captureDir == "") 
        {
            return eGpResult::ERR;
        }

        captureDir.append(fileName);

        file = fopen(captureDir.c_str(), "wb");
        if(file)
        {
            auto writeRet = fwrite(*data, size, 1, file);
            fclose(file);

            if(writeRet < 1)
            {
                //failed file written
                GpExceptionThrower(eGpResult::ERR_IO_WRITE);
            }
        }
        else
        {
            // failed open the file discriptor
            std::cout << "[gPhoto] Error - file open " << captureDir << " failed\n";
            GpExceptionThrower(eGpResult::ERR_IO_READ);
        }

        std::cout << "[gPhoto] Success capture, Save as " << fileName << std::endl;
        return ret;
    }

    eGpResult captureToMemeory(Camera *camera, GPContext *context, const char **picture, unsigned long int *size)
    {
        int retval;
        CameraFile *file;
        CameraFilePath path;

        /* NOP: This gets overridden in the library to /capt0000.jpg */
        strcpy(path.folder, "/");

        retval = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context);
        GpExceptionThrower thrower(retval);

        retval = gp_file_new(&file);
        thrower.checkOnErrorOrg(retval);

        retval = gp_camera_file_get(camera, path.folder, path.name,
                                    GP_FILE_TYPE_NORMAL, file, context);
        thrower.checkOnErrorOrg(retval);

        gp_file_get_data_and_size(file, picture, size);

        retval = gp_camera_file_delete(camera, path.folder, path.name,
                                       context);
        thrower.checkOnErrorOrg(retval);

        return eGpResult::SUCCESS;
    }

    eGpResult captureToFile(Camera *camera, GPContext *context, std::string fileName)
    {
        int fd;
        CameraFile *file;
        CameraFilePath path;
        CameraFileInfo info;

        auto projDir = fs::current_path().parent_path();
        std::string captureDir("");

        // find capture directory
        for (auto const &dirEntry : fs::directory_iterator{projDir})
        {
            if (!dirEntry.path().filename().compare("capture"))
            {
                if (dirEntry.is_directory())
                {
                    captureDir = dirEntry.path().string() + "/";
                }
                else
                {
                    fs::remove(dirEntry);
                }

                break;
            }
        }

        // if not found capture directory, create capture directory
        if (captureDir == "")
        {
            captureDir = projDir.string().append("/capture");
            if (fs::create_directory(captureDir))
            {
                captureDir.push_back('/');
            }
            else
            {
                return eGpResult::ERR;
            }
        }

        strcpy(path.folder, "/");

        // try capture
        auto res = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context);
        GpExceptionThrower thrower(res);

        // after capture, get capture file info from Camera
        gp_camera_file_get_info(camera, path.folder, path.name, &info, context);

        // download and save image file from Camera
        auto saveFile = captureDir.append(fileName);
        fd = open(saveFile.c_str(), O_CREAT | O_WRONLY, 0644);
        gp_file_new_from_fd(&file, fd);
        gp_camera_file_get(camera, path.folder, path.name, GP_FILE_TYPE_NORMAL, file, context);

        // file free
        gp_file_free(file);
        gp_camera_file_delete(camera, path.folder, path.name, context);

        return eGpResult::SUCCESS;
    }

    int waitCaptureEvent(Camera *camera, GPContext *context)
    {
        int result = 0; //success
        void *data = nullptr;

        CameraEventType evtType;
        GpExceptionThrower thrower;

        auto start = std::chrono::system_clock::now();

        // wait for capture complate
        while(1)
        {
            auto end = std::chrono::system_clock::now();
            std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            result = gp_camera_wait_for_event(camera, 0, &evtType, &data, context);

            if (evtType == GP_EVENT_CAPTURE_COMPLETE)
            {
                std::cout << "[gPhoto] Capture complete\n";
                break;
            }

            //time out
            if (diff.count() >= CAMERA_TIMEOUT)
            {
                std::cout << "[gPhoto] Event time-out\n";
                break;
            }
        }

        return result;
    }

};