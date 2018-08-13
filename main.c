#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "LTC6804_2.h"
#include "LTC68042_PI.h"

int main(void)
{

	printf("Raspberry Pi LTC6804-2 voltage test program");
	uint16_t cell_codes[][12]={0,0,0,0,0,0,0,0,0,0,0,0};
	LTC6804_initialize();
        pinMode(SCK, OUTPUT);             //! 1) Setup SCK as output
        pinMode(MOSI, OUTPUT);            //! 2) Setup MOSI as output
        pinMode(LTC6804_CS, OUTPUT);      //! 3) Setup CS as output
        output_low(SCK);
        output_low(MOSI);
        output_high(LTC6804_CS);
	LTC6804_adcv();
	LTC6804_rdcv(0, 1, cell_codes);
	return 0;
}
