#ifndef KIOSK_HANDLER_H
#define KIOSK_HANDLER_H

#include <KioskFSM/KioskInterface.h>

namespace CameraKiosk
{
    class KioskHandler
    {
    private:
        evtList_t _eventList;

    public:
        KioskHandler() {};
        ~KioskHandler(){};

        void addEvent(eKioskStates evtId, KioskEvent *event);
        void runEvent(eKioskStates evtId);
        void onTransition(eKioskStates evtId);
    };
};

#endif
