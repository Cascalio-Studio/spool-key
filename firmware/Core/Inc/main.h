/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_dma.h"

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
#define KEY_UP_Pin LL_GPIO_PIN_0
#define KEY_UP_GPIO_Port GPIOC
#define KEY_UP_EXTI_IRQn EXTI0_IRQn
#define KEY_DOWN_Pin LL_GPIO_PIN_1
#define KEY_DOWN_GPIO_Port GPIOC
#define KEY_DOWN_EXTI_IRQn EXTI1_IRQn
#define KEY_BACK_Pin LL_GPIO_PIN_2
#define KEY_BACK_GPIO_Port GPIOC
#define KEY_BACK_EXTI_IRQn EXTI2_IRQn
#define KEY_OK_Pin LL_GPIO_PIN_3
#define KEY_OK_GPIO_Port GPIOC
#define KEY_OK_EXTI_IRQn EXTI3_IRQn
#define SEN_Pin LL_GPIO_PIN_4
#define SEN_GPIO_Port GPIOA
#define LED3_Pin LL_GPIO_PIN_10
#define LED3_GPIO_Port GPIOB
#define LED2_Pin LL_GPIO_PIN_11
#define LED2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
