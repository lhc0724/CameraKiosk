#ifndef ENUMS_H
#define ENUMS_H

namespace CameraKiosk
{
    enum class eKioskStates
    {
        NONE,           //not state
        INIT,           //hardware initialization. setup gpio, camera, etc...
        WAIT,
        AUTHENTICATION, 
        MOVE,           //move an object, Conveyor start
        EXTRACT,        //running Conveyor do until object extracted from stopper
        DETECT,         //after IR sensor enabled. object capture and calculation volumn
        COMMUNICATION,  //object data upload to service server(acSell)
        EXIT,           //state machine exit.
    };

    enum class eKioskResult
    {
        TRANSITION,
        CONTINUE,
        ERROR
    };
};
#endif
