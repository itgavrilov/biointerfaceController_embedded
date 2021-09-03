/*******************************************************************************
 * @file        ads125x.h
 * @brief       C Library for ads1255/ads1256 family of Analog to Digital
 *              Conterters (ADC)
 * @details     This file implements the functionalities of the ADC.
 * @version     1.0
 * @author      Simon Burkhardt
 * @date        2020.05.22
 * @copyright   (c) 2020 eta systems GmbH
********************************************************************************
 * @note        Parts of this library are based on Adien Akhmads Arduino Library 

MIT License

Copyright (c) 2016 Adien Akhmad

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdint.h>
#include "ads125x.h"

//#define DEBUG_ADS1255

#ifdef DEBUG_ADS1255
#include <stdio.h>
#endif

void delay(){
	uint32_t counter = 160;
	while(--counter != 0);
}
//void ADS125x_EXTI_Callback(ADS125x_t *adc){
//	
//}

/**
  * @brief  waits for DRDY pin to go low
  * @param  *adc pointer to adc handle
  */
void ADS125x_DRDY_Wait(ADS125x_t *adc){
  while(HAL_GPIO_ReadPin(adc->drdyPort, adc->drdyPin) == GPIO_PIN_SET);
}

void ADS125x_DRDY_Wait_ReSet(ADS125x_t *adc){
  while(HAL_GPIO_ReadPin(adc->drdyPort, adc->drdyPin) == GPIO_PIN_RESET);
}

/**
  * @brief  toggles chip select pin of ADS
  * @param  *adc pointer to adc handle
  * @param  on [0 = unselect, 1 = select]
  */
void ADS125x_CS(ADS125x_t *adc, uint8_t on){
	GPIO_PinState pin = on?GPIO_PIN_RESET:GPIO_PIN_SET;
  HAL_GPIO_WritePin(adc->csPort, adc->csPin, pin);
}

/**
  * @brief  readc from internal registers
  * @param  *adc pointer to adc handle
  * @param  reg  first register to be read
  * @param  *pData pointer for return data
  * @param  n number of registers to read
  * @return 
  * @see    Datasheet Fig. 34 RREG Command Example
  */
uint8_t ADS125x_Register_Read(ADS125x_t *adc, uint8_t reg, uint8_t* pData, uint8_t n){
  uint8_t spiTx[2];
	spiTx[0] = ADS125x_CMD_RREG | reg; // 1st command byte
	spiTx[1] = n-1;                    // 2nd command byte = bytes to be read -1
	
  ADS125x_CS(adc, 1);
  HAL_SPI_Transmit(adc->hspix, spiTx, 2, 1);
  delay(); // t6 delay (50*tCLKIN)
  HAL_SPI_Receive(adc->hspix, pData, n, 1);
  ADS125x_CS(adc, 0);
	
	return 0;
}

/**
  * @brief  writes to internal registers
  * @param  *adc pointer to adc handle
  * @param  reg  first register to write to
  * @param  payload to write to register
  * @return 
  * @see    Datasheet Fig. 35 WREG Command Example
  */
void ADS125x_Register_Write(ADS125x_t *adc, uint8_t reg, uint8_t data) {
	uint8_t spiTx[3] = {
		ADS125x_CMD_WREG | reg, // 1st command byte
		0,                      // 2nd command byte = payload length = 1 bytes -1 = 0
		data,
	};
	
	HAL_SPI_Transmit(adc->hspix, spiTx, 3, 10);
}

/**
  * @brief  send a single command to the ADS
  * @param  *adc pointer to adc handle
  * @param  cmd the command
  * @return 
  * @see    Datasheet Fig. 33 SDATAC Command Sequence
  */
void ADS125x_CMD_Send(ADS125x_t *adc, uint8_t cmd){
  HAL_SPI_Transmit(adc->hspix, &cmd, 1, 1);
}
/**
  * @brief  resets, initializes and calibrates the ADS125x
  * @param  *adc pointer to adc handle
  * @param  drate the datarate of the converter
	* @param  gain  the gain of the PGA
	* @param  buffer_en [0 = off, 1 = on]
  * @return 
  * @see    Datasheet Fig. 33 SDATAC Command Sequence
  */
void ADS125x_Init(ADS125x_t *adc, SPI_HandleTypeDef *hspi, uint8_t drate, uint8_t gain, uint8_t buffer_en){
  adc->hspix = hspi;
  adc->pga = 1 << gain;
	
	ADS125x_CS(adc, 1);
	
	ADS125x_CMD_Send(adc, ADS125x_CMD_RESET);
	
	ADS125x_CMD_Send(adc, ADS125x_CMD_SDATAC);
#ifdef DEBUG_ADS1255
	uint8_t tmp[5]; // buffer
	ADS125x_Register_Read(adc, ADS125x_REG_STATUS, tmp, 1);
	printf("STATUS: %#.2x\n", tmp[0]);
#endif
	
	ADS125x_Register_Write(adc, ADS125x_REG_ADCON, ADS125x_CLKOUT_OFF | gain);  // enable clockout = clkin/1
#ifdef DEBUG_ADS1255
	ADS125x_Register_Read(adc, ADS125x_REG_ADCON, tmp, 1);
	printf("ADCON: %#.2x\n", tmp[0]);
#endif

  ADS125x_Register_Write(adc, ADS125x_REG_DRATE, drate);
#ifdef DEBUG_ADS1255
	ADS125x_Register_Read(adc, ADS125x_REG_DRATE, tmp, 1);
	printf("DRATE: %#.2x\n", tmp[0]);
#endif

	ADS125x_Register_Write(adc, ADS125x_REG_IO, 0x00); // all GPIOs are outputs (do not leave floating) - D0 is CLKOUT
#ifdef DEBUG_ADS1255
	ADS125x_Register_Read(adc, ADS125x_REG_IO, tmp, 1);
	printf("IO   : %#.2x\n", tmp[0]);
#endif

	ADS125x_CMD_Send(adc, ADS125x_CMD_SELFCAL);
	
	ADS125x_CS(adc, 0);
	
	adc->isInit = 1;
}


/**
  * @brief  set internal multiplexer to single channel with AINCOM as negative input
  * @param  *adc pointer to adc handle
  * @param  p_chan positive analog input
  * @see    Datasheet p. 31 MUX : Input Multiplexer Control Register (Address 01h)
  */
void ADS125x_Channel_Set(ADS125x_t *adc, int8_t channel){ 
  ADS125x_ChannelDiff_Set(adc, channel, ADS125x_MUXN_AINCOM);
}

/**
  * @brief  set internal multiplexer to differential input channel
  * @param  *adc pointer to adc handle
  * @param  p_chan positive analog input
  * @param  n_chan negative analog input
  * @see    Datasheet p. 31 MUX : Input Multiplexer Control Register (Address 01h)
  */
void ADS125x_ChannelDiff_Set(ADS125x_t *adc, int8_t p_chan, int8_t n_chan){
	ADS125x_DRDY_Wait(adc);
	ADS125x_CS(adc, 1);
  ADS125x_Register_Write(adc, ADS125x_REG_MUX, p_chan | n_chan);
	ADS125x_CMD_Send(adc, ADS125x_CMD_SELFOCAL);
  ADS125x_CMD_Send(adc, ADS125x_CMD_SYNC);
  ADS125x_CMD_Send(adc, ADS125x_CMD_WAKEUP);
	ADS125x_CMD_Send(adc, ADS125x_CMD_RDATAC);
	delay();
	ADS125x_CS(adc, 0);
}

int32_t ADS125x_read_int32 (ADS125x_t *adc){
	uint8_t spiRx[3] = {ADS125x_CMD_RDATA,0,0};
	
	ADS125x_CS(adc, 1);
	//HAL_SPI_Transmit(adc->hspix, spiRx, 1, 1);
	//delay();
	HAL_SPI_Receive(adc->hspix, spiRx, 3, 1);
	ADS125x_CS(adc, 0);
	
	return (spiRx[0] << 16) | (spiRx[1] << 8) | (spiRx[2]);
}

void ADS125x_Continuous(ADS125x_t *adc){
	ADS125x_CS(adc, 1);
  ADS125x_CMD_Send(adc, ADS125x_CMD_RDATAC);
	ADS125x_CS(adc, 0);
}

void ADS125x_Continuous_Channel_Set(ADS125x_t *adc, int8_t channel){
	ADS125x_CS(adc, 1);
  ADS125x_Register_Write(adc, ADS125x_REG_MUX, channel | ADS125x_MUXN_AINCOM);
	ADS125x_CS(adc, 0);
}

int32_t ADS125x_Continuous_read_int32 (ADS125x_t *adc){
	uint8_t spiRx[3] = {ADS125x_CMD_RDATA,0,0};
	
	ADS125x_CS(adc, 1);
	HAL_SPI_Receive(adc->hspix, spiRx, 3, 10);
	ADS125x_CS(adc, 0);
	
	return (spiRx[0] << 16) | (spiRx[1] << 8) | (spiRx[2]);
}



