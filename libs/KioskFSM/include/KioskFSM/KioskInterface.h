#ifndef KIOSK_INTERFACE_H
#define KIOSK_INTERFACE_H

#include "Enums.h"
#include <map>
#include <string>

namespace CameraKiosk
{
    class KioskEvent;
    using evtList_t = std::map<eKioskStates, KioskEvent*>;

    class EventListener //: public Observer<eKioskStates>
    {
    private:
        eKioskStates _nextState;
        eKioskStates _currState;

        int _errCnt;
        void onError();
        void changeState(eKioskStates nextState);

    public:
        EventListener() : _errCnt(0) {
            _nextState = eKioskStates::INIT;
        };
        ~EventListener() {};

        void update(eKioskResult result, eKioskStates state);
        eKioskStates getNextState();
        eKioskStates getCurrState();
    };

    class KioskEvent {
    private:
        eKioskStates _id;

    public:
        EventListener *_listener;

        KioskEvent(EventListener *listener)
        {
            this->_listener = listener;
        };
        ~KioskEvent(){};

        void setId(eKioskStates id);
        eKioskStates getId();

        virtual void notify(eKioskResult res, eKioskStates nextState = eKioskStates::NONE) = 0;
        virtual void onEvent() = 0;
        virtual void onEnter() = 0;
    };

    std::string stateEnumToString(eKioskStates state);
};

#endif