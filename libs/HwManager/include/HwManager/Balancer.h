#ifndef BALANCER_H
#define BALANCER_H

#include "SerialDriver.h"

#define ANDCMD_READ "Q\r\n"
#define ANDCMD_ZERO "Z\r\n"
#define AND_DEFAULT_ID "AND"

namespace CameraKiosk
{
    namespace BalancerAND
    {
        int setup();
        int setToZero();
        void balancerEnd();
        float getWeightValue();
    }

} 

#endif