#ifndef JSON_READER_H
#define JSON_READER_H

#include <jsoncpp/json/json.h>
#include <string>

namespace CameraKiosk
{
    namespace JsonBuilder
    {
        Json::Value readJsonFile(std::string filePath);
        bool writeJson(Json::Value json, std::string fileName);
    }
}

#endif
