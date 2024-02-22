#ifndef CUBE_EYE_WRAPPER_H
#define CUBE_EYE_WRAPPER_H

#include <CubeEye.h>

#include <CubeEyeFrame.h>
#include <CubeEyeData.h>
#include <CubeEyeSink.h>
#include <CubeEyeBasicFrame.h>
#include <CubeEyeList.h>

#define NS_CUBEEYE_BEGIN  namespace CubeEye {
#define NS_CUBEEYE_END  }

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    // camera device 관련
    using sptrSourceList = meere::sensor::sptr_source_list;
    using sptrCamSource = meere::sensor::sptr_source;

    using ptrCamSource = meere::sensor::ptr_source;
    using CamSource = meere::sensor::CubeEyeSource;

    using ptrCamera = meere::sensor::ptr_camera;
    using sptrCamera = meere::sensor::sptr_camera;
    using Camera = meere::sensor::CubeEyeCamera;
    using FoV = meere::sensor::FoV;

    // camera properties 관련
    using sptrCamProp = meere::sensor::sptr_property;
    using sptrCamResProp = meere::sensor::result_property;
    using CubeEyeDataType = meere::sensor::CubeEyeData::DataType;

    // camera event 관련
    using eCubeEyeRes = meere::sensor::result;  // enum
    using eCubeEyeState = meere::sensor::State; // enum
    using eCubeEyeError = meere::sensor::Error; // enum

    using PreparedListener = meere::sensor::prepared_listener;
    using ptrPreparedListener = meere::sensor::ptr_prepared_listener;
    using FrameType = meere::sensor::FrameType;

    // camera frame data 관련
    using CubeEyeList = meere::sensor::CubeEyeList<uint16_t>;
    using sptrFrameList = meere::sensor::sptr_frame_list;

    NS_CUBEEYE_END
}

#endif