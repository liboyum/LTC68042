#ifndef LTC68042_PI_H
#define LTC68042_PI_H

#include <wiringPi.h>
#include <wiringPiSPI.h>

#define output_low(pin)    digitalWrite(pin, LOW);
#define output_high(pin)   digitalWrite(pin, HIGH);
#define input(pin)         digitalRead(pin);

#define CHANNEL 0
#define SPEED   500000

#endif
