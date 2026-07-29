/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

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
#define EA1_Pin GPIO_PIN_0
#define EA1_GPIO_Port GPIOA
#define EB1_Pin GPIO_PIN_1
#define EB1_GPIO_Port GPIOA
#define AIN2_Pin GPIO_PIN_8
#define AIN2_GPIO_Port GPIOE
#define PWMA_Pin GPIO_PIN_9
#define PWMA_GPIO_Port GPIOE
#define AIN1_Pin GPIO_PIN_10
#define AIN1_GPIO_Port GPIOE
#define PWMB_Pin GPIO_PIN_11
#define PWMB_GPIO_Port GPIOE
#define BIN1_Pin GPIO_PIN_12
#define BIN1_GPIO_Port GPIOE
#define BIN2_Pin GPIO_PIN_15
#define BIN2_GPIO_Port GPIOE
#define EA2_Pin GPIO_PIN_6
#define EA2_GPIO_Port GPIOC
#define EB2_Pin GPIO_PIN_7
#define EB2_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
