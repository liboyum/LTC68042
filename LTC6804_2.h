#ifndef LTC68042_H
#define LTC68042_H

#include <stdint.h>
#include <stdlib.h>

void isoSPI_WakeUp_Write();

// void LTC68042_Address_RDCFG();

// void LTC68042_Broadcast_WRCFG();

void LTC68042_Broadcast_ADCV();

void LTC68042_Address_RDCVA();

void LTC68042_Broadcast_CLRCELL();

#endif