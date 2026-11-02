/*
 * lcd_hd44780.h
 *
 * 16x2 character LCD (HD44780) on the DFRobot-style "LCD Keypad Shield",
 * 4-bit mode, write-only (R/W tied to GND on the shield).
 *
 *   RS  -> PA9  (D8)
 *   E   -> PC7  (D9)   shared with OLED D/C, see ssd1306.c
 *   DB4 -> PB5  (D4)
 *   DB5 -> PB4  (D5)
 *   DB6 -> PB10 (D6)
 *   DB7 -> PA8  (D7)
 *   Backlight -> D10 = PB6 = OLED CS (kept high when OLED is idle)
 *
 * Based on COA-LAB-DOCUMENTATION/LCD_Keypad/LCD_Keypad.c, rewritten with
 * microsecond delays (the lab version spends ~6 ms per character, which
 * would stall the game loop) and 2-line mode.
 */

#ifndef INC_LCD_HD44780_H_
#define INC_LCD_HD44780_H_

#include "stm32l4xx_hal.h"

#define LCD_COLS 16
#define LCD_ROWS 2

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Print(const char *s);
/* Writes a whole row, padded with spaces to 16 chars (no flicker, no clear) */
void LCD_PrintLine(uint8_t row, const char *s);

/* Busy-wait microsecond delay (DWT cycle counter), started by LCD_Init() */
void delay_us(uint32_t us);

#endif /* INC_LCD_HD44780_H_ */
