#ifndef __flash_H
#define __flash_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"

HAL_StatusTypeDef flashStatus(void);
HAL_StatusTypeDef flashWrite(uint32_t, uint8_t*,uint32_t);

#ifdef __cplusplus
}
#endif
#endif
