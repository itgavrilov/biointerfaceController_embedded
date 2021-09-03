/**
	*******************************************************************************
	* @file    biointerfaceControllerLib.c																				*
	* @brief   Biointerface controller function library.													*
	*******************************************************************************
	*																																							*
	* COPYRIGHT(c) 2021 Gavrilov Stepan Alecsandrovich (itgavrilov@gmail.com)			*
	*																																							*
	*******************************************************************************
	*/
#include "biointerfaceControllerLib.h"
#include <stdlib.h> //malloc()
#include <string.h> //memcpy()
#include "usbd_cdc_if.h"
#include "tim.h"
#include "usart.h"
#include "spi.h"

#include "flash.h"
#include "ADS125x.h"                // Подключаем поддержку АЦП

#define TEST_PROTOCOL

ADS125x_t adc = {0};

status_t controllerStatus = {0};
uint8_t  dataIsReady = 0;

ConfigPacket_t config = {0};

//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void configWriteInFlash(){
	if(
		config.serialNumber 
		&& memcmp(&config, (void*)CONFIG_IN_FLASH, sizeof(ConfigPacket_t)) != 0
		&& flashStatus() == HAL_OK
		){
		flashWrite((unsigned)CONFIG_IN_FLASH, (void*)&config, sizeof(ConfigPacket_t));
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void genTestSignal(ChennalPacket_t *data){
	static uint8_t point = 0;
	
	for(uint8_t i = 0; i < MAX_NUMBER_OF_CHENNAL; i++){
		if(i < 5)
			data->ch[i].value = (testSignalTable[i][point]) << (2 * data->ch[i].scale);
		else
			data->ch[i].value = (testSignalTable[5 - (i-5)][point]) << (2 * data->ch[i].scale);
	}
	point++;
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void downsampling(){ // DDC FIR
	static uint16_t stepDecimation = 0;
	static ChennalPacket_t packetForMsg = {0};
	static int16_t x[MAX_NUMBER_OF_CHENNAL][FILTER_ORDER] = {0};
	
	

		for(uint8_t i = 0; i < MAX_NUMBER_OF_CHENNAL; i++){
			int32_t y = 0;
			packetForMsg.ch[i].scale = controllerStatus.chennals.ch[i].scale;
			
			for(uint8_t j = FILTER_ORDER-1; j > 0; j--){
				x[i][j] = x[i][j-1];
			}
			
			x[i][0] = controllerStatus.chennals.ch[i].value;
			
			
			for(uint8_t j = 0; j < FILTER_ORDER; j++)
				y += ddcFirNumerator[j] * x[i][j];
			
			packetForMsg.ch[i].value = y / 65536;
		}
	
		if(++stepDecimation == 6){
			stepDecimation = 0;
			
			if(controllerStatus.serialPort.sendChannelData){
				sendDataFromChannels(&packetForMsg);
				HAL_GPIO_TogglePin(LED_2_GPIO_Port, LED_2_Pin);
			} else 
				HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);
		}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /* USER CODE BEGIN Callback 0 */
	if (htim->Instance == TIM2) {
#ifdef TEST_PROTOCOL
		genTestSignal(&controllerStatus.chennals);
		HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
#else
		if(dataIsReady){
			dataIsReady = 0;
			sendDataFromChannels();
			HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
		}
#endif
		downsampling();
  }
  /* USER CODE END Callback 0 */

  /* USER CODE BEGIN Callback 1 */
	
  /* USER CODE END Callback 1 */
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void getResultADC(void){	
#if !defined(TEST_PROTOCOL)
	if(adc.waitingMeasurement){
		int tmp = ADS125x_read_int32(&adc);
		controllerStatus.chennals.ch[0].value = tmp;
//		if(++adc.numberOfChennal > 7){
			dataIsReady = 1;
//			adc.numberOfChennal = 6;
//		}
		//adc.waitingMeasurement = 0;
	}
#endif
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == adc.drdyPin){
		if(adc.isInit){
			getResultADC();
			delay();
		}
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void setChannelADC(void){
	if(!adc.waitingMeasurement){
		//ADS125x_Channel_Set(&adc, (adc.numberOfChennal) << 4);
		ADS125x_Channel_Set(&adc, ADS125x_MUXP_AIN7);
		adc.waitingMeasurement = 1;
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void initController(void){
	HAL_Delay(500);
	memcpy(&config, (void*)CONFIG_IN_FLASH, sizeof(ConfigPacket_t));
	if(config.serialNumber == 0 || config.serialNumber == 0xFFFF){
		config.numberOfChannels = MAX_NUMBER_OF_CHENNAL;
		config.serialNumber = 1;
	}
	
#if defined(TEST_PROTOCOL)
	for(uint8_t i = 0; i < MAX_NUMBER_OF_CHENNAL; i++){
		controllerStatus.chennals.ch[i].scale = scale16;
	}
#endif
	
	adc.csPort = ADS1256_CS_GPIO_Port;
	adc.csPin  = ADS1256_CS_Pin;
	
	adc.drdyPort = ADS1256_DRDY_GPIO_Port;
	adc.drdyPin  = ADS1256_DRDY_Pin;
	adc.oscFreq = ADS125x_OSC_FREQ;
	adc.numberOfChennal = 7;
	
	ADS125x_Init(&adc, &hspi1, ADS125x_DRATE_2000SPS, ADS125x_PGA1, 0);
#if !defined(TEST_PROTOCOL)
	setChannelADC();
#endif
	HAL_TIM_Base_Start_IT(&htim2);
}
/************************ (C) Gavrilov S.A. (itgavrilov@gmail.com) ************************/
/****END OF FILE****/
