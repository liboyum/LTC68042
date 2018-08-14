#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#define output_low(pin)    digitalWrite(pin, LOW);
#define output_high(pin)   digitalWrite(pin, HIGH);
#define input(pin)         digitalRead(pin);

#define CE0  10
#define MISO 12 
#define MOSI 13
#define SCK  14
#define LTC6804_CS CE0

#define CHANNEL 1
#define SPEED   1000000

static const unsigned int crc15Table[256] = {0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,  //!<precomputed CRC15 Table
0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 
0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 
0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 
0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d, 
0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 
0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921, 
0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 
0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 
0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 
0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 
0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 
0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 
0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 
0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453, 
0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 
0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 
0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095}; 


 /*! 
 
  |MD| Dec  | ADC Conversion Model|
  |--|------|---------------------|
  |01| 1    | Fast 			   	  |
  |10| 2    | Normal 			  |
  |11| 3    | Filtered 		   	  |
*/
#define MD_FAST 1
#define MD_NORMAL 2
#define MD_FILTERED 3


 /*! 
 |CH | Dec  | Channels to convert |
 |---|------|---------------------|
 |000| 0    | All Cells  		  |
 |001| 1    | Cell 1 and Cell 7   |
 |010| 2    | Cell 2 and Cell 8   |
 |011| 3    | Cell 3 and Cell 9   |
 |100| 4    | Cell 4 and Cell 10  |
 |101| 5    | Cell 5 and Cell 11  |
 |110| 6    | Cell 6 and Cell 12  |
*/

#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6


/*!
  |CHG | Dec  |Channels to convert   | 
  |----|------|----------------------|
  |000 | 0    | All GPIOS and 2nd Ref| 
  |001 | 1    | GPIO 1 			     |
  |010 | 2    | GPIO 2               |
  |011 | 3    | GPIO 3 			  	 |
  |100 | 4    | GPIO 4 			  	 |
  |101 | 5    | GPIO 5 			 	 |
  |110 | 6    | Vref2  			  	 |
*/

#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6

//uint8_t CHG = 0; //!< aux channels to be converted
 /*!****************************************************
  \brief Controls if Discharging transitors are enabled
  or disabled during Cell conversions.
  
 |DCP | Discharge Permitted During conversion |
 |----|----------------------------------------|
 |0   | No - discharge is not permitted         |
 |1   | Yes - discharge is permitted           |
 
********************************************************/
#define DCP_DISABLED 0
#define DCP_ENABLED 1

#define TOTAL_IC 1
uint16_t cell_codes[TOTAL_IC][12];
int rdError = 0;
int setupError = 0;
int spiError = 0;

void LTC6804_initialize();

void set_adc(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG);

void LTC6804_adcv(); 

void LTC6804_adax();

uint8_t LTC6804_rdcv(uint8_t reg, uint8_t total_ic, uint16_t cell_codes[][12]);

void LTC6804_rdcv_reg(uint8_t reg, uint8_t total_ic, uint8_t *data);

int8_t LTC6804_rdaux(uint8_t reg, uint8_t total_ic, uint16_t aux_codes[][6]);

void LTC6804_rdaux_reg(uint8_t reg, uint8_t total_ic,uint8_t *data);

void LTC6804_clrcell();

void LTC6804_clraux();

void LTC6804_wrcfg(uint8_t total_ic,uint8_t config[][6]);

int8_t LTC6804_rdcfg(uint8_t total_ic, uint8_t r_config[][8]);

void wakeup_idle();

void wakeup_sleep();

uint16_t pec15_calc(uint8_t len, uint8_t *data);

void spi_write_array( uint8_t len, uint8_t *data);

void spi_write_read(uint8_t *tx_Data, uint8_t tx_len, uint8_t *rx_data, uint8_t rx_len);

void print_voltage();

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

uint8_t ADCV[2]; //!< Cell Voltage conversion command.
uint8_t ADAX[2]; //!< GPIO conversion command.
/*!
  \brief This function will initialize all 6804 variables and the SPI port.
  input: 
  ------
  IC: number of ICs being controlled. The address of the ICs in a LTC6804-2 network will start at 0 and continue in an ascending order.
*/
void LTC6804_initialize()
{
  setupError = wiringPiSetup();
  if(setupError == -1){
	  printf("WiringPi setup failed\n");
  }
  spiError = wiringPiSPISetup(CHANNEL, SPEED);
  if(spiError == -1){
	printf("SPI setup failed\n");
  }
  set_adc(MD_NORMAL,DCP_DISABLED,CELL_CH_ALL,AUX_CH_ALL);
}

/*!******************************************************************************************************************
 \brief Maps  global ADC control variables to the appropriate control bytes for each of the different ADC commands
 
@param[in] uint8_t MD The adc conversion mode
@param[in] uint8_t DCP Controls if Discharge is permitted during cell conversions
@param[in] uint8_t CH Determines which cells are measured during an ADC conversion command
@param[in] uint8_t CHG Determines which GPIO channels are measured during Auxiliary conversion command
 
 Command Code: \n
			|command	|  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   | 
			|-----------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
			|ADCV:	    |   0   |   1   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CH[2] | CH[1] | CH[0] | 
			|ADAX:	    |   1   |   0   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CHG[2]| CHG[1]| CHG[0]| 
 ******************************************************************************************************************/
void set_adc(uint8_t MD, //ADC Mode
			 uint8_t DCP, //Discharge Permit
			 uint8_t CH, //Cell Channels to be measured
			 uint8_t CHG //GPIO Channels to be measured
			 )
{
  uint8_t md_bits;
  
  md_bits = (MD & 0x02) >> 1;
  ADCV[0] = md_bits + 0x02;
  md_bits = (MD & 0x01) << 7;
  ADCV[1] =  md_bits + 0x60 + (DCP<<4) + CH;
 
  md_bits = (MD & 0x02) >> 1;
  ADAX[0] = md_bits + 0x04;
  md_bits = (MD & 0x01) << 7;
  ADAX[1] = md_bits + 0x60 + CHG ;
  
}


/*!*********************************************************************************************
  \brief Starts cell voltage conversion
  
  Starts ADC conversions of the LTC6804 Cpin inputs.
  The type of ADC conversion done is set using the associated global variables:
 |Variable|Function                                      | 
 |--------|----------------------------------------------|
 | MD     | Determines the filter corner of the ADC      |
 | CH     | Determines which cell channels are converted |
 | DCP    | Determines if Discharge is Permitted	     |
  
***********************************************************************************************/
void LTC6804_adcv()
{

  uint8_t cmd[4];
  uint16_t temp_pec;
  
  //1
  cmd[0] = ADCV[0];
  cmd[1] = ADCV[1];
  
  //2
  temp_pec = pec15_calc(2, ADCV);
  cmd[2] = (uint8_t)(temp_pec >> 8);
  cmd[3] = (uint8_t)(temp_pec);
  
  //3
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  
  //4
  output_low(LTC6804_CS);
  spi_write_array(4,cmd);
  output_high(LTC6804_CS);

}
/*
  LTC6804_adcv Function sequence:
  
  1. Load adcv command into cmd array
  2. Calculate adcv cmd PEC and load pec into cmd array
  3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
  4. send broadcast adcv command to LTC6804 stack
*/


/*!******************************************************************************************************
 \brief Start an GPIO Conversion
 
  Starts an ADC conversions of the LTC6804 GPIO inputs.
  The type of ADC conversion done is set using the associated global variables:
 |Variable|Function                                      | 
 |--------|----------------------------------------------|
 | MD     | Determines the filter corner of the ADC      |
 | CHG    | Determines which GPIO channels are converted |
 
*********************************************************************************************************/
void LTC6804_adax()
{
  uint8_t cmd[4];
  uint16_t temp_pec;
 
  cmd[0] = ADAX[0];
  cmd[1] = ADAX[1];
  temp_pec = pec15_calc(2, ADAX);
  cmd[2] = (uint8_t)(temp_pec >> 8);
  cmd[3] = (uint8_t)(temp_pec);
 
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  output_low(LTC6804_CS);
  spi_write_array(4,cmd);
  output_high(LTC6804_CS);

}
/*
  LTC6804_adax Function sequence:
  
  1. Load adax command into cmd array
  2. Calculate adax cmd PEC and load pec into cmd array
  3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
  4. send broadcast adax command to LTC6804 stack
*/


/***********************************************//**
 \brief Reads and parses the LTC6804 cell voltage registers.
 
 The function is used to read the cell codes of the LTC6804.
 This function will send the requested read commands parse the data
 and store the cell voltages in cell_codes variable. 
 
 
@param[in] uint8_t reg; This controls which cell voltage register is read back. 
 
          0: Read back all Cell registers 
		  
          1: Read back cell group A 
		  
          2: Read back cell group B 
		  
          3: Read back cell group C 
		  
          4: Read back cell group D 
 
@param[in] uint8_t total_ic; This is the number of ICs in the network
 
@param[out] uint16_t cell_codes[]; An array of the parsed cell codes from lowest to highest. The cell codes will
  be stored in the cell_codes[] array in the following format:
  |  cell_codes[0]| cell_codes[1] |  cell_codes[2]|    .....     |  cell_codes[11]|  cell_codes[12]| cell_codes[13] |  .....   |
  |---------------|----------------|--------------|--------------|----------------|----------------|----------------|----------|
  |IC1 Cell 1     |IC1 Cell 2      |IC1 Cell 3    |    .....     |  IC1 Cell 12   |IC2 Cell 1      |IC2 Cell 2      | .....    |
 
 @return int8_t, PEC Status.
 
	0: No PEC error detected
  
	-1: PEC error detected, retry read
 *************************************************/
uint8_t LTC6804_rdcv(uint8_t reg,
					 uint8_t total_ic,
					 uint16_t cell_codes[][12]
					 )
{
  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  const uint8_t CELL_IN_REG = 3;
  
  uint8_t *cell_data;
  int8_t pec_error = 0;
  uint16_t parsed_cell;
  uint16_t received_pec;
  uint16_t data_pec;
  uint8_t data_counter=0; //data counter
  cell_data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
  //1.a
  if (reg == 0)
  {
    //a.i
    for(uint8_t cell_reg = 1; cell_reg<5; cell_reg++)         			 //executes once for each of the LTC6804 cell voltage registers
    {
      data_counter = 0;
      LTC6804_rdcv_reg(cell_reg, total_ic,cell_data);
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
      {																 	  // current_ic is used as an IC counter
        //a.ii
		  for(uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++)	 								  // This loop parses the read back data. Loops 
        {														   		  // once for each cell voltages in the register 
          parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);
          cell_codes[current_ic][current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
          data_counter = data_counter + 2;
        }
		//a.iii
        received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter+1];
        data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT ]);
        if(received_pec != data_pec)
        {
          pec_error--;//pec_error = -1;
        }
        data_counter=data_counter+2;
      }
    }
  }
 //1.b
  else
  {
	//b.i
	
    LTC6804_rdcv_reg(reg, total_ic,cell_data);
    for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
    {							   									// current_ic is used as an IC counter
		//b.ii
		for(uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++)   									// This loop parses the read back data. Loops 
		{						   									// once for each cell voltage in the register 
			parsed_cell = cell_data[data_counter] + (cell_data[data_counter+1]<<8);
			cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] = 0x0000FFFF & parsed_cell;
			data_counter= data_counter + 2;
		}
		//b.iii
	    received_pec = (cell_data[data_counter] << 8 )+ cell_data[data_counter + 1];
        data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT * (reg-1)]);
		if(received_pec != data_pec)
		{
			pec_error--;//pec_error = -1;
		}
	}
  }
 free(cell_data);
 //2
return(pec_error);
}
/*
	LTC6804_rdcv Sequence
	
	1. Switch Statement:
		a. Reg = 0
			i. Read cell voltage registers A-D for every IC in the stack
			ii. Parse raw cell voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
		b. Reg != 0 
			i.Read single cell voltage register for all ICs in stack
			ii. Parse raw cell voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
	2. Return pec_error flag
*/


/***********************************************//**
 \brief Read the raw data from the LTC6804 cell voltage register
 
 The function reads a single cell voltage register and stores the read data
 in the *data point as a byte array. This function is rarely used outside of 
 the LTC6804_rdcv() command. 
 
 @param[in] uint8_t reg; This controls which cell voltage register is read back. 
         
          1: Read back cell group A 
		  
          2: Read back cell group B 
		  
          3: Read back cell group C 
		  
          4: Read back cell group D 
		  
 @param[in] uint8_t total_ic; This is the number of ICs in the network
 
 @param[out] uint8_t *data; An array of the unparsed cell codes 
 *************************************************/
void LTC6804_rdcv_reg(uint8_t reg,
					  uint8_t total_ic, 
					  uint8_t *data
					  )
{
  uint8_t cmd[4];
  uint16_t temp_pec;
  
  //1
  if (reg == 1)
  {
    cmd[1] = 0x04;
    cmd[0] = 0x00;
  }
  else if(reg == 2)
  {
    cmd[1] = 0x06;
    cmd[0] = 0x00;
  } 
  else if(reg == 3)
  {
    cmd[1] = 0x08;
    cmd[0] = 0x00;
  } 
  else if(reg == 4)
  {
    cmd[1] = 0x0A;
    cmd[0] = 0x00;
  } 

  //2
 
  
  //3
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  
  //4
  for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    temp_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(temp_pec >> 8);
	cmd[3] = (uint8_t)(temp_pec); 
	output_low(LTC6804_CS);
	spi_write_read(cmd,4,&data[current_ic*8],8);
	output_high(LTC6804_CS);
  }
}
/*
  LTC6804_rdcv_reg Function Process:
  1. Determine Command and initialize command array
  2. Calculate Command PEC
  3. Wake up isoSPI, this step is optional
  4. Send Global Command to LTC6804 stack
*/


/***********************************************************************************//**
 \brief Reads and parses the LTC6804 auxiliary registers.
 
 The function is used
 to read the  parsed GPIO codes of the LTC6804. This function will send the requested 
 read commands parse the data and store the gpio voltages in aux_codes variable 
 
 @param[in] uint8_t reg; This controls which GPIO voltage register is read back. 
		  
          0: Read back all auxiliary registers 
		  
          1: Read back auxiliary group A 
		  
          2: Read back auxiliary group B 
		  
 
 @param[in] uint8_t total_ic; This is the number of ICs in the network
 @param[out] uint8_t aux_codes[]; An array of the aux codes from lowest to highest. The GPIO codes will
 be stored in the aux_codes[] array in the following format:
 |  aux_codes[0]| aux_codes[1] |  aux_codes[2]|  aux_codes[3]|  aux_codes[4]|  aux_codes[5]| aux_codes[6] |aux_codes[7]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|------------|-----------|
 |IC1 GPIO1     |IC1 GPIO2     |IC1 GPIO3     |IC1 GPIO4     |IC1 GPIO5     |IC1 Vref2     |IC2 GPIO1     |IC2 GPIO2   |  .....    |
 
 
 @return int8_t, PEC Status.
 
	0: No PEC error detected
  
	-1: PEC error detected, retry read
 *************************************************/
int8_t LTC6804_rdaux(uint8_t reg,
					 uint8_t total_ic, 
					 uint16_t aux_codes[][6]
					 )
{


  const uint8_t NUM_RX_BYT = 8;
  const uint8_t BYT_IN_REG = 6;
  const uint8_t GPIO_IN_REG = 3;
  
  uint8_t *data;
  uint8_t data_counter = 0; 
  int8_t pec_error = 0;
  uint16_t received_pec;
  uint16_t data_pec;
  data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
  //1.a
  if (reg == 0)
  {
	//a.i
    for(uint8_t gpio_reg = 1; gpio_reg<3; gpio_reg++)		 	   		 //executes once for each of the LTC6804 aux voltage registers
    {
      data_counter = 0;
      LTC6804_rdaux_reg(gpio_reg, total_ic,data);
      for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++) // This loop executes once for each LTC6804
      {									  								 // current_ic is used as an IC counter
        //a.ii
		for(uint8_t current_gpio = 0; current_gpio< GPIO_IN_REG; current_gpio++)	// This loop parses GPIO voltages stored in the register
        {								   													
          
          aux_codes[current_ic][current_gpio +((gpio_reg-1)*GPIO_IN_REG)] = data[data_counter] + (data[data_counter+1]<<8);
          data_counter=data_counter+2;
		  
        }
		//a.iii
        received_pec = (data[data_counter]<<8)+ data[data_counter+1];
        data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT*(gpio_reg-1)]);
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
       
        data_counter=data_counter+2;
      }
   

    }
  
  }
  else
  {
	//b.i
    LTC6804_rdaux_reg(reg, total_ic, data);
    for (int current_ic = 0 ; current_ic < total_ic; current_ic++) // executes for every LTC6804 in the stack
    {							   // current_ic is used as an IC counter
		//b.ii
		for(int current_gpio = 0; current_gpio<GPIO_IN_REG; current_gpio++)   // This loop parses the read back data. Loops 
		{						   // once for each aux voltage in the register 
			aux_codes[current_ic][current_gpio +((reg-1)*GPIO_IN_REG)] = 0x0000FFFF & (data[data_counter] + (data[data_counter+1]<<8));
			data_counter=data_counter+2;
		}
		//b.iii
		received_pec = (data[data_counter]<<8) + data[data_counter+1];
        data_pec = pec15_calc(6, &data[current_ic*8*(reg-1)]);
        if(received_pec != data_pec)
        {
          pec_error = -1;
        }
	}
  }
  free(data);
  return (pec_error);
}
/*
	LTC6804_rdaux Sequence
	
	1. Switch Statement:
		a. Reg = 0
			i. Read GPIO voltage registers A-D for every IC in the stack
			ii. Parse raw GPIO voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
		b. Reg != 0 
			i.Read single GPIO voltage register for all ICs in stack
			ii. Parse raw GPIO voltage data in cell_codes array
			iii. Check the PEC of the data read back vs the calculated PEC for each read register command
	2. Return pec_error flag
*/


/***********************************************//**
 \brief Read the raw data from the LTC6804 auxiliary register
 
 The function reads a single GPIO voltage register and stores thre read data
 in the *data point as a byte array. This function is rarely used outside of 
 the LTC6804_rdaux() command. 
 
 @param[in] uint8_t reg; This controls which GPIO voltage register is read back. 
		  
          1: Read back auxiliary group A
		  
          2: Read back auxiliary group B 
         
 @param[in] uint8_t total_ic; This is the number of ICs in the stack
 
 @param[out] uint8_t *data; An array of the unparsed aux codes 
 *************************************************/
void LTC6804_rdaux_reg(uint8_t reg, 
					   uint8_t total_ic,
					   uint8_t *data
					   )
{
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  //1
  if (reg == 1)
  {
    cmd[1] = 0x0C;
    cmd[0] = 0x00;
  }
  else if(reg == 2)
  {
    cmd[1] = 0x0e;
    cmd[0] = 0x00;
  } 
  else
  {
     cmd[1] = 0x0C;
     cmd[0] = 0x00;
  }
  //2
  cmd_pec = pec15_calc(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
  //3
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake, this command can be removed.
  //4
   for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec); 
	output_low(LTC6804_CS);
	spi_write_read(cmd,4,&data[current_ic*8],8);
	output_high(LTC6804_CS);
  }
}
/*
  LTC6804_rdaux_reg Function Process:
  1. Determine Command and initialize command array
  2. Calculate Command PEC
  3. Wake up isoSPI, this step is optional
  4. Send Global Command to LTC6804 stack
*/

/********************************************************//**
 \brief Clears the LTC6804 cell voltage registers
 
 The command clears the cell voltage registers and intiallizes 
 all values to 1. The register will read back hexadecimal 0xFF
 after the command is sent.
************************************************************/
void LTC6804_clrcell()
{
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  //1
  cmd[0] = 0x07;
  cmd[1] = 0x11;
  
  //2
  cmd_pec = pec15_calc(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec );
  
  //3
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  
  //4
  output_low(LTC6804_CS);
  spi_write_read(cmd,4,0,0);
  output_high(LTC6804_CS);
}
/*
  LTC6804_clrcell Function sequence:
  
  1. Load clrcell command into cmd array
  2. Calculate clrcell cmd PEC and load pec into cmd array
  3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
  4. send broadcast clrcell command to LTC6804 stack
*/


/***********************************************************//**
 \brief Clears the LTC6804 Auxiliary registers
 
 The command clears the Auxiliary registers and intiallizes 
 all values to 1. The register will read back hexadecimal 0xFF
 after the command is sent.
***************************************************************/
void LTC6804_clraux()
{
  uint8_t cmd[4];
  uint16_t cmd_pec;
  
  //1
  cmd[0] = 0x07;
  cmd[1] = 0x12;
  
  //2
  cmd_pec = pec15_calc(2, cmd);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
  
  //3
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake.This command can be removed.
  //4
  output_low(LTC6804_CS);
  spi_write_read(cmd,4,0,0);
  output_high(LTC6804_CS);
}
/*
  LTC6804_clraux Function sequence:
  
  1. Load clraux command into cmd array
  2. Calculate clraux cmd PEC and load pec into cmd array
  3. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
  4. send broadcast clraux command to LTC6804 stack
*/


/*****************************************************//**
 \brief Write the LTC6804 configuration register 
 
 This command will write the configuration registers of the stacks 
 connected in a stack stack. The configuration is written in descending 
 order so the last device's configuration is written first.
 
@param[in] uint8_t total_ic; The number of ICs being written. 
 
@param[in] uint8_t *config an array of the configuration data that will be written, the array should contain the 6 bytes for each
 IC in the stack. The lowest IC in the stack should be the first 6 byte block in the array. The array should
 have the following format:
 |  config[0]| config[1] |  config[2]|  config[3]|  config[4]|  config[5]| config[6] |  config[7]|  config[8]|  .....    |
 |-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
 |IC1 CFGR0  |IC1 CFGR1  |IC1 CFGR2  |IC1 CFGR3  |IC1 CFGR4  |IC1 CFGR5  |IC2 CFGR0  |IC2 CFGR1  | IC2 CFGR2 |  .....    |
 
 The function will calculate the needed PEC codes for the write data
 and then transmit data to the ICs on a stack.
********************************************************/
void LTC6804_wrcfg(uint8_t total_ic,uint8_t config[][6])
{
  const uint8_t BYTES_IN_REG = 6;
  const uint8_t CMD_LEN = 4+(8*total_ic);
  uint8_t *cmd;
  uint16_t temp_pec;
  uint8_t cmd_index; //command counter
  
  cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
  //1
  cmd[0] = 0x00;
  cmd[1] = 0x01;
  cmd[2] = 0x3d;
  cmd[3] = 0x6e;
  
  //2
  cmd_index = 4;
  for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--) 			// executes for each LTC6804 in stack, 
  {								
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) // executes for each byte in the CFGR register
    {							// i is the byte counter
	
      cmd[cmd_index] = config[current_ic-1][current_byte]; 		//adding the config data to the array to be sent 
      cmd_index = cmd_index + 1;                
    }
	//3
    temp_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic-1][0]);// calculating the PEC for each board
    cmd[cmd_index] = (uint8_t)(temp_pec >> 8);
    cmd[cmd_index + 1] = (uint8_t)temp_pec;
    cmd_index = cmd_index + 2;
  }
  
  //4
  wakeup_idle (); 															 //This will guarantee that the LTC6804 isoSPI port is awake.This command can be removed.
  //5
   for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    temp_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(temp_pec >> 8);
	cmd[3] = (uint8_t)(temp_pec); 
	output_low(LTC6804_CS);
	spi_write_array(4,cmd);
	spi_write_array(8,&cmd[4+(8*current_ic)]);
	output_high(LTC6804_CS);
  }
  free(cmd);
}
/*
	1. Load cmd array with the write configuration command and PEC
	2. Load the cmd with LTC6804 configuration data
	3. Calculate the pec for the LTC6804 configuration data being transmitted
	4. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
	5. Write configuration of each LTC6804 on the stack
*/

/*!******************************************************
 \brief Reads configuration registers of a LTC6804 stack
 
@param[in] uint8_t total_ic: number of ICs in the stack
@param[out] uint8_t *r_config: array that the function will write configuration data to. The configuration data for each IC 
is stored in blocks of 8 bytes with the configuration data of the lowest IC on the stack in the first 8 bytes 
of the array, the second IC in the second 8 byte etc. Below is an table illustrating the array organization:
|r_config[0]|r_config[1]|r_config[2]|r_config[3]|r_config[4]|r_config[5]|r_config[6]  |r_config[7] |r_config[8]|r_config[9]|  .....    |
|-----------|-----------|-----------|-----------|-----------|-----------|-------------|------------|-----------|-----------|-----------|
|IC1 CFGR0  |IC1 CFGR1  |IC1 CFGR2  |IC1 CFGR3  |IC1 CFGR4  |IC1 CFGR5  |IC1 PEC High |IC1 PEC Low |IC2 CFGR0  |IC2 CFGR1  |  .....    |
@return int8_t PEC Status.
	0: Data read back has matching PEC
 
	-1: Data read back has incorrect PEC
********************************************************/
int8_t LTC6804_rdcfg(uint8_t total_ic, uint8_t r_config[][8])
{
  const uint8_t BYTES_IN_REG = 8;
  
  uint8_t cmd[4];
  uint8_t *rx_data;
  int8_t pec_error = 0; 
  uint16_t data_pec;
  uint16_t received_pec;
  rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
  //1
  cmd[0] = 0x00;
  cmd[1] = 0x02;
  cmd[2] = 0x2b;
  cmd[3] = 0x0A;
 
  //2
  wakeup_idle (); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
  //3
   for(int current_ic = 0; current_ic<total_ic; current_ic++)
  {
	cmd[0] = 0x80 + (current_ic<<3); //Setting address
    data_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(data_pec >> 8);
	cmd[3] = (uint8_t)(data_pec); 
	output_low(LTC6804_CS);
	spi_write_read(cmd,4,&rx_data[current_ic*8],8);
	output_high(LTC6804_CS);
  }
 
  for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++) //executes for each LTC6804 in the stack
  { 
    //4.a
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
    {
      r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
    }
    //4.b
    received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
    data_pec = pec15_calc(6, &r_config[current_ic][0]);
    if(received_pec != data_pec)
    {
      pec_error = -1;
    }  
  }
  free(rx_data);
  //5
  return(pec_error);
}
/*
	1. Load cmd array with the write configuration command and PEC
	2. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
	3. read configuration of each LTC6804 on the stack
	4. For each LTC6804 in the stack
	  a. load configuration data into r_config array
	  b. calculate PEC of received data and compare against calculated PEC
	5. Return PEC Error
*/

/*!****************************************************
  \brief Wake isoSPI up from idle state
 Generic wakeup commannd to wake isoSPI up out of idle
 *****************************************************/
void wakeup_idle()
{
  output_low(LTC6804_CS);
  delayMicroseconds(10); //Guarantees the isoSPI will be in ready mode
  output_high(LTC6804_CS);
}

/*!****************************************************
  \brief Wake the LTC6804 from the sleep state
  
 Generic wakeup commannd to wake the LTC6804 from sleep
 *****************************************************/
void wakeup_sleep()
{
  output_low(LTC6804_CS);
  delay(1); // Guarantees the LTC6804 will be in standby
  output_high(LTC6804_CS);
}
/*!**********************************************************
 \brief calaculates  and returns the CRC15
  
@param[in]  uint8_t len: the length of the data array being passed to the function
               
@param[in]  uint8_t data[] : the array of data that the PEC will be generated from
  
@return  The calculated pec15 as an unsigned int16_t
***********************************************************/
uint16_t pec15_calc(uint8_t len, uint8_t *data)
{
	uint16_t remainder,addr;
	
	remainder = 16;//initialize the PEC
	for(uint8_t i = 0; i<len;i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address 
		remainder = (remainder<<8)^crc15Table[addr];
	}
	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}


/*!
 \brief Writes an array of bytes out of the SPI port
 
 @param[in] uint8_t len length of the data array being written on the SPI port
 @param[in] uint8_t data[] the data array to be written on the SPI port
 
*/
void spi_write_array(uint8_t len, // Option: Number of bytes to be written on the SPI port
					 uint8_t *data //Array of bytes to be written on the SPI port
					 )
{
    wiringPiSPIDataRW(CHANNEL, data, len);
  // for(uint8_t i = 0; i < len; i++)
  // {
  //    spi_write((char)data[i]);
  // }
}
/*!
 \brief Writes and read a set number of bytes using the SPI port.
@param[in] uint8_t tx_data[] array of data to be written on the SPI port
@param[in] uint8_t tx_len length of the tx_data array
@param[out] uint8_t rx_data array that read data will be written too. 
@param[in] uint8_t rx_len number of bytes to be read from the SPI port.
*/

void spi_write_read(uint8_t *tx_Data,//array of data to be written on SPI port 
					uint8_t tx_len, //length of the tx data arry
					uint8_t *rx_data,//Input: array that will store the data read by the SPI port
					uint8_t rx_len //Option: number of bytes to be read from the SPI port
					)
{
    wiringPiSPIDataRW(CHANNEL, tx_Data, tx_len);
  // for(uint8_t i = 0; i < tx_len; i++)
  // {
  //  spi_write(tx_Data[i]);
  // }
    wiringPiSPIDataRW(CHANNEL, rx_data, 1);
  // for(uint8_t i = 0; i < rx_len; i++)
  // {
  //   rx_data[i] = (uint8_t)spi_read(0xFF);
  // }
}

void print_voltage()
{
    for(int i=0; i<12; i++)
    {
      printf("The voltage is %.2f\n", cell_codes[TOTAL_IC][i]*0.0001);
    }
}
