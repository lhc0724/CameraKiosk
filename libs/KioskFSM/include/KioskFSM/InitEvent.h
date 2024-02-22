#ifndef INITEVENT_H
#define INITEVENT_H

#include "KioskInterface.h"
#include <GpCamera/GpCamera.h>
#include <HwManager/GpioDriver.h>
#include <HwManager/Balancer.h>

namespace CameraKiosk
{
    class InitEvent : public KioskEvent
    {

    public:
        InitEvent(EventListener *listener, eKioskStates evtId): KioskEvent(listener)
        {
            setId(evtId);
        }
        ~InitEvent() {};

        void notify(eKioskResult res, eKioskStates nextState) override; 
        void onEnter() override;
        void onEvent() override;
    };
};

#endif
