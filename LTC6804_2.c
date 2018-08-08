#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "LTC6804_2.h"
#include "LTC68042_PI.h"

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
		spi_write(0x00);
		output_high(CSB);
		delayMicroseconds(totalisoWakeUpTime);
	}
}

// void LTC68042_Address_RDCFG()
// {
// 	const uint8_t TOTALCOMMANDBYTES = 4;
// 	const uint8_t TOTALDATABYTES = 8;
// 	uint8_t dataIndex = 0;
// 	uint8_t ReadConfig_CommandArray[TOTALCOMMANDBYTES] = {0x88, 0x02, 0xA8, 0xE0}；
// 	uint8_t ReadConfig_DataArray[TOTALDATABYTES] = {0, 0, 0, 0, 0, 0, 0, 0}；

// 	isoSPI_WakeUp_Write();
// 	output_low(CSB);
// 	spi_write(ReadConfig_CommandArray[0]);
// 	spi_write(ReadConfig_CommandArray[1]);
// 	spi_write(ReadConfig_CommandArray[2]);
// 	spi_write(ReadConfig_CommandArray[3]);
// 	for(dataIndex = 0; dataIndex < TOTALDATABYTES; dataIndex++)
// 	{
// 		ReadConfig_DataArray[dataIndex] = spi_read(0);
// 	}
// 	output_high(CSB);
// }

// void LTC68042_Broadcast_WRCFG()
// {
// 	const uint8_t TOTALDEVICES = 3;
// 	const uint8_t TOTALDATABYTES = 8;
// 	uint8_t dataIndex = 0;
// 	uint8_t WriteConfig_DataArray[TOTALDATABYTES] = 
// 		{0xFC, 0x97, 0x16, 0xA4, 0x00, 0x00, 0xCD, 0x9E};
// 	isoSPI_WakeUp_Write();
// 	output_low(CSB);
// 	spi_write(0x00);
// 	spi_write(0x01);
// 	spi_write(0x3D);
// 	spi_write(0x6E);
// 	for(dataIndex = 0; dataIndex < TOTALDATABYTES; dataIndex++)
// 	{
// 		spi_write(WriteConfig_DataArray[dataIndex]);
// 	}
// 	output_high(CSB);
// }


void LTC68042_Broadcast_ADCV()
{
	isoSPI_WakeUp_Write();
	output_low(CSB);
	spi_write(0x03);
	spi_write(0x70);
	spi_write(0xAF);
	spi_write(0x42);	
	output_high(CSB);
}

void LTC68042_Address_RDCVA()
{
	const uint8_t TOTALCOMMANDBYTES = 4;
	const uint8_t TOTALDATABYTES = 8;
	uint8_t dataIndex = 0;
	uint8_t RDCVA_DataArray[TOTALDATABYTES];
	isoSPI_WakeUp_Write();
	output_low(CSB);
	spi_write(0x88);
	spi_write(0x04);
	spi_write(0x84);
	spi_write(0x28);
	for(dataIndex = 0; dataIndex < TOTALDATABYTES; dataIndex++)
	{
		RDCVA_DataArray[dataIndex] = spi_read(0);
		printf("The voltage is %d \n", 
			RDCVA_DataArray[dataIndex]);
	}
	output_high(CSB);
}	

void LTC68042_Broadcast_CLRCELL()
{
	isoSPI_WakeUp_Write();
	output_low(CSB);
	spi_write(0x07);
	spi_write(0x11);
	spi_write(0xC9);
	spi_write(0xC0);	
	output_high(CSB);	
}
