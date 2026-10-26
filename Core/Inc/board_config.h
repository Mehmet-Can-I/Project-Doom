/* board_config.h - NUCLEO-L476RG pin ve donanım ayarları
 * Tüm pin atamaları burada; donanım değişirse sadece bu dosyayı düzenleyin.
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32l4xx_hal.h"

/* ---------------- Ekran (ILI9341, SPI1) ----------------
 * SCK  = PA5 (D13)   MOSI = PA7 (D11)   MISO = PA6 (D12, opsiyonel)
 * CS   = PB6 (D10)   DC   = PA9 (D8)    RST  = PC7 (D9)
 * NOT: PA5 aynı zamanda LD2 LED'i, SPI kullanırken LED'i unutun.
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

/* ---------------- Özel klavye (4x4 matris) ----------------
 * Satırlar: çıkış (push-pull), sırayla LOW çekilir
 * Sütunlar: giriş, dahili pull-up
 * Tuşlarda diyot kullanın (ghosting'e karşı), özellikle aynı anda
 * "ileri + ateş + strafe" basılacağı için.
 */
#define KB_ROWS               4
#define KB_COLS               4

#define KB_ROW_PORT           GPIOC
#define KB_ROW_PINS           { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3 }

#define KB_COL_PORT           GPIOB
#define KB_COL_PINS           { GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15 }

#define KB_SCAN_PERIOD_MS     1     /* TIM6 kesmesi ile her 1 ms bir satır */
#define KB_DEBOUNCE_SCANS     5     /* 5 ardışık aynı okuma = kararlı */

/* ---------------- WAD kaynağı ----------------
 * WAD_SOURCE_INTERNAL : WAD dahili flash'a gömülü (sadece çok küçük WAD'lar)
 * WAD_SOURCE_QSPI     : Harici QSPI flash, memory-mapped (0x90000000)
 */
#define WAD_SOURCE_QSPI
#define WAD_QSPI_BASE_ADDR    0x90000000UL

/* ---------------- Debug UART (ST-LINK VCP) ----------------
 * USART2: PA2 TX, PA3 RX -> printf çıktısı USB üzerinden
 */
#define DEBUG_UART            huart2

extern SPI_HandleTypeDef  hspi1;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim6;

#endif /* BOARD_CONFIG_H */
