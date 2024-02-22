#ifndef EXTRACT_EVENT_H
#define EXTRACT_EVENT_H

#include <HwManager/GpioDriver.h>

#include "KioskInterface.h"

namespace CameraKiosk
{
    class ExtractEvent : public KioskEvent
    {
    private:
        int _irDetectCnt;
    public:
        ExtractEvent(EventListener *listener, eKioskStates evtId) : KioskEvent(listener)
        {
            _irDetectCnt = 0;
            setId(evtId);
        };
        ~ExtractEvent(){};

        void notify(eKioskResult res, eKioskStates nextState) override;
        void onEnter() override;
        void onEvent() override;
    };
};

#endif
