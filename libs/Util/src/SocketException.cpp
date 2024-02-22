#include "SocketException.h"
#include <iostream>

namespace CameraKiosk
{
    namespace Util
    {
        char *SocketException::what()
        {
            std::string msg("");

            switch (ESocketError(_errCode))
            {
            case ESocketError::ERR_CREATE_SOCKET:
                msg.assign("Error - Failed create socket");
                break;
            case ESocketError::ERR_SOCKET_BIND:
                msg.assign("Error - Failed binding socket to local address");
                break;
            case ESocketError::ERR_SOCKET_ACCESS:
                msg.assign("Error - Connection error");
                break;
            case ESocketError::ERR_ENVIRONMENT:
                msg.assign("Error - No such environment value");
                break;
            default:
                break;
            }

            return (char *)msg.c_str();
        }
    };
}