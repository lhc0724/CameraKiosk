#include "EcCubeEyeProps.h"
#include <iostream>
#include <algorithm>

namespace CamDriver
{
    NS_CUBEEYE_BEGIN

    void PrintProperty(const sptrCamProp &prop);

    sptrCamProp PropManager::makeProperty(PropAttr attribute, void *value)
    {
        sptrCamProp prop;

        switch (attribute.second)
        {
        case CubeEyeDataType::DataType_Boolean:
            prop = meere::sensor::make_property_bool(attribute.first, *(bool*)value);
            break;
        case CubeEyeDataType::DataType_8S:
            prop = meere::sensor::make_property_8s(attribute.first, *(int8_t*)value);
            break;
        case CubeEyeDataType::DataType_8U:
            prop = meere::sensor::make_property_8u(attribute.first, *(uint8_t*)value);
            break;
        case CubeEyeDataType::DataType_16S:
            prop = meere::sensor::make_property_16s(attribute.first, *(int16_t*)value);
            break;
        case CubeEyeDataType::DataType_16U:
            prop = meere::sensor::make_property_16u(attribute.first, *(uint16_t*)value);
            break;
        case CubeEyeDataType::DataType_32F:
            prop = meere::sensor::make_property_32f(attribute.first, *(float*)value);
            break;
        default:
            break;
        }
        return prop;
    }

    void PropManager::makeDefaultPropList()
    {
        _manageProps.push_back(makeProperty(AttrFrameRate, (void *)&PropValFrameRate));
        _manageProps.push_back(makeProperty(AttrAutoExposure, (void *)&PropValAutoExposure));

        _manageProps.push_back(makeProperty(AttrIlluminate, (void *)&PropValIlluminate));

        _manageProps.push_back(makeProperty(AttrMaxDepth, (void *)&PropValDepthMax));
        _manageProps.push_back(makeProperty(AttrMinDepth, (void *)&PropValDepthMin));

        _manageProps.push_back(makeProperty(AttrFlyingPixelFilter, (void *)&PropValFlyPixelFilter));
        _manageProps.push_back(makeProperty(AttrFlyingPixelThreshold, (void *)&PropValFlyThreshold));
        _manageProps.push_back(makeProperty(AttrScattThreshold, (void *)&PropValScattThreshold));
    }

    void PropManager::setDefaultProperties(Camera *cam)
    {
        this->_manageProps.clear();
        makeDefaultPropList();

        if (cam != nullptr)
        {
            for (auto it : _manageProps)
            {
                auto resProp = cam->getProperty(it->key());
                if(compareProperty(std::get<1>(resProp), it))
                {
                    cam->setProperty(it);
                }
            }
        }
    }

    sptrCamResProp PropManager::addProperty(Camera *cam, PropAttr attribute, void *value)
    {
        if(cam == nullptr)
        {
            return std::make_tuple(eCubeEyeRes::no_such_device, nullptr);
        }

        auto newProp = makeProperty(attribute, value);

        for(size_t i = 0; i < _manageProps.size(); i++)
        {
            auto prop = _manageProps.at(i);
            
            //중복 키 체크
            if(prop->key() == attribute.first)
            {
                _manageProps.erase(_manageProps.begin() + i);
                break;
            }
        }

        _manageProps.push_back(newProp);

        // debug 
        // for(auto it:_manageProps)
        // {
        //     PrintProperty(it);
        // }

        cam->setProperty(newProp);
        return cam->getProperty(newProp.get()->key());
    }

    sptrCamProp PropManager::findProperty(PropAttr attribute)
    {
        for(auto it:_manageProps)
        {
            if(it->key() == attribute.first)
            {
                return it;
            }
        }

        return nullptr;
    }
    sptrCamProp PropManager::findProperty(std::string attrKey)
    {
        for(auto it:_manageProps)
        {
            if(it->key() == attrKey)
            {
                return it;
            }
        }

        return nullptr;
    }

    int compareProperty(const sptrCamProp &propOrg, const sptrCamProp &propComp)
    {
        if (propOrg->key() != propComp->key())
        {
            return -1;
        }

        if(propOrg->dataType() != propComp->dataType())
        {
            return -1;
        }

        switch(propOrg->dataType())
        {
        case CubeEyeDataType::DataType_Boolean:
            if(propOrg->asBoolean() != propComp->asBoolean())
            {
                std::cout << propOrg->key() << " " << propOrg->asBoolean() << " -> " << propComp->asBoolean() << std::endl;
                return 1;
            }
        case CubeEyeDataType::DataType_8S:
            if(propOrg->asInt8s() != propComp->asInt8s())
            {
                std::cout << propOrg->key() << " " << propOrg->asInt8s() << " -> " << propComp->asInt8s() << std::endl;
                return 1;
            }
        case CubeEyeDataType::DataType_8U:
            if(propOrg->asInt8u() != propComp->asInt8u())
                return 1;
        case CubeEyeDataType::DataType_16S:
            if(propOrg->asInt16s() != propComp->asInt16s())
                return 1;
        case CubeEyeDataType::DataType_16U:
            if(propOrg->asInt16u() != propComp->asInt16u())
                return 1;
        case CubeEyeDataType::DataType_32S:
            if(propOrg->asInt32s() != propComp->asInt32s())
                return 1;
            break;
        case CubeEyeDataType::DataType_32U:
            if(propOrg->asInt32u() != propComp->asInt32u())
                return 1;
            break;
        case CubeEyeDataType::DataType_32F:
            if(propOrg->asFlt32() != propComp->asFlt32())
                return 1;
            break;
        case CubeEyeDataType::DataType_64F:
            if(propOrg->asFlt64() != propComp->asFlt64())
                return 1;
            break;
        case CubeEyeDataType::DataType_String:
            if(propOrg->asString() != propComp->asString())
                return 1;
            break;
        default:
            return 0;
        }

        return 0;
    }

    void PrintProperty(const sptrCamProp &prop)
	{
		if (nullptr != prop && !prop->key().empty())
		{
			std::cout << prop->key() << " -> result : ";
			switch (prop->dataType())
			{
			case CubeEyeDataType::DataType_Boolean:
				printf("%s\n", prop->asBoolean() ? "true" : "false");
				break;
			case CubeEyeDataType::DataType_8S:
				printf("%hhd\n", prop->asInt8s());
				break;
			case CubeEyeDataType::DataType_8U:
				printf("%hhu\n", prop->asInt8u());
				break;
			case CubeEyeDataType::DataType_16S:
				printf("%hd\n", prop->asInt16s());
				break;
			case CubeEyeDataType::DataType_16U:
				printf("%hu\n", prop->asInt16u());
				break;
			case CubeEyeDataType::DataType_32S:
				printf("%d\n", prop->asInt32s());
				break;
			case CubeEyeDataType::DataType_32U:
				printf("%u\n", prop->asInt32u());
				break;
			case CubeEyeDataType::DataType_32F:
				printf("%f\n", prop->asFlt32());
				break;
			case CubeEyeDataType::DataType_64F:
				printf("%lf\n", prop->asFlt64());
				break;
			case CubeEyeDataType::DataType_String:
				std::cout << prop->asString() << std::endl;
				break;
			default:
				std::cout << "invalid data type" << std::endl;
				break;
			}
		}
	};
    NS_CUBEEYE_END
}