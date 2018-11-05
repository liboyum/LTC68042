#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

using namespace std;

static const int CHANNEL = 0;

int main()
{
	wiringPiSetup();
	wiringPiSPISetup(CHANNEL,500000);
	
	unsigned char cmd0[] = {0x03,0x60,0xf4,0x6c};
	wiringPiSPIDataRW(CHANNEL,cmd0,4);
	unsigned char cmd1[] = {0x80,0x04,0x77,0xd6};
	wiringPiSPIDataRW(CHANNEL,cmd1,4);
    
	uint8_t cell_data[8];
	cell_data[0] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[1] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[2] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[3] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[4] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[5] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[6] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	cell_data[7] = wiringPiSPIDataRW(CHANNEL,0xFF,1);
	
	printf("%d\n",cell_data[0]+(cell_data[1]<<8));
	printf("%d\n",cell_data[2]+(cell_data[3]<<8));
	printf("%d\n",cell_data[4]+(cell_data[5]<<8));
	
	return 0;
}
