/* lcd_ili9341.h - Minimal ILI9341 SPI driver (RGB565) */
#ifndef LCD_ILI9341_H
#define LCD_ILI9341_H

#include <stdint.h>

void lcd_init(void);
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/* Sends one line of RGB565 data via DMA; waits for the previous transfer first */
void lcd_write_line_dma(const uint16_t *pixels, uint16_t count);
void lcd_wait_dma(void);

void lcd_fill(uint16_t color);

#endif /* LCD_ILI9341_H */
