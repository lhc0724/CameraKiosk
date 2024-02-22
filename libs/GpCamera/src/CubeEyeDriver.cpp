#include <iostream>
#include <cmath>

#include "CubeEyeDriver.h"

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    uptrCubeEyeContext gCubeEyeContext;
    uptrCubeEyeEvtListener gCubeEyeEvtListener;

    int CubeEyeContext::SearchSources()
    {
        this->_sourceList = search_camera_source();
        if (this->_sourceList.get() != nullptr)
        {
            return this->_sourceList->size();
        }

        return 0;
    }

    void CubeEyeContext::PrintSourceList()
    {
        if (_sourceList.get() != nullptr)
        {
            int i = 0;
            for (auto source : (*_sourceList))
            {
                std::cout << "\nCam[" << i << "] info" << std::endl;
                std::cout << "  source name : " << source->name() << std::endl;
                std::cout << "  serialNumber : " << source->serialNumber() << std::endl;
                std::cout << "  uri : " << source->uri() << std::endl;
                i++;
            }
        }
    }

    /* select Camera and create cam object */
    eCubeEyeRes CubeEyeContext::SelectCamera(uint16_t CamNum)
    {
        if(this->_sourceList.get() == nullptr)
        {
            return eCubeEyeRes::no_such_device;
        }

        try
        {
            this->_camSource = this->_sourceList->at(CamNum);
            this->_camera = create_camera(_camSource);
            this->_propMgr.makeDefaultPropList();

            std::cout << "[CubeEye] Selected: " << _camSource->name() << "\n";
        }
        catch (const std::exception &e)
        {
            // vector.at() range over error
            e.what();
            return eCubeEyeRes::invalid_parameter;
        }

        return eCubeEyeRes::success;
    }

    eCubeEyeRes CubeEyeContext::PreparedCamera(EventListener *listener)
    {
        if(this->_camera.get() == nullptr)
        {
            return eCubeEyeRes::no_such_device;
        }

        //this->_camera = create_camera(_camSource);

        add_prepared_listener(listener);
        this->_camera->addSink(listener);

        if (_camera->prepare() == eCubeEyeRes::success)
        {
            _propMgr.setDefaultProperties(this->_camera.get());
            return eCubeEyeRes::success;
        }

        return eCubeEyeRes::fail;
    }

    eCubeEyeRes CubeEyeContext::CameraRun(FrameType wantedFrame = FrameType::FrameType_Unknown)
    {

        if (wantedFrame == FrameType::FrameType_Unknown)
        {
            return eCubeEyeRes::invalid_data_type;
        }

        if (_camera.get() != nullptr)
        {
            return _camera->run(wantedFrame);
        }

        // not selected Camera
        return eCubeEyeRes::empty;
    }

    eCubeEyeRes CubeEyeContext::CameraStop()
    {
        if(this->_camera.get() != nullptr)
        {
            return this->_camera->stop();
        }
        return eCubeEyeRes::no_such_device;
    }

    void CubeEyeContext::CameraEnd()
    {
        if(this->_camera.get() != nullptr)
        {
            this->_camera->release();
            destroy_camera(this->_camera);
            this->_camera.reset();
        }
    }

    eCubeEyeRes CubeEyeContext::CameraSetProp(sptrCamProp prop)
    {
        if(this->_camera.get() == nullptr)
        {
            return eCubeEyeRes::invalid_parameter;
        }

        return this->_camera->setProperty(prop);
    }

    void CubeEyeContext::CameraInitialize()
    {
        sptrCamResProp _prop;

        if (_camera != nullptr)
            _propMgr.setDefaultProperties(_camera.get());
    }

    eCubeEyeRes CubeEyeContext::SetupFoVScale()
    {
        if (this->_camera.get() == nullptr)
        {
            return eCubeEyeRes::no_such_device;
        }

        auto _fov = this->_camera->fov(0);
        
        //convert degree to radian
        double _ConvRadH = std::get<0>(_fov) * (M_PI / 180.0);
        double _ConvRadV = std::get<1>(_fov) * (M_PI / 180.0);

        this->_fovScaleH = 2 * tan(_ConvRadH / 2.0);
        this->_fovScaleV = 2 * tan(_ConvRadV / 2.0);

        // std::cout << "FoV: " << std::get<0>(_fov) << "(H) * " << std::get<1>(_fov) << "(V)" << std::endl;
        // std::cout << "Fov Scaled: " << this->_fovScaleH << " * " << this->_fovScaleV << std::endl;
        return eCubeEyeRes::success;
    }

    /* event listener functions */
    void EventListener::onCubeEyeCameraState(const ptrCamSource source, eCubeEyeState state)
    {
        if (state == eCubeEyeState::Prepared)
        {
            //큐 초기화
            this->mFrameListQ = std::queue<sptrFrameList>();
            this->_frameCnt = 0;
        }
        else if (state == eCubeEyeState::Running)
        {
        }
        else if (state == eCubeEyeState::Stopped)
        {
            //this->StopFlag = false;
        }
    }

    void EventListener::onCubeEyeCameraError(const ptrCamSource source, eCubeEyeError error)
    {
        std::cout << __FUNCTION__ << __LINE__ << source->uri().c_str() << error << std::endl;
    }

    void EventListener::onCubeEyeFrameList(const ptrCamSource source, const sptrFrameList &frames)
    {
        if (!_readFlag)
        {
            this->_frameCnt = 0;
        }
        else
        {
            this->_frameCnt++;

            if (this->_frameCnt > FILTER_CNT && this->mFrameListQ.size() < MAX_FRAME_CNT)
            {
                auto _copied_frame_list = meere::sensor::copy_frame_list(frames);
                if (_copied_frame_list)
                {
                    // std::lock_guard<std::mutex> guard(this->queueLocker);
                    this->mFrameListQ.push(std::move(_copied_frame_list));
                }
            }
        }
    }

    void EventListener::onCubeEyeCameraPrepared(const ptrCamera camera)
    {
        std::cout << "[CubeEye] " << __FUNCTION__ << ": source(" << camera->source()->uri() << ")\n";
    }

    void EventListener::setReadFlag(bool value)
    {
        this->_readFlag = value;
    }

    uptrCubeEyeContext processMakeContext()
    {
        uptrCubeEyeContext context = std::make_unique<CubeEyeContext>();

        if (!context->SearchSources())
        {
            return nullptr;
        }

        if(context->SelectCamera(0) != eCubeEyeRes::success)
        {
            return nullptr;
        }

        if(context->SetupFoVScale() != eCubeEyeRes::success)
        {
            return nullptr;
        }

        return context;
    }

    NS_CUBEEYE_END
}