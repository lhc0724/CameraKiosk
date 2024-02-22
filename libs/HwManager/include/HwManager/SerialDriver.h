#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

#include <string>
#include <map>

namespace CameraKiosk
{
    static std::map<std::string, int> gSerialList; //pair<id, port>

    /* general functions of serial */
    int serialOpen(const char *__file, int buadRate, bool useParity);
    int serialWrite(int port, const char *data);
    int getSerialAvail(int port);
    const char *serialRead(int port, int size);
    void serialClear(int port);
    void serialClose(int port);
}

#endif
