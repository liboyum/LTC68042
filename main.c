#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "LTC6804_2.h"
#include "LTC68042_PI.h"

#define TOTAL_IC 1
uint16_t cell_codes[TOTAL_IC][12];
int rdError = 0;
int setupError = 0;
int spiError = 0;

int main(void)
{
	printf("Raspberry Pi LTC6804-2 voltage test program\n");
	LTC6804_initialize();
// 	LTC6804_wrcfg(TOTAL_IC,tx_cfg);
        pinMode(SCK, OUTPUT);             //! 1) Setup SCK as output
        pinMode(MOSI, OUTPUT);            //! 2) Setup MOSI as output
        pinMode(CE0, OUTPUT);      //! 3) Setup CS as output
//         output_low(SCK);
//         output_low(MOSI);
//         output_high(LTC6804_CS);
	LTC6804_adcv();
	rdError = LTC6804_rdcv(0, TOTAL_IC, cell_codes);
	if(rdError == -1){
		printf("A PEC error was detected in the received data\n");
	}
	else{
		print_voltage();
	}
	LTC6804_clrcell();
	return 0;
}
