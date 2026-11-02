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
#include "stm32l4xx_hal.h"

#include "stm32l4xx_nucleo.h"

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
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define KEYPAD_ADC_Pin GPIO_PIN_0
#define KEYPAD_ADC_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_5
#define OLED_SCK_GPIO_Port GPIOA
#define OLED_MOSI_Pin GPIO_PIN_7
#define OLED_MOSI_GPIO_Port GPIOA
#define LCD_DB6_Pin GPIO_PIN_10
#define LCD_DB6_GPIO_Port GPIOB
#define RGB_R_Pin GPIO_PIN_13
#define RGB_R_GPIO_Port GPIOB
#define RGB_G_Pin GPIO_PIN_14
#define RGB_G_GPIO_Port GPIOB
#define RGB_B_Pin GPIO_PIN_15
#define RGB_B_GPIO_Port GPIOB
#define LCD_E_OLED_DC_Pin GPIO_PIN_7
#define LCD_E_OLED_DC_GPIO_Port GPIOC
#define LCD_DB7_Pin GPIO_PIN_8
#define LCD_DB7_GPIO_Port GPIOA
#define LCD_RS_Pin GPIO_PIN_9
#define LCD_RS_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LCD_DB5_Pin GPIO_PIN_4
#define LCD_DB5_GPIO_Port GPIOB
#define LCD_DB4_Pin GPIO_PIN_5
#define LCD_DB4_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_6
#define OLED_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/* PC7 is ONE wire shared by the LCD shield "E" (D9) and the OLED "D/C".
 * Aliases so each driver reads naturally: */
#define LCD_E_Pin        LCD_E_OLED_DC_Pin
#define LCD_E_GPIO_Port  LCD_E_OLED_DC_GPIO_Port
#define OLED_DC_Pin      LCD_E_OLED_DC_Pin
#define OLED_DC_GPIO_Port LCD_E_OLED_DC_GPIO_Port

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
