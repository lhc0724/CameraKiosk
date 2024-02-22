#include "MoveEvent.h"

namespace CameraKiosk
{
    void MoveEvent::notify(eKioskResult res, eKioskStates nextState)
    {
        this->_listener->update(res, nextState);
    }

    void MoveEvent::onEnter()
    {
        this->_irDetectCnt = 0;
        runConveyor();
        notify(eKioskResult::CONTINUE, getId());
    }

    void MoveEvent::onEvent()
    {
        //sensor chattering filter
        if(readIRSensor() == IR_ENABLE) 
        {
            this->_irDetectCnt++;
        }
        else 
        {
            this->_irDetectCnt = 0;
        }

        if(_irDetectCnt >= IR_FILTER_CNT) {
            stopConveyor();
            notify(eKioskResult::TRANSITION, eKioskStates::DETECT);
        }
    }
}