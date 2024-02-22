#include <iostream>
#include <filesystem>
#include <string>
#include <chrono>

#include "Balancer.h"

namespace fs = std::filesystem;
namespace CameraKiosk
{
    namespace BalancerAND
    {
        std::string readWithTimeout(int port, int mSecTime)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            std::string rxData("");

            while (1)
            {
                auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
                int rxBytes = getSerialAvail(port);

                if (!rxBytes)
                {
                    // exit loop when the buffer remains empty for more than mSecTime[milliSecond]
                    auto miliSec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                    if (miliSec >= mSecTime)
                    {
                        break;
                    }
                }
                else
                {
                    //time reset
                    startTime = std::chrono::high_resolution_clock::now();

                    //read serial buffer
                    const char *buffer = serialRead(port, rxBytes);
                    rxData.append(buffer);
                }
            }

            return rxData;
        }

        const char *detectSerialDevice()
        {
            fs::directory_iterator itr("/dev");

            const char *comDevice = nullptr;

            while (itr != fs::end(itr))
            {
                const fs::directory_entry &entry = *itr;
                std::string fileName = entry.path();
                if (fileName.find("ttyUSB") != std::string::npos)
                {
                    comDevice = fileName.c_str();
                    break;
                }
                itr++;
            }

            return comDevice;
        }

        int setup()
        {
            int port = serialOpen(detectSerialDevice(), 9600, true);
            if (port == -1)
            {
                std::cout << "Unable to open serial device" << std::endl;
                return -1;
            }

            gSerialList.insert({AND_DEFAULT_ID, port});

            return 0;
        }

        int setToZero()
        {
            //int port = getPort();
            auto serialInfo = gSerialList.find(AND_DEFAULT_ID);
            if(serialInfo == gSerialList.end())
            {
                // error - not found serial info;
                std::cout << "\nerror: not found serial" << serialInfo->first << std::endl;
                return -1;
            }

            int port = serialInfo->second;
            serialWrite(port, ANDCMD_ZERO);

            auto rxRes = readWithTimeout(port, 500);
            if(rxRes.length() < 3 ||  rxRes.find("Z") == std::string::npos)
            {
                return -1;
            }

            return 0;
        }

        float getWeightValue()
        {
            auto portInfo = gSerialList.find(AND_DEFAULT_ID);
            if(portInfo == gSerialList.end())
            {
                //on Error: not found serial port
                return -1;
            }

            int port = portInfo->second;

            serialClear(port);
            serialWrite(port, ANDCMD_READ);

            auto rxRes = readWithTimeout(port, 100);
            float weightValue = 0;

            std::cout << "\n[Balancer] (" << rxRes.size() << ") - " << rxRes;
            if (rxRes.size() >= 17)
            {
                //check header
                if (rxRes.find("ST") || rxRes.find("US"))
                {
                    auto data = rxRes.substr(rxRes.find("+") + 1, 8);
                    weightValue = std::stof(data);
                }
            }

            return weightValue;
        }

        void balancerEnd()
        {
            auto port = gSerialList.find(AND_DEFAULT_ID);
            if (port != gSerialList.end())
            {
                gSerialList.erase(port);
                serialClose(port->second);
            }
        }
    }
};
