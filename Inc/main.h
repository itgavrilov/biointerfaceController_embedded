/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Pos_Servo_3_Pin GPIO_PIN_0
#define Pos_Servo_3_GPIO_Port GPIOC
#define Shunt_Servo_3_Pin GPIO_PIN_1
#define Shunt_Servo_3_GPIO_Port GPIOC
#define Pos_Servo_4_Pin GPIO_PIN_2
#define Pos_Servo_4_GPIO_Port GPIOC
#define Shunt_Servo_4_Pin GPIO_PIN_3
#define Shunt_Servo_4_GPIO_Port GPIOC
#define CH_1_Pin GPIO_PIN_0
#define CH_1_GPIO_Port GPIOA
#define CH_2_Pin GPIO_PIN_1
#define CH_2_GPIO_Port GPIOA
#define CH_3_Pin GPIO_PIN_2
#define CH_3_GPIO_Port GPIOA
#define CH_4_Pin GPIO_PIN_3
#define CH_4_GPIO_Port GPIOA
#define CH_5_Pin GPIO_PIN_4
#define CH_5_GPIO_Port GPIOA
#define Pos_Servo_1_Pin GPIO_PIN_5
#define Pos_Servo_1_GPIO_Port GPIOA
#define Shunt_Servo_1_Pin GPIO_PIN_6
#define Shunt_Servo_1_GPIO_Port GPIOA
#define Pos_Servo_2_Pin GPIO_PIN_7
#define Pos_Servo_2_GPIO_Port GPIOA
#define Shunt_Servo_2_Pin GPIO_PIN_0
#define Shunt_Servo_2_GPIO_Port GPIOB
#define PWM_Servo_4_Pin GPIO_PIN_1
#define PWM_Servo_4_GPIO_Port GPIOB
#define PWM_Servo_1_Pin GPIO_PIN_6
#define PWM_Servo_1_GPIO_Port GPIOC
#define PWM_Servo_2_Pin GPIO_PIN_7
#define PWM_Servo_2_GPIO_Port GPIOC
#define PWM_Servo_3_Pin GPIO_PIN_8
#define PWM_Servo_3_GPIO_Port GPIOC
#define LED_2_Pin GPIO_PIN_8
#define LED_2_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
