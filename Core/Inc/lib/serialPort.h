/**
	*******************************************************************************
	* @file    serialPort.h																												*
	* @brief   Biointerface controller function library.													*
	*******************************************************************************
	*																																							*
	* COPYRIGHT(c) 2021 Gavrilov Stepan Alecsandrovich (itgavrilov@gmail.com)			*
	*																																							*
	*******************************************************************************
	*/
#ifndef __serialPort_h
#define __serialPort_h
#include "main.h"

#define MAX_NUMBER_OF_CHENNAL 8

#define PACKAGE_TYPE_CONFIG 0
#define PACKAGE_TYPE_CONTROL 1
#define PACKAGE_TYPE_DATA 2

#pragma pack (push, 1)

typedef struct{
	unsigned serialNumber:16;
	unsigned numberOfChannels:8;
	unsigned enableChannels:8;
} ConfigPacket_t;

typedef struct {
	unsigned command:8;
} ControlPacket_t;

typedef struct {
	unsigned index:8;
	int value:32;
} Chennal_t;

typedef struct {
	Chennal_t ch[MAX_NUMBER_OF_CHENNAL];
} ChennalPacket_t;

typedef  struct {
		unsigned start:16;
		unsigned packageType:8;
		unsigned length:8;
		union {
			ConfigPacket_t config;
			ChennalPacket_t chennals;
			ControlPacket_t control;
		} data;
} Message_t;
#pragma pack (pop)

/**
* @brief  data processing function to be sentmessage.
* @param  Buf: Buffer of data to be sent
* @param  Len: Number of data to be sent (in bytes)
* @retval Result of the operation: void
*/
void receiveMessage(uint8_t* Buf, uint32_t Len);
/**
* @brief  Sending channel data packets.
* @retval Result of the operation: void
*/
void sendDataFromChannels(void);


#endif  // __serialPort_h
/************************ (C) itgavrilov@gmail.com ************************/
/****END OF FILE****/

