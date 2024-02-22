#include <iostream>
#include "KioskInterface.h"

namespace CameraKiosk
{
    /* event listener functions */
    void EventListener::changeState(eKioskStates nextState)
    {
        this->_nextState = nextState;
    }

    void EventListener::update(eKioskResult result, eKioskStates state)
    {

        switch (result)
        {
        case eKioskResult::CONTINUE:
        case eKioskResult::TRANSITION:
            this->_errCnt = 0;
            break;
        default:
            break;
        }

        switch (result)
        {
        case eKioskResult::CONTINUE:
            std::cout << "[StateMachine] noti - continue " << stateEnumToString(state) << std::endl;
            changeState(eKioskStates::NONE);
            this->_currState = state;
            break;
        case eKioskResult::TRANSITION:
            std::cout << "[StateMachine] noti - transition to " << stateEnumToString(state) << std::endl;
            changeState(state);
            break;
        case eKioskResult::ERROR:
            _errCnt++;
            onError();
            break;
        default:
            break;
        }
    }

    eKioskStates EventListener::getNextState()
    {
        return this->_nextState;
    }

    eKioskStates EventListener::getCurrState()
    {
        return this->_currState;
    }

    void EventListener::onError()
    {
        std::cout << "[" << stateEnumToString(this->_currState) << "] Error try again... " << this->_errCnt << std::endl;
        changeState(this->_currState);
        if(this->_errCnt >= 5)
        {
            // program exit
            changeState(eKioskStates::EXIT);
        }
    }

    /* event interface base functions */
    void KioskEvent::setId(eKioskStates evtId)
    {
        this->_id = evtId;
    }

    eKioskStates KioskEvent::getId()
    {
        return this->_id;
    }

    std::string stateEnumToString(eKioskStates state)
    {
        switch (state)
        {
        case eKioskStates::NONE:
            return "NONE";
        case eKioskStates::INIT:
            return "INIT";
        case eKioskStates::WAIT:
            return "WAIT";
        case eKioskStates::AUTHENTICATION:
            return "AUTHENTICATION";
        case eKioskStates::MOVE:
            return "MOVE";
        case eKioskStates::DETECT:
            return "DETECT";
        case eKioskStates::COMMUNICATION:
            return "COMMUNICATION";
        case eKioskStates::EXTRACT:
            return "EXTRACT";
        case eKioskStates::EXIT:
            return "EXIT";
        }

        return "";
    }

};