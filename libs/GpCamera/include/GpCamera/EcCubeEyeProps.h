#ifndef EC_CUBEEYE_PROPS_H
#define EC_CUBEEYE_PROPS_H

#include <string>
#include <vector>

#include "CubeEyeWrapper.h"

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    using PropAttr = std::pair<std::string, CubeEyeDataType>;
    using PropManageList = std::vector<sptrCamProp>;
    // using PropManageMap = std::map<PropAttr, uint32_t*>;

    /** 
    * @brief
    * Attribute와 Property의 차이점:
    * CubeEye Lidar ToF Camera에 직접적으로 쓰여지는 값이 Property.
    * Attribute는 어떤 Property에 접근 하기 위한 일종의 키 속성 객체. 
    * 
    * Attribute + Value = Property
    */

    const PropAttr AttrAutoExposure({"auto_exposure", CubeEyeDataType::DataType_Boolean});
    const PropAttr AttrIlluminate({"illumination", CubeEyeDataType::DataType_Boolean});
    const PropAttr AttrFrameRate({"framerate", CubeEyeDataType::DataType_8U});

    const PropAttr AttrMinDepth({"depth_range_min", CubeEyeDataType::DataType_16S});
    const PropAttr AttrMaxDepth({"depth_range_max", CubeEyeDataType::DataType_16S});
    const PropAttr AttrDepthOffset({"depth_offset", CubeEyeDataType::DataType_16S});

    const PropAttr AttrMinAmpThreshold({"amplitude_threshold_min", CubeEyeDataType::DataType_16S});
    const PropAttr AttrMaxAmpThreshold({"amplitude_threshold_max", CubeEyeDataType::DataType_16S});
    const PropAttr AttrFlyingPixelThreshold({"flying_pixel_remove_threshold", CubeEyeDataType::DataType_16S});
    const PropAttr AttrFlyingPixelFilter({"flying_pixel_remove_filter", CubeEyeDataType::DataType_16S});
    const PropAttr AttrScattThreshold({"scattering_threshold", CubeEyeDataType::DataType_16S});
    const PropAttr AttrAmpTimeFilter({"amplitude_time_filter", CubeEyeDataType::DataType_Boolean});

    const PropAttr AttrLogLevel({"log_level", CubeEyeDataType::DataType_8S});                   //write only
    const PropAttr AttrResetProps({"reset_properties", CubeEyeDataType::DataType_Boolean});     //write only
    const PropAttr AttrTemperature({"temperature", CubeEyeDataType::DataType_32F});             //read only

    //const std::string AttrFlyingPixelFilter("flying_pixel_remove_filter");

    /* default Camera setup properties value */

    // dpeth values, [mm] scale
    const int16_t PropValDepthOffset    = 952;    
    const int16_t PropValDepthMin       = 150; 
    const int16_t PropValDepthMax       = 1100; 

    // filtering values
    const int16_t PropValScattThreshold = 200;
    const int16_t PropValFlyThreshold   = 50;
    const bool PropValFlyPixelFilter    = true;

    // camera control values
    const bool PropValAutoExposure      = true;
    const bool PropValIlluminate        = true;
    const uint8_t PropValFrameRate      = 30;

    /* default Camera (W x H) Size */
    const int32_t CamWidth = 640;
    const int32_t CamHeight = 480;

    class PropManager
    {
    private:
        PropManageList _manageProps;

    public:
        PropManager()
        {
            _manageProps.clear();
        }
        ~PropManager() {}

        void setDefaultProperties(Camera *cam);
        void makeDefaultPropList();

        sptrCamResProp addProperty(Camera *cam, PropAttr attribute, void *value);
        sptrCamProp makeProperty(PropAttr attribute, void *value);

        sptrCamProp findProperty(PropAttr attribute);
        sptrCamProp findProperty(std::string attrKey);
    };

    int compareProperty(const sptrCamProp &propOrg, const sptrCamProp &propComp);
    void PrintProperty(const sptrCamProp &prop);

    NS_CUBEEYE_END
}

#endif