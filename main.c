#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "LTC6804_2.h"
#include "LTC68042_PI.h"

#define TOTAL_IC 1

void print_voltage();
uint16_t cell_codes[TOTAL_IC][12];
int rdError = 0;

int main(void)
{
	printf("Raspberry Pi LTC6804-2 voltage test program\n");
	LTC6804_initialize();
	LTC6804_adcv();
	rdError = LTC6804_rdcv(0, TOTAL_IC, cell_codes);
	if(rdError == -1){
		printf("A PEC error was detected in the received data\n");
	}
	else{
		print_voltage();
	}
	return 0;
}

void print_voltage()
{
    for(int i=0; i<12; i++)
    {
       printf("The voltage is %.4f\n", cell_codes[TOTAL_IC][i]*0.0001);
     }
}
