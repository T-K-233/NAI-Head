/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#define MEM_ALIGNMENT                   4
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPIO_SPI5_CS_Pin GPIO_PIN_3
#define GPIO_SPI5_CS_GPIO_Port GPIOE
#define GPIO_SPI4_CS_Pin GPIO_PIN_4
#define GPIO_SPI4_CS_GPIO_Port GPIOE
#define GPIO_SPI4_DC_Pin GPIO_PIN_5
#define GPIO_SPI4_DC_GPIO_Port GPIOE
#define GPIO_SPI5_DC_Pin GPIO_PIN_8
#define GPIO_SPI5_DC_GPIO_Port GPIOF
#define GPIO_LCD_RESET_Pin GPIO_PIN_0
#define GPIO_LCD_RESET_GPIO_Port GPIOG
#define GPIO_LCD_BLK_Pin GPIO_PIN_1
#define GPIO_LCD_BLK_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */
#include "app.h"
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
