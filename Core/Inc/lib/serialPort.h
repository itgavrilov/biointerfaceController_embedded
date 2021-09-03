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
} ConfigPacket_t;

typedef struct {
	unsigned command:8;
} ControlPacket_t;
typedef enum {
	scale12  =  0, // 	0 - 12 bit 
	scale14  =  1, // 	1 - 14 bit >> 2
	scale16  =  2, // 	2 - 16 bit >> 4
	scale18  =  3, // 	3 - 18 bit >> 6
	scale20  =  4, // 	4 - 20 bit >> 8
	scale22  =  5, // 	5 - 22 bit >> 10
	scale24  =  6, // 	6 - 24 bit >> 12
	scale26  =  7, // 	7 - 26 bit >> 14
	scale28  =  8, // 	8 - 28 bit >> 16
	scale30  =  9, // 	9 - 30 bit >> 18
	scale32  = 10, // 10 - 32 bit >> 20
	scale34  = 11, // 11 - 34 bit >> 22
	scale36  = 12  // 12 - 36 bit >> 24
} ScaleBitTypeDef;

typedef struct {
	ScaleBitTypeDef scale:4;
	unsigned value:12;
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
void sendDataFromChannels(ChennalPacket_t *ch);


#endif  // __serialPort_h
/************************ (C) itgavrilov@gmail.com ************************/
/****END OF FILE****/

