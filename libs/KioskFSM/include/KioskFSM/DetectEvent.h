#ifndef DETECT_EVENT_H
#define DETECT_EVENT_H

#include "KioskInterface.h"

#include <ctime>
#include <HwManager/GpioDriver.h>
#include <HwManager/Balancer.h>
#include <GpCamera/GpCamera.h>
#include <GpCamera/CubeEyeDriver.h>
#include <Json/builder.h>

#include <future>

namespace CameraKiosk
{
    class DetectEvent : public KioskEvent
    {
    private:
        std::future<float> _fObjWeight;
        Json::Value infoJsonObj;

    public:
        DetectEvent(EventListener *listener, eKioskStates evtId): KioskEvent(listener)
        {
            setId(evtId);
        }
        ~DetectEvent() {};

        void notify(eKioskResult res, eKioskStates nextState) override; 
        void onEnter() override;
        void onEvent() override;
    };
};

#endif
