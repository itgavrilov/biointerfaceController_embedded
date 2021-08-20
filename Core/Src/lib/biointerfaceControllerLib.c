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

//#define TEST_PROTOCOL

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
void genTestSignal(ChennalPacket_t *data, uint8_t numberChannel){
	static uint8_t point[MAX_NUMBER_OF_CHENNAL] = {0};
	int32_t tmp = 0;
	
	if(numberChannel < 4)
		tmp += (testSignalTable[numberChannel][point[numberChannel]] << 24);
	else
		tmp += (testSignalTable[4 - (numberChannel-4)][point[numberChannel]] << 24);
	
	data->ch[numberChannel].value = tmp;
	
	point[numberChannel]++;
} 
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void sendToSerialPort(){
	if(dataIsReady){
		dataIsReady = 0;
		sendDataFromChannels();
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	
  /* USER CODE END Callback 0 */
//  if (htim->Instance == TIM1) {
//    HAL_IncTick();
//  }
  /* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM2) {
		sendToSerialPort();
  }
  /* USER CODE END Callback 1 */
}

//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void setChannelADC(void){
	if(!adc.waitingMeasurement){
		ADS125x_Channel_Set(&adc, adc.numberOfChennal << 4);
		adc.waitingMeasurement = 1;
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void getResultADC(void){	
	if(adc.waitingMeasurement){
		int tmp = ADS125x_read_int32(&adc);
		
		#ifdef TEST_PROTOCOL
		genTestSignal(&controllerStatus.chennals, adc.numberOfChennal);
		#else
		controllerStatus.chennals.ch[adc.numberOfChennal].value = (tmp-4194303)<<9;
		#endif
		
		if(++adc.numberOfChennal > 7){
			HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
			dataIsReady = 1;
			adc.numberOfChennal = 0;
		}
		adc.waitingMeasurement = 0;
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == ADS1256_DRDY_Pin){
		if(adc.isInit){
			if(HAL_GPIO_ReadPin(adc.drdyPort, adc.drdyPin) == GPIO_PIN_SET){
				getResultADC();
			} else {
				setChannelADC();
			}
		}
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
		config.enableChannels = 0xFF;
	}
	
	for(uint8_t i = 0; i < MAX_NUMBER_OF_CHENNAL; i++){
		controllerStatus.chennals.ch[i].index = i;
	}
	adc.csPort = ADS1256_CS_GPIO_Port;
	adc.csPin  = ADS1256_CS_Pin;
	
	adc.drdyPort = ADS1256_DRDY_GPIO_Port;
	adc.drdyPin  = ADS1256_DRDY_Pin;
	adc.oscFreq = ADS125x_OSC_FREQ;
	
	ADS125x_Init(&adc, &hspi1, ADS125x_DRATE_30000SPS, ADS125x_PGA1, 0);
	
	HAL_TIM_Base_Start_IT(&htim2);
}
/************************ (C) Gavrilov S.A. (itgavrilov@gmail.com) ************************/
/****END OF FILE****/
