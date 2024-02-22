#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "SerialDriver.h"

namespace CameraKiosk
{
    int serialOpen(const char *__file, int buadRate, bool useParity = false)
    {
        struct termios tty;
        // int port = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
        int port = open(__file, O_RDWR | O_NOCTTY | O_NDELAY);

        if (port < 0)
        {
            return -1;
            // fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
        }

        tcgetattr(port, &tty);

        // tty.c_cflag |= B9600 | CS7 | CLOCAL | CREAD | PARODD | PARENB; //<Set baud rate
        tty.c_cflag = CREAD | CLOCAL;
        if (useParity)
        {
            tty.c_cflag |= CS7 | PARODD | PARENB;
        }
        else
        {
            tty.c_cflag |= CS8;
        }

        switch (buadRate)
        {
        case 2400:
            tty.c_cflag |= B2400;
            break;
        case 4800:
            tty.c_cflag |= B4800;
            break;
        case 9600:
            tty.c_cflag |= B9600;
            break;
        case 19200:
            tty.c_cflag |= B19200;
            break;
        case 38400:
            tty.c_cflag |= B38400;
            break;
        case 57600:
            tty.c_cflag |= B57600;
            break;
        case 115200:
            tty.c_cflag |= B115200;
            break;
        default:
            // not support buadrate
            close(port);
            return -1;
        }

        tty.c_iflag = IGNPAR;
        tty.c_oflag = 0;
        tty.c_lflag = 0;
        tcflush(port, TCIFLUSH);

        tcsetattr(port, TCSANOW, &tty);

        return port;
    }

    int serialWrite(int port, const char *data)
    {
        if (port == -1)
        {
            return -1;
        }

        int writeBytes = write(port, data, sizeof(data));
        return writeBytes;
    }

    int getSerialAvail(int port)
    {
        int bytes;

        if (port == -1)
        {
            return -1;
        }

        fcntl(port, F_SETFL, 0);
        ioctl(port, FIONREAD, &bytes);

        return bytes;
    }

    const char *serialRead(int port, int size)
    {
        unsigned char *buffer = new unsigned char[size + 1];

        int rxLen = read(port, (void *)buffer, size);
        if (rxLen < 0)
        {
            // on Error;
            return nullptr;
        }
        else
        {
            return (const char *)buffer;
        }
    }

    void serialClear(int port)
    {
        ioctl(port, TCFLSH, 2);
    }

    void serialClose(int port)
    {
        if (port != -1)
        {
            serialClear(port);
            close(port);
        }
    }
}