#ifndef GP_COMMON_H
#define GP_COMMON_H

#include <iostream>
#include <exception>
#include <gphoto2/gphoto2-camera.h>

namespace CamDriver
{
    enum class eGpResult
    {
        SUCCESS = GP_OK,
        ERR = GP_ERROR,
        ERR_BAD_PARAM = GP_ERROR_BAD_PARAMETERS,
        ERR_NO_MEMORY = GP_ERROR_NO_MEMORY,
        ERR_LIBRARY = GP_ERROR_LIBRARY,
        ERR_UNKNOWN_PORT = GP_ERROR_UNKNOWN_PORT,
        ERR_NOT_SUPPORTED = GP_ERROR_NOT_SUPPORTED,
        ERR_IO = GP_ERROR_IO,
        ERR_OVERFLOW = GP_ERROR_FIXED_LIMIT_EXCEEDED,
        ERR_TIMEOUT = GP_ERROR_TIMEOUT,
        ERR_IO_SERIAL = GP_ERROR_IO_SUPPORTED_SERIAL,
        ERR_IO_USB = GP_ERROR_IO_SUPPORTED_USB,
        ERR_IO_INIT = GP_ERROR_IO_INIT,
        ERR_IO_READ = GP_ERROR_IO_READ,
        ERR_IO_WRITE = GP_ERROR_IO_WRITE,
        ERR_IO_UPDATE = GP_ERROR_IO_UPDATE,
        ERR_IO_SERIAL_SPEED = GP_ERROR_IO_SERIAL_SPEED,
        ERR_IO_USB_HALT = GP_ERROR_IO_USB_CLEAR_HALT,
        ERR_IO_USB_FIND = GP_ERROR_IO_USB_FIND,
        ERR_IO_USB_CLAIM = GP_ERROR_IO_USB_CLAIM,
        ERR_IO_LOCK = GP_ERROR_IO_LOCK,
        ERR_HAL = GP_ERROR_HAL,
    };

    enum class eGpCapMode
    {
        NO_EVENT,
        TO_FILE,
        TO_MEMORY,
        EXIT
    };

    enum class eGpCamStatus
    {
        IDLE,
        ON_EVENT
    };

    class GpException : public std::exception
    {
    private:
        eGpResult _res;
    public:
        GpException(eGpResult result) {
            this->_res = result;
        }

        char *what()
        {
            return (char *)gp_port_result_as_string(static_cast<int>(this->_res));
        };

        eGpResult code()
        {
            return _res;
        }
    };

    class GpExceptionThrower
    {
    public:
        GpExceptionThrower() {};
        GpExceptionThrower(eGpResult result)
        {
            checkOnError(result);
        };
        GpExceptionThrower(int orgResult)
        {
            // auto convRes = static_cast<GP_RESULT>(orgResult);
            checkOnErrorOrg(orgResult);
        };
        ~GpExceptionThrower() {};

        void checkOnError(eGpResult res)
        {
            if(res < eGpResult::SUCCESS)
            {
                throw(GpException(res));
            }
        }
        void checkOnErrorOrg(int orgRes)
        {
            auto convRes = static_cast<eGpResult>(orgRes);
            checkOnError(convRes);
        }
    };
}

#endif