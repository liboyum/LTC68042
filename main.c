#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define output_low(pin)    digitalWrite(pin, LOW);
#define output_high(pin)   digitalWrite(pin, HIGH);
#define input(pin)         digitalRead(pin);

#define CSB  10
#define MISO 12 
#define MOSI 13
#define SCK  14

#define CHANNEL 0
#define SPEED   500000

void isoSPI_WakeUp_Write();

void LTC68042_Broadcast_ADCV();

void LTC68042_Address_RDCVA();

void LTC68042_Broadcast_CLRCELL();

int main(void)
{

	printf("Raspberry Pi LTC6804-2 voltage test program");
	if(wiringPiSetup() == -1)
	{
		exit(1);
	}
	LTC68042_Broadcast_ADCV();
	delay(500);
	LTC68042_Address_RDCVA();
	delay(500);
	LTC68042_Broadcast_CLRCELL();
	return 0;
}

void isoSPI_WakeUp_Write()
{
	int8_t isoSPIMode = 1;
	int8_t isoSPI_IDLE = 1;
	int8_t core_SLEEP = 1;
	int8_t daisyChain = 0;
	uint8_t totalDevices = 1;
	//Trigger to output a Wake-Up byte
	int8_t sendWakeUp = 0;

	//Variables to set delay time ensuring isoSPI state is READY
	uint16_t isoWakeUpTime = 0;
	uint8_t devicesConnected = 0;
	uint16_t totalisoWakeUpTime = 0;

	if(isoSPIMode)
	{
		if(isoSPI_IDLE)
		{
			sendWakeUp = 1;
			if(core_SLEEP)
			{
				isoWakeUpTime = 300;
			}
			else if(!core_SLEEP)
			{
				isoWakeUpTime = 10;
			}
			if(daisyChain)
			{
				devicesConnected = totalDevices;
			}
			else if(!daisyChain)
			{
				devicesConnected = 1;
			}
			totalisoWakeUpTime = (isoWakeUpTime * devicesConnected);
		}
		else if(!isoSPI_IDLE)
		{
			sendWakeUp = 0;
		}
	}
	else if(!isoSPIMode)
	{
		sendWakeUp = 0;
	}
	if(sendWakeUp)
	{
		output_low(CSB);
		//spi_write(0x00);
		wiringPiSPIDataRW(CHANNEL, 0x00, 1);
		output_high(CSB);
		delayMicroseconds(totalisoWakeUpTime);
	}
}

void LTC68042_Broadcast_ADCV()
{
	uint8_t cmd[4] = {0x03, 0x70, 0xAF, 0x42};
	isoSPI_WakeUp_Write();
	output_low(CSB);
	wiringPiSPIDataRW(CHANNEL, cmd, 4);
	// spi_write(0x03);
	// spi_write(0x70);
	// spi_write(0xAF);
	// spi_write(0x42);	
	output_high(CSB);
}

void LTC68042_Address_RDCVA()
{
	uint8_t cmd[4] = {0x80, 0x04, 0x84, 0x28};
	uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t dataIndex = 0;
	isoSPI_WakeUp_Write();
	output_low(CSB);
	wiringPiSPIDataRW(CHANNEL, cmd, 4);
	delay(500);
	// spi_write(0x80);
	// spi_write(0x04);
	// spi_write(0x84);
	// spi_write(0x28);
	wiringPiSPIDataRW(CHANNEL, data, 8);
	for(dataIndex = 0; dataIndex < 8; dataIndex++)
	{
		printf("The voltage is %d \n", data[dataIndex]);
	}
	output_high(CSB);
}	

void LTC68042_Broadcast_CLRCELL()
{
	uint8_t cmd[4] = {0x07, 0x11, 0xC9, 0xC0};
	isoSPI_WakeUp_Write();
	output_low(CSB);
	wiringPiSPIDataRW(CHANNEL, cmd, 4);
	// spi_write(0x07);
	// spi_write(0x11);
	// spi_write(0xC9);
	// spi_write(0xC0);	
	output_high(CSB);	
}
