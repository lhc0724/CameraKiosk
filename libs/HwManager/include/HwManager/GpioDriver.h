#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <iostream>
#include <memory>
#include <wiringPi.h>

#define GPIO_IR       25  //input
#define GPIO_RELAY    24  //output
#define GPIO_PWRSIG   23  //input

// IR sensor On/Off definition
#define IR_ENABLE   0
#define IR_DISABLE  1

#define IR_FILTER_CNT  10

namespace CameraKiosk
{
    bool gpioSetup();
    void addInterrupt(int gpio, void interruptFunc());

    /* read function */
    int readIRSensor();
    int readPower();

    /* write function */
    void runConveyor();
    void stopConveyor();

};

#endif
