/* board_config.h - NUCLEO-L476RG pin and hardware configuration
 * All pin assignments live here; if the hardware changes, edit only this file.
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32l4xx_hal.h"

/* ---------------- Display (ILI9341, SPI1) ----------------
 * SCK  = PA5 (D13)   MOSI = PA7 (D11)   MISO = PA6 (D12, optional)
 * CS   = PB6 (D10)   DC   = PA9 (D8)    RST  = PC7 (D9)
 * NOTE: PA5 is also the LD2 LED; the LED is unusable while SPI is active.
 */
#define LCD_SPI               hspi1
#define LCD_CS_PORT           GPIOB
#define LCD_CS_PIN            GPIO_PIN_6
#define LCD_DC_PORT           GPIOA
#define LCD_DC_PIN            GPIO_PIN_9
#define LCD_RST_PORT          GPIOC
#define LCD_RST_PIN           GPIO_PIN_7

#define LCD_WIDTH             320
#define LCD_HEIGHT            240

/* ---------------- Custom keyboard (4x4 matrix) ----------------
 * Rows:    outputs (push-pull), driven LOW one at a time
 * Columns: inputs with internal pull-up
 * Put a diode on every key (anti-ghosting), since "forward + fire + strafe"
 * will often be held at the same time.
 */
#define KB_ROWS               4
#define KB_COLS               4

#define KB_ROW_PORT           GPIOC
#define KB_ROW_PINS           { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3 }

#define KB_COL_PORT           GPIOB
#define KB_COL_PINS           { GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15 }

#define KB_SCAN_PERIOD_MS     1     /* TIM6 interrupt scans one row every 1 ms */
#define KB_DEBOUNCE_SCANS     5     /* 5 consecutive identical reads = stable */

/* ---------------- WAD source ----------------
 * WAD_SOURCE_INTERNAL : WAD embedded in internal flash (very small WADs only)
 * WAD_SOURCE_QSPI     : External QSPI flash, memory-mapped (0x90000000)
 */
#define WAD_SOURCE_QSPI
#define WAD_QSPI_BASE_ADDR    0x90000000UL

/* ---------------- Debug UART (ST-LINK VCP) ----------------
 * USART2: PA2 TX, PA3 RX -> printf output over USB
 */
#define DEBUG_UART            huart2

extern SPI_HandleTypeDef  hspi1;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim6;

#endif /* BOARD_CONFIG_H */
