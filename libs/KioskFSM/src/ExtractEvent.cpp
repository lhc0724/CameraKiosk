#include "ExtractEvent.h"
#include <HwManager/Balancer.h>

namespace CameraKiosk
{
    void ExtractEvent::notify(eKioskResult res, eKioskStates nextState)
    {
        this->_listener->update(res, nextState);
    }

    void ExtractEvent::onEnter()
    {
        this->_irDetectCnt = 0;
        runConveyor();
        BalancerAND::balancerEnd();
        notify(eKioskResult::CONTINUE, getId());
    }

    void ExtractEvent::onEvent()
    {
        //sensor chattering filter
        if(readIRSensor() == IR_DISABLE) 
        {
            this->_irDetectCnt++;
        }
        else 
        {
            this->_irDetectCnt = 0;
        }

        if(_irDetectCnt >= IR_FILTER_CNT) {
            notify(eKioskResult::TRANSITION, eKioskStates::MOVE);
        }
    }
}