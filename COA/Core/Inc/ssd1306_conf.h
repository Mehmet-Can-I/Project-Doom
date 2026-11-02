/**
 * Private configuration file for the SSD1306/SSD1309 library
 * (lab version from COA-LAB-DOCUMENTATION/Wyswietlacz_OLED).
 *
 * Hardware: 128x64 SSD1309 OLED soldered on the lab base-stand
 * ("Nucleo-64 key" board), connector Z3:
 *   CS   -> PB6  (Arduino D10)
 *   D/C  -> PC7  (Arduino D9)  !! same wire as LCD-Keypad shield "E"
 *   RES  -> NRST (no GPIO)
 *   SDA  -> PA7  (SPI1_MOSI)
 *   SCLK -> PA5  (SPI1_SCK)
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

#define STM32L4

#define SSD1306_USE_SPI

// SPI Configuration
// The driver clocks these pins as GPIO so LD2 never inserts a partial byte.
#define SSD1306_SCK_Port         GPIOA
#define SSD1306_SCK_Pin          GPIO_PIN_5
#define SSD1306_MOSI_Port        GPIOA
#define SSD1306_MOSI_Pin         GPIO_PIN_7
#define SSD1306_SPI_PORT        hspi1
#define SSD1306_CS_Port         GPIOB
#define SSD1306_CS_Pin          GPIO_PIN_6
#define SSD1306_DC_Port         GPIOC
#define SSD1306_DC_Pin          GPIO_PIN_7
/* RES is tied to NRST on the base-stand. These are kept only so the library
 * header compiles; the patched ssd1306_Reset() never touches them. */
#define SSD1306_Reset_Port      GPIOA
#define SSD1306_Reset_Pin       GPIO_PIN_9

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Only the fonts used by the game (saves flash)
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18

#endif /* __SSD1306_CONF_H__ */
