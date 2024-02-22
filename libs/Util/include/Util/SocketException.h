#ifndef EC_SOCKET_EXCEPTION_H
#define EC_SOCKET_EXCEPTION_H

#include <exception>
#include "SocketEnum.h"

namespace CameraKiosk
{
    namespace Util
    {
        enum class ESocketError;

        class SocketException : public std::exception
        {
        private:
            ESocketError _errCode;

        public:
            SocketException(ESocketError code) : _errCode(code){};
            ~SocketException(){};
            ESocketError code() const { return _errCode; };
            char *what();
        };

    };
}

#endif