#include "KioskHandler.h"

namespace CameraKiosk
{
    void KioskHandler::addEvent(eKioskStates evtId, KioskEvent *event) 
    {
        this->_eventList.insert(std::make_pair(evtId, event));
    }

    void KioskHandler::runEvent(eKioskStates evtId)
    {
        auto event = this->_eventList.at(evtId);
        event->onEvent();
    }

    void KioskHandler::onTransition(eKioskStates evtId)
    {
        auto event = this->_eventList.at(evtId);
        event->onEnter();
    }
};