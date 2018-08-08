#ifndef LTC68042_PI_H
#define LTC68042_PI_H

#include <wiringPi.h>

#define output_low(pin)    digitalWrite(pin, LOW);
#define output_high(pin)   digitalWrite(pin, HIGH);
#define input(pin)         digitalRead(pin);

#define CSB  10
#define MISO 12 
#define MOSI 13
#define SCK  14

#define CHANNEL 0;
#define SPEED   500000
#endif