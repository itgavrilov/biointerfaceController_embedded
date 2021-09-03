/**
	*******************************************************************************
	* @file    serialPort.c																												*
	* @brief   Biointerface controller function library.													*
	*******************************************************************************
	*																																							*
	* COPYRIGHT(c) 2021 Gavrilov Stepan Alecsandrovich (itgavrilov@gmail.com)			*
	*																																							*
	*******************************************************************************
	*/
#include "usbd_cdc_if.h"
#include "serialPort.h"
#include "biointerfaceControllerLib.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern ConfigPacket_t config;
extern status_t controllerStatus;
Message_t message = {0xFFFF, 0, 0, {0}};;
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
/**
* @brief  Sending channel data packets.
* @retval Result of the operation: void
*/
void sendDataFromChannels(ChennalPacket_t *ch){
	message.packageType = PACKAGE_TYPE_DATA;
	message.length = sizeof(ChennalPacket_t);
	
	memcpy(&message.data.chennals.ch, ch, message.length);
	
	if(CDC_Transmit_FS((void*)&message, 4 + message.length) == USBD_FAIL){
		controllerStatus.serialPort.sendChannelData = 0;
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void parsingConfigMessage(ConfigPacket_t *msg){
	if(msg->serialNumber) 
		config.serialNumber = msg->serialNumber;
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void parsingCommandMessage(ControlPacket_t *msg){
	switch(msg->command){
		case 0:{
			message.packageType = PACKAGE_TYPE_CONFIG;
			message.length = sizeof(ConfigPacket_t);
			memcpy(&message.data.config, &config, message.length);
				
			if(CDC_Transmit_FS((void*)&message, 4 + message.length) == USBD_FAIL){
				controllerStatus.serialPort.sendChannelData = 0;
			}
		} break;
		case 1: 
				controllerStatus.serialPort.sendChannelData = 1; 
			break;
		case 2: 
				controllerStatus.serialPort.sendChannelData = 0; 
			break;
		case 3: 
				NVIC_SystemReset(); 
			break;
	}
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
void parsingDataMessage(ChennalPacket_t *msg){
	
}
//###############################################################################################################################
//###############################################################################################################################
//###############################################################################################################################
/**
* @brief  data processing function to be sentmessage.
* @param  buf: Buffer of data to be sent
* @param  len: Number of data to be sent (in bytes)
* @retval Result of the operation: void
*/
void receiveMessage(uint8_t* buf, uint32_t len){
	controllerStatus.serialPort.connected = 1;
	
	if(len > 4){
		Message_t* msg = (Message_t*)buf;
		
		while(len){
			if(msg->start == 0xFFFF && len >= msg->length+4){
				switch(msg->packageType){
					case PACKAGE_TYPE_CONFIG:
						parsingConfigMessage((ConfigPacket_t*)&msg->data);
						break;
					case PACKAGE_TYPE_CONTROL:
						parsingCommandMessage((ControlPacket_t*)&msg->data);
						break;
					case PACKAGE_TYPE_DATA:
						parsingDataMessage((ChennalPacket_t*)&msg->data);
						break;
				}
				len -= msg->length+4;
			} else {
				buf = &buf[1];
				len--;
			}
		}
	}
}
/************************ (C) itgavrilov@gmail.com ************************/
/****END OF FILE****/

