#ifndef SOCKET_ENUM_H
#define SOCKET_ENUM_H

namespace CameraKiosk
{
    namespace Util
    {
        enum class ESocketStatus
        {
            IDLE,
            LISTEN,
            ACCEPT_CLIENT,
            COMMUNICATION,
            EXIT
        };

        enum class ESocketMessageType
        {
            STRING,
            JSON,
            BINARY
        };

        enum class ESocketError
        {
            ERR_CREATE_SOCKET,
            ERR_ENVIRONMENT,
            ERR_SOCKET_BIND,
            ERR_SOCKET_ACCESS
        };
    };
};

#endif