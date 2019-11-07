/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    biointerfaceControllerLib.c
  * @brief   Biointerface controller function library.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 Gavrilov Stepan Alecsandrovich (itgavrilov@gmail.com)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "biointerfaceControllerLib.h"
#include <stdlib.h> //malloc()
#include <string.h> //memcpy()
#include "usbd_cdc_if.h"

uint8_t testProtocol = 1;

extern USBD_HandleTypeDef hUsbDeviceFS;
extern ADC_HandleTypeDef hadc1, hadc2;
extern TIM_HandleTypeDef htim2, htim3, htim8;

#pragma pack (push, 1)
#pragma anon_unions
typedef union {
	struct {
		uint16_t start;
		uint8_t lenOfPack;
		uint8_t typeOfPack;
		union {
			uint32_t ch[COUNT_EMG_CHENNAL];
			servo_t sevo[COUNT_SERVO_CHENNAL];
		};
	};
} ms_t;
#pragma pack (pop)

ms_t message = {0};
status_t controllerStatus;

uint16_t *getValuesEMG;
uint16_t *getValuesServo;

void initController(void){
	getValuesEMG = malloc(COUNT_EMG_CHENNAL*sizeof(uint16_t));
	getValuesServo = malloc(COUNT_SERVO_CHENNAL*2*sizeof(uint16_t));
	
	memset(&controllerStatus, 0, sizeof(status_t));
	memset(getValuesEMG, 0, COUNT_EMG_CHENNAL * 2);
	memset(getValuesServo, 0, COUNT_SERVO_CHENNAL * 4);
	
	HAL_ADC_Start_DMA(&hadc1,	(uint32_t *)getValuesEMG, COUNT_EMG_CHENNAL);
	HAL_ADC_Start_DMA(&hadc2,	(uint32_t *)getValuesServo, COUNT_SERVO_CHENNAL*2);
	
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim8);
	HAL_TIM_Base_Start(&htim3);
}
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance==TIM2){
		
  }else if (htim->Instance==TIM3){
		
  }else if (htim->Instance==TIM8){
		
  }
}
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void genTestSignal(uint32_t *ch, uint8_t count){
	static uint8_t point=0;

	for(uint8_t i=0;i<count-1;i++){
		uint16_t tmp = 2047 + (testSignalTable[i][point]<<4);
		ch[i] = tmp;
	}
	ch[count-1] = (point << 4)^0xFFF;
	
	point++;
} 
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC1){
		memcpy(controllerStatus.valuesEMG, getValuesEMG, COUNT_EMG_CHENNAL);
		
		if(controllerStatus.receive){
			HAL_GPIO_WritePin(LED_2_GPIO_Port,LED_2_Pin, GPIO_PIN_SET);
			message.start = 0xFFFF;
			message.lenOfPack = COUNT_EMG_CHENNAL * sizeof(uint32_t) + 1;
			message.typeOfPack = 1;

			if(testProtocol){
				genTestSignal(message.ch, COUNT_EMG_CHENNAL);
			}else{
				for(uint8_t i=0; i < COUNT_EMG_CHENNAL; i++){
					message.ch[i] = controllerStatus.valuesEMG[i];
				}
			}
			
			if(CDC_Transmit_FS((void*)&message, 3 + message.lenOfPack) == USBD_FAIL){
				controllerStatus.receive = 0;
				return;
			}
			
			HAL_GPIO_WritePin(LED_2_GPIO_Port,LED_2_Pin, GPIO_PIN_RESET);
		}
	} else if(hadc->Instance == ADC2){
		for(uint8_t i=0; i<COUNT_SERVO_CHENNAL; i++){
			controllerStatus.valuesServo[i].position = (controllerStatus.valuesServo[i].position + ((servo_t*)getValuesServo)[i].position) >> 1;
			controllerStatus.valuesServo[i].amperage = (controllerStatus.valuesServo[i].amperage + ((servo_t*)getValuesServo)[i].amperage) >> 1;
		}
			
		if(controllerStatus.receive){
			static uint8_t counItrations = 0;
			if(counItrations == 7){
				HAL_GPIO_WritePin(LED_2_GPIO_Port,LED_2_Pin, GPIO_PIN_SET);
				counItrations = 0;
				message.start = 0xFFFF;
				message.lenOfPack = COUNT_SERVO_CHENNAL * sizeof(servo_t) + 1;
				message.typeOfPack = 2;
				
				if(testProtocol){
					memcpy(message.sevo, controllerStatus.valuesServo, COUNT_SERVO_CHENNAL*4);
					message.sevo[0].amperage = 500;
				}else{
					memcpy(message.sevo, controllerStatus.valuesServo, COUNT_SERVO_CHENNAL*4);
				}
				
				if(CDC_Transmit_FS((void*)&message, 3 + message.lenOfPack) == USBD_FAIL){
					controllerStatus.receive = 0;
					return;
				}
				HAL_GPIO_WritePin(LED_2_GPIO_Port,LED_2_Pin, GPIO_PIN_RESET);
			} else counItrations++;
		}	
	}
}
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
  * @brief  data processing function to be sentmessage.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: void
  */
uint32_t crc_f(uint32_t* Buf, uint32_t Len) {
	/* Reset CRC data register if necessary */
	CRC->CR = CRC_CR_RESET;
	/* Calculate CRC */
	for(uint8_t i=0; i<Len; i++){
		/* Set new value */
		CRC->DR = Buf[i];
	}
	/* Return data */
	return CRC->DR;
}
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
/**
  * @brief  data processing function to be sentmessage.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: void
  */
void messageProcessing(uint8_t* Buf, uint32_t Len){
	controllerStatus.connected = 1;
	if(Len >= 3){
		if(Buf[0] == 0xFF && Buf[1] == 0xFF && Len == Buf[2]){
			switch(Buf[3]){
				case 1: 
						NVIC_SystemReset(); 
					break;
				case 2: 
						
						HAL_GPIO_TogglePin(LED_1_GPIO_Port,LED_1_Pin);
						controllerStatus.receive = 1; 
					break;
				case 3: 
						
						HAL_GPIO_TogglePin(LED_1_GPIO_Port,LED_1_Pin);
						controllerStatus.receive = 0; 
					break;
			}
		}
	}
}

/************************ (C) itgavrilov@gmail.com ************************/
/****END OF FILE****/
