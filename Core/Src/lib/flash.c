//##############################################################################
//Created: 2018
//Author: Gavrilov S.A.
//itgavrilov@gmail.com 
//##############################################################################
#include "flash.h"
//------------------------------------------------------------------------------
HAL_StatusTypeDef status = HAL_OK;	
HAL_StatusTypeDef flashStatus(void){return status;};
//------------------------------------------------------------------------------
HAL_StatusTypeDef flashWrite(uint32_t addr, uint8_t* data, uint32_t len)	{
	// cnt - количество единиц записи 
	// typeProgram - тип записи(размер еденицы записи): 
	// FLASH_TYPEPROGRAM_HALFWORD   блоки по 16 бит
	// FLASH_TYPEPROGRAM_WORD       блоки по 32 бита
	// FLASH_TYPEPROGRAM_DOUBLEWORD блоки по 64 бита
	// !!!Размер данных дожен быть делим без остатка на выбранный размер блоков!!!!
	
	if(status == HAL_OK) {
		uint32_t PageError = 0;
		FLASH_EraseInitTypeDef EraseInitStruct = {0};
		status = HAL_BUSY;
		//############################
		#if defined  (STM32F0)
			// Для stm32f072
			EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			EraseInitStruct.PageAddress = addr;
			EraseInitStruct.NbPages = 1;
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR);
		#else
		// Для stm32f205
			EraseInitStruct.Banks = FLASH_BANK_1;
			EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
			EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
			if(addr < 0x08010000) EraseInitStruct.Sector = FLASH_SECTOR_3;
			else EraseInitStruct.Sector = FLASH_SECTOR_4;
			EraseInitStruct.NbSectors = 1;
			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
		#endif
		//############################
			HAL_FLASH_Unlock();
			if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
					HAL_FLASH_Lock(); 
					return status = HAL_ERROR;
			}
			for (uint32_t i=0; i<len; i+=2 ){
				uint16_t tmp = data[i+1]<<8;
				tmp += data[i];
				status = HAL_BUSY;
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr+i, tmp) != HAL_OK) {
					HAL_FLASH_Lock();
					return status = HAL_ERROR;
				}
			}

			HAL_FLASH_Lock();	
			return status = HAL_OK;
	} else return status;
}
//------------------------------------------------------------------------------
