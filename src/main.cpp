
#include <iostream>
#include <memory>
#include <csignal>

#include <KioskFSM/InitEvent.h>
#include <KioskFSM/MoveEvent.h>
#include <KioskFSM/DetectEvent.h>
#include <KioskFSM/ExtractEvent.h>

#include <HwManager/SerialDriver.h>
#include <GpCamera/CubeEyeDriver.h>
#include <Util/Socket.h>

#include "KioskHandler.h"

using namespace CameraKiosk;

void exitProcess()
{
    std::cout << "stopped Conveyor" << std::endl;
    stopConveyor();

    std::cout << "stopped DSLR" << std::endl;
    if (CamDriver::gSharedCamContext != nullptr)
        CamDriver::gSharedCamContext->stopAsync();

    std::cout << "stopped Serial" << std::endl;
    if (!gSerialList.empty())
    {
        for (auto serialInfo : gSerialList)
        {
            if (serialInfo.second != -1)
            {
                serialClose(serialInfo.second);
            }
        }
    }

    std::cout << "stopped Cube-eye lidar" << std::endl;
    if (CamDriver::CubeEye::gCubeEyeContext != nullptr)
        CamDriver::CubeEye::gCubeEyeContext.reset();

    std::cout << "stopped Socket" << std::endl;
    if (Util::gSocketHandler != nullptr)
        Util::gSocketHandler->exitService();
    
}

void exitHandler(int sig)
{
    signal(sig, SIG_IGN);

    std::cout << "\nCanceling Program..." << std::endl;
    
    exitProcess();
    exit(0);
}

int main(int argc, const char * argv[])
{
    /* handler, listener, events 선언 */
    KioskHandler handler;
    EventListener *listener = new EventListener();

    /* events는 각 객체가 하나의 스테이트, ID 부여 및 listener subscribe */
    InitEvent *initState = new InitEvent(listener, eKioskStates::INIT);
    MoveEvent *moveState = new MoveEvent(listener, eKioskStates::MOVE);
    ExtractEvent *extState = new ExtractEvent(listener, eKioskStates::EXTRACT);
    DetectEvent *detectState = new DetectEvent(listener, eKioskStates::DETECT);


    /* handler에 event들을 등록 */
    handler.addEvent(initState->getId(), initState);
    handler.addEvent(moveState->getId(), moveState);
    handler.addEvent(extState->getId(), extState);
    handler.addEvent(detectState->getId(), detectState);

    /* add system interrupt handler */
    signal(SIGINT, exitHandler);

    /* socket server setup */
    Util::gSocketHandler = std::make_unique<Util::SocketHandler>(new Util::SocketAcceptor());

    if (Util::gSocketHandler->openAcceptor() == SOCKET_ERROR)
        exit(0);

    auto fSocketService = Util::gSocketHandler->runAsyncSocketService();

    /* DSLR camera setup */
    CamDriver::gSharedCamContext = std::make_shared<CamDriver::CamGPContext>();
    auto fGpCameraService = CamDriver::gSharedCamContext->runCaptureListener();

    while (1)
    {
        auto nextState = listener->getNextState();
        if (nextState != eKioskStates::NONE)
        {
            if(nextState == eKioskStates::EXIT)
            {
                std::cout << "program exit" << std::endl;
                exitProcess();
                break;
            }
            handler.onTransition(nextState);
        }
        else
        {
            handler.runEvent(listener->getCurrState());
        }
    }

    fSocketService.get();
    fGpCameraService.get();

    return 0;
}