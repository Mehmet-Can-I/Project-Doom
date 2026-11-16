/*
 * lcd_hd44780.h  -  FEATURE 2: 16x2 character LCD (HD44780), LCD Keypad Shield
 *
 * 4-bit interface, write-only (R/W is tied to GND on the shield).
 *
 *   RS  -> PA9  (D8)
 *   E   -> PC7  (D9)   !! same wire as the OLED D/C line, see ssd1306.c
 *   DB4 -> PB5  (D4)
 *   DB5 -> PB4  (D5)
 *   DB6 -> PB10 (D6)
 *   DB7 -> PA8  (D7)
 *   Backlight -> D10 = PB6 = OLED CS (high = backlight on when OLED idle)
 *
 * Based on COA-LAB-DOCUMENTATION/LCD_Keypad/LCD_Keypad.c, rewritten with
 * microsecond delays (the lab version waits ~6 ms per character, which would
 * stall the game) and in 2-line mode.
 */

#ifndef INC_LCD_HD44780_H_
#define INC_LCD_HD44780_H_

#include "stm32l4xx_hal.h"

#define LCD_COLS 16
#define LCD_ROWS 2

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
/* Writes a whole row, padded with spaces to 16 chars (no clear -> no flicker) */
void LCD_PrintLine(uint8_t row, const char *s);

#endif /* INC_LCD_HD44780_H_ */
