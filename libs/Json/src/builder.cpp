#include "builder.h"

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
namespace CameraKiosk
{
    namespace JsonBuilder
    {
        Json::Value readJsonFile(std::string filePath)
        {
            Json::Value root;
            std::ifstream fileId(filePath);

            fileId >> root;
            fileId.close();

            return root;
        }

        bool writeJson(Json::Value json, std::string fileName)
        {
            auto projDir = fs::current_path().parent_path();
            std::string jsonDir("");
            bool ret = false;

            for (auto const &dirEntry : fs::directory_iterator{projDir})
            {
                if (!dirEntry.path().filename().compare("jsons"))
                {
                    if (dirEntry.is_directory())
                    {
                        jsonDir = dirEntry.path().string() + "/";
                    }
                    break;
                }
            }

            if(jsonDir != "")
            {
                std::ofstream fileId(jsonDir.append(fileName));

                try
                {
                    // Json::StyledStreamWriter writer;
                    // writer.write(fileId, json);

                    /* jsoncpp new version test code */
                    Json::StreamWriterBuilder builder;
                    builder["commentStyle"] = "None";
                    builder["indentation"] = "\t";

                    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

                    writer->write(json, &fileId);
                    fileId.close();
                    ret = true;
                }
                catch(const std::exception& e)
                {
                    // std::cerr << e.what() << '\n';
                    ret = false;
                }
                
            }

            return ret;
        }
    }
}
