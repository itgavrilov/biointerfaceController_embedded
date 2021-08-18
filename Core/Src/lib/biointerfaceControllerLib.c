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


#include "flash.h"
#include "ads1256_defs.h"           // Псевдонимы Управляющих Регистров и Команд микросхемы ADS1256
#include "ads1256.h"                // Подключаем поддержку АЦП
#include "delays.h"

#define TEST_PROTOCOL

volatile static TDataRegistrator ADS1256_OriginalDataRegistrator = 0;
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
void ADS1256_DataRegistratorWrapper(const int32_t value){
	// Добавить очередной Замер к Выборке (обертка, чтобы еще считать число замеров)
	// Если подключен "регистратор данных", то отправить ему полученные данные
	if(ADS1256_OriginalDataRegistrator)
		(*ADS1256_OriginalDataRegistrator)(value);   
};
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
void doManualConversion(void){
	static uint8_t numberOfChennal = 0;
	int32_t  value;
  // В режиме "Одиночного запроса" требуется вручную прочитать Замер и передать его Регистратору
  // Замечу: даже если не включён режим "непрерывной конвертации" (DATAC),
  // Внутренне, АЦП всё равно производит конвертацию с заданной "частотой семплирования",
  // только результат не передаёт по Шине SPI микроконтроллеру, а просто ложит его во внутренний "регистр данных",
  // откуда микроконтроллер может его прочитать методом ADS1256_API_ReadLastData();
  // value = ADS1256_API_ConvertDataOnce();


  // Если частота семплирования достаточно велика, то нет особого смысла заставлять АЦП проводить внеочередную конвертацию методом ADS1256_API_ReadLastData();
  // Однако, иногда это полезно:
  //  - Если частота семплирования мала, скажем несколько раз в секунду, а нужно словить момент - можно произвости "Синхронную конвертацию" по внешнему событию.
  //  - Если АЦП погружается с спящий режим "STANBY" (для экономии питания устройства), а потом периодически пробуждается для быстрых одиночных замеров - удобно эти замеры инициировать вручную.
  value = ADS1256_API_ReadLastData();
	

	#ifdef TEST_PROTOCOL
	genTestSignal(&controllerStatus.chennals, numberOfChennal);
	#else
	controllerStatus.chennals.ch[numberOfChennal].value = value;
	#endif
	
	if(++numberOfChennal > 7){
		HAL_GPIO_TogglePin(LED_1_GPIO_Port, LED_1_Pin);
		dataIsReady = 1;
		numberOfChennal = 0;
	}
	
	ADS1256_API_SetMultiplexerSingleP(numberOfChennal);
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void sendToSerialPort(){
	if(dataIsReady){
		sendDataFromChannels();
		dataIsReady = 0;
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
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM2) {
		doManualConversion();
  }
  /* USER CODE END Callback 1 */
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void initController(void){
	memcpy(&config, (void*)CONFIG_IN_FLASH, sizeof(ConfigPacket_t));
	if(config.serialNumber == 0 || config.serialNumber == 0xFFFF){
		config.numberOfChannels = MAX_NUMBER_OF_CHENNAL;
		config.serialNumber = 1;
		config.enableChannels = 0xFF;
	}
	
	for(uint8_t i = 0; i < MAX_NUMBER_OF_CHENNAL; i++){
		controllerStatus.chennals.ch[i].index = i;
	}
	
	// Затем: Инициализация АЦП
  // Настройка регистров/режимов аппаратной микросхемы АЦП / Инициализация АЦП модуля (установка режима по-умолчанию)
  ADS1256_Init();
	// Остановить режим "SDATAC: Stop Read Data Continuous" СИНХРОННО
  // (Примечание: После выполнения этой функции, АЦП вновь станет доступным к управлению!)
  ADS1256_API_StopDataContinuousModeSynchronous();
  ADS1256_OriginalDataRegistrator = ADS1256_API_GetDataRegistrator();
  ADS1256_API_SetDataRegistrator(ADS1256_DataRegistratorWrapper);
  // TODO: чтобы настроить АЦП на свою конфигурацию и задачу - можно изменить код стандартной процедуры инициализации ADS1256_Init(),
  // а можно (если немного перенастроить) просто запустить несколько дополнительных команд после... (API настройки описано в модуле "ads1256.h")
  // Затем, можно перенастроить АЦП в свой режим:
  ADS1256_API_SetMultiplexerSingleP(0);                                 //DEBUG: мультиплексор входных каналов (Регистр MUX)
  ADS1256_API_SetInputBufferMode( ADS1256_STATUS_BUFEN_OFF );         //DEBUG: Входной Повторитель (увеличивает импеданс до 10..80МОм, но уменьшает динамический диапазон до 0..3V) (бит BUFEN в регистре STATUS)
	ADS1256_API_SetProgrammableGainAmplifierMode( ADS1256_ADCON_PGA_2 );  //DEBUG: Настройка "Усилителя с Программируемым коэффициентом" для входного сигнала (биты PGAx регистра ADCON)
	//ADS1256_Test();
	HAL_TIM_Base_Start_IT(&htim2);
}
/************************ (C) Gavrilov S.A. (itgavrilov@gmail.com) ************************/
/****END OF FILE****/
