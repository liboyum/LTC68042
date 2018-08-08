#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include "LTC6804_2.h"
#include "LTC68042_PI.h"

int main(void)
{
	printf("Raspberry Pi LTC6804-2 voltage test program");
	if(wiringPiSetUp() == -1)
		exit(1);
	while(1);
	{
		LTC68042_Broadcast_ADCV();
		delay(500);
		LTC68042_Address_RDCVA();
		delay(500);
	}
	LTC68042_Broadcast_CLRCELL();
	return 0;
}
