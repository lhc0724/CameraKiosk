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
        using namespace std;
        const char *ENV_NAME = "SOCKET_PORT";

        void SocketAcceptor::initSocket()
        {
            char *envValue = nullptr;
            struct ifreq ifr;
            sockaddr_in servAddr;

            /// @brief create socket IO
            int socketFd = socket(AF_INET, SOCK_DGRAM, 0);

            if (socketFd < 0)
            {
                throw(SocketException(ESocketError::ERR_CREATE_SOCKET));
            }

            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, TCP_INTERFACE, IFNAMSIZ - 1);

            ioctl(socketFd, SIOCGIFADDR, &ifr);

            close(socketFd);

            /// @brief get server port number by enviornment variable
            envValue = std::getenv(ENV_NAME);
            if (envValue == nullptr)
            {
                throw(SocketException(ESocketError::ERR_ENVIRONMENT));
            }

            /// @brief setup server descriptor
            bzero((char *)&servAddr, sizeof(servAddr));
            servAddr.sin_family = AF_INET;
            servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            servAddr.sin_port = htons(std::stoi(envValue));

            _listener = socket(AF_INET, SOCK_STREAM, 0);

            if (_listener < 0)
            {
                throw(SocketException(ESocketError::ERR_CREATE_SOCKET));
            }

            /// @brief set socket receive timeout
            timeval tv;
            tv.tv_sec = 0.5;
            //tv.tv_usec = 500000;
            setsockopt(_listener, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(timeval));

            /// @brief set reuseable socket descripter
            bool reuseflag = true;
            setsockopt(_listener, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseflag, sizeof(reuseflag));

            int bindStatus = bind(_listener, (struct sockaddr *)&servAddr, sizeof(servAddr));
            if (bindStatus < 0)
            {
                throw SocketException(ESocketError::ERR_SOCKET_BIND);
            }

        }

        void SocketAcceptor::cancelAcceptClient()
        {
            this->_canceller = true;
        }

        std::future<SocketInfo_t> SocketAcceptor::acceptClientAsync()
        {
            return std::async([this]
                              { return this->acceptClient(); });
        }

        SocketInfo_t SocketAcceptor::acceptClient()
        {
            int clientSd = SOCKET_ERROR;
            sockaddr_in clientAddr;
            socklen_t addrSize = sizeof(clientAddr);

            while (1)
            {
                auto listenRes = listen(this->_listener, DEFAULT_LISTEN_Q);
                if (listenRes != SOCKET_ERROR)
                {
                    clientSd = accept(this->_listener, (sockaddr *)&clientAddr, &addrSize);
                }

                if (clientSd > SOCKET_ERROR || this->_canceller)
                    break;

                this_thread::sleep_for(300ms);
            }

            return {clientSd, clientAddr};
        }
    };
}