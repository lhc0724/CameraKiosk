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

#include "Socket.h"

namespace CameraKiosk
{
    namespace Util
    {
        uptrSocketHandler gSocketHandler;

        int SocketHandler::openAcceptor()
        {
            std::cout << "[Socket] service initialize" << std::endl;

            try
            {
                this->_acceptor->initSocket();
            }
            catch(SocketException &ex)
            {
                std::cerr << ex.what();

                return SOCKET_ERROR;
            }

            return 0;
        }

        int SocketHandler::handle()
        {
            SocketInfo_t clientInfo;
            char clientAddr[INET_ADDRSTRLEN];
            while (1)
            {
                switch (this->_status)
                {
                case ESocketStatus::IDLE:

                    clientInfo = this->_acceptor->acceptClient();

                    if (clientInfo.first == SOCKET_ERROR)
                    {
                        this->_status = ESocketStatus::EXIT;
                    }
                    else
                    {
                        memset(clientAddr, 0, INET_ADDRSTRLEN);
                        inet_ntop(AF_INET, &clientInfo.second.sin_addr, clientAddr, INET_ADDRSTRLEN);
                        std::cout << "[Socket] Accept client: " << clientAddr << std::endl;

                        this->_status = ESocketStatus::ACCEPT_CLIENT;
                    }

                    break;
                case ESocketStatus::ACCEPT_CLIENT:

                    this->_acceptClient = std::make_unique<SocketInfo_t>(clientInfo);
                    this->_communicator = new SocketCommunicator(clientInfo.first);
                    this->_status = ESocketStatus::COMMUNICATION;

                    break;
                case ESocketStatus::COMMUNICATION:

                    this->_status = this->_communicator->runCommunication();
                    std::cout << "[Socket] Exit client: " << clientAddr << std::endl;

                    delete this->_communicator;
                    this->_acceptClient.reset();

                    break;
                default:
                    break;
                }

                if(this->_status == ESocketStatus::EXIT)
                    break;
            }

            return 0;
        }

        std::future<int> SocketHandler::runAsyncSocketService()
        {
            return std::async([&]()
                              { return handle(); });
        }

        void SocketHandler::exitService()
        {
            if(this->_acceptor != nullptr)
                this->_acceptor->cancelAcceptClient();

            if(this->_communicator != nullptr)
                this->_communicator->cancelCommunication();
        }

        void setHeader(SocketMessage_t &message, ESocketMessageType type)
        {
            std::string typeStr = "type:";
            switch (type)
            {
            case ESocketMessageType::BINARY:
                typeStr.append("binary");
                break;
            case ESocketMessageType::STRING:
                typeStr.append("string");
                break;
            case ESocketMessageType::JSON:
                typeStr.append("json");
                break;
            default:
                typeStr.append(" ");
                break;
            }

            typeStr.append(",");

            message.header.append(typeStr);
        }

        void setHeader(SocketMessage_t &message, std::string headerName, std::string headerValue)
        {
            std::string headerStr(headerName + ":" + headerValue);
            message.header.append(headerStr + ",");
        }

        bool SocketHandler::sendReqeust(std::string msg, ESocketMessageType type)
        {
            std::string sendData = "";

            if(this->_communicator == nullptr || this->_status != ESocketStatus::COMMUNICATION)
                return false;

            SocketMessage_t sendMsg;
            setHeader(sendMsg, type);

            sendMsg.data = std::vector<char>(msg.begin(), msg.end());

            return this->_communicator->enqueueData(sendMsg);
        }

        bool SocketHandler::sendReqeust(char **binaryData, size_t size, std::string fileName)
        {
            if(this->_communicator == nullptr || this->_status != ESocketStatus::COMMUNICATION)
                return false;

            SocketMessage_t sendMsg;
            setHeader(sendMsg, ESocketMessageType::BINARY);
            setHeader(sendMsg, "fileName", fileName);

            sendMsg.data.insert(sendMsg.data.end(), *binaryData, *binaryData + size);

            return this->_communicator->enqueueData(sendMsg);
        }

        ESocketStatus SocketHandler::getStatus()
        {
            return this->_status;
        }
    };
}