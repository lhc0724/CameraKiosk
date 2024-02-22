#include "Socket.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <errno.h>

namespace CameraKiosk
{
    namespace Util
    {
        using namespace std;

        union IntToByte
        {
            unsigned int data;
            char transData[4];
        };

        bool SocketCommunicator::enqueueData(SocketMessage_t message)
        {
            lock_guard<mutex>(this->_sendQM);

            auto beforeSize = this->sendMessageQ.size();
            if (beforeSize < 10)
            {
                this->sendMessageQ.emplace(message);
            }

            return (this->sendMessageQ.size() > beforeSize) ? true : false;
        }

        bool SocketCommunicator::sendData(const void *buff, size_t buffSize)
        {
            const unsigned char *pbuf = (const unsigned char *)buff;

            while (buffSize > 0)
            {
                int sendBytes = send(this->_clientSd, pbuf, buffSize, MSG_NOSIGNAL);

                if(sendBytes == SOCKET_ERROR)
                {
                    return false;
                }

                pbuf += sendBytes;
                buffSize -= sendBytes;
            }

            return true;
        }

        bool SocketCommunicator::sendData(SocketMessage_t &message)
        {

            IntToByte headerSize, dataSize;
            headerSize.data = (unsigned int)message.header.size();
            dataSize.data = (unsigned int)message.data.size();

            // vector size: headerSize (4Byte) + dataSize (4Byte) + header + data
            std::vector<char> sendData = std::vector<char>(headerSize.data + dataSize.data + 8);
            sendData.clear();

            // size info inserts
            for (auto i : headerSize.transData)
                sendData.push_back(i);

            for (auto i : dataSize.transData)
                sendData.push_back(i);

            //header insert
            sendData.insert(sendData.end(), message.header.begin(), message.header.end());

            //data insert
            sendData.insert(sendData.end(), message.data.begin(), message.data.end());

            if (sendData.size())
            {
                auto writeBytes = send(this->_clientSd, &sendData[0], sendData.size(), MSG_NOSIGNAL);
                std::cout << "[Socket] send " << writeBytes << "[Byte]\n";
                if (writeBytes == SOCKET_ERROR)
                    return false;
            }

            return true;
        }

        void SocketCommunicator::recvMessageHandle(char *recvMsg)
        {
            SocketMessage_t message;
            std::string sendMsg = "";

            setHeader(message, ESocketMessageType::STRING);

            if (!strcmp(recvMsg, "ACK"))
            {
                sendMsg = "ok.";
            }
            else
            {
                sendMsg = "invalid command.";
            }

            message.data = std::vector<char>(sendMsg.begin(), sendMsg.end());
            this->enqueueData(message);
        }

        void SocketCommunicator::cancelCommunication()
        {
            this->_canceller = true;
        }

        ESocketStatus SocketCommunicator::runCommunication()
        {
            int read = 0;

            ESocketStatus result = ESocketStatus::EXIT;
            char readBuff[512];

            while(1)
            {
                memset(&readBuff, 0, sizeof(readBuff));
                read = recv(this->_clientSd, (char*) &readBuff, sizeof(readBuff), 0);

                if (!strcmp(readBuff, "exit"))
                {
                    result = ESocketStatus::IDLE;
                    break;
                }

                if (this->_canceller)
                {
                    result = ESocketStatus::EXIT;
                    break;
                }

                if (read != SOCKET_ERROR)
                {
                    recvMessageHandle(readBuff);
                    read = 0;
                }

                if (!this->sendMessageQ.empty())
                {
                    std::lock_guard<std::mutex>(this->_sendQM);
                    auto sendBuff = this->sendMessageQ.front();
                    this->sendMessageQ.pop();

                    if (!this->sendData(sendBuff))
                    {
                        if(errno == EPIPE)
                        {
                            result = ESocketStatus::IDLE;
                            break;
                        }
                        else
                        {
                            std::cerr << "[Socket] send failed: " << strerror(errno) << std::endl;
                        }
                    }
                }
                else
                {
                    this_thread::sleep_for(200ms);
                }
            }

            return result;
        }

    };
}