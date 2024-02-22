#ifndef MOVE_EVENT_H
#define MOVE_EVENT_H

#include <HwManager/GpioDriver.h>

#include "KioskInterface.h"

namespace CameraKiosk
{
    class MoveEvent : public KioskEvent
    {
    private:
        int _irDetectCnt;
    public:
        MoveEvent(EventListener *listener, eKioskStates evtId) : KioskEvent(listener)
        {
            _irDetectCnt = 0;
            setId(evtId);
        };
        ~MoveEvent(){};

        void notify(eKioskResult res, eKioskStates nextState) override;
        void onEnter() override;
        void onEvent() override;
    };
};

#endif
