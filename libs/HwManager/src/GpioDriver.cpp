#include "GpioDriver.h"

namespace CameraKiosk
{
    bool gpioSetup()
    {
        if (wiringPiSetupGpio() == -1)
        {
            // failed raspberry pi gpio settings...
            return false;
        }

        // Setup IR Sensor - input
        pinMode(GPIO_IR, INPUT);

        // Setup Relay - pullDown, output
        pullUpDnControl(GPIO_RELAY, PUD_DOWN);
        pinMode(GPIO_RELAY, OUTPUT);
        digitalWrite(GPIO_RELAY, LOW);

        // Setup Power Signal - input/interrupt
        pullUpDnControl(GPIO_PWRSIG, PUD_DOWN);
        pinMode(GPIO_PWRSIG, INPUT);
        // wiringPiISR(GPIO_PWRSIG, INT_EDGE_FALLING, &handleInterrupt);

        return true;
    };
    void addInterrupt(int gpio, void interruptFunc())
    {
        pinMode(gpio, INPUT);
        wiringPiISR(gpio, INT_EDGE_FALLING, interruptFunc);
    };

    int readIRSensor()
    {
        return digitalRead(GPIO_IR);
    };

    int readPower()
    {
        return digitalRead(GPIO_PWRSIG);
    };

    void runConveyor()
    {
        digitalWrite(GPIO_RELAY, HIGH);
    };

    void stopConveyor()
    {
        digitalWrite(GPIO_RELAY, LOW);
    };
}