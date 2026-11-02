/*
 * lcd_hd44780.c
 *
 * HD44780 16x2 LCD driver, 4-bit interface. See lcd_hd44780.h for wiring.
 */

#include "lcd_hd44780.h"
#include "main.h"
#include <string.h>

/* HD44780 instructions */
#define LCD_CLEARDISPLAY   0x01
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_FUNCTIONSET    0x20
#define LCD_SETDDRAMADDR   0x80

#define LCD_ENTRYLEFT      0x02
#define LCD_DISPLAYON      0x04
#define LCD_4BITMODE       0x00
#define LCD_2LINE          0x08
#define LCD_5x8DOTS        0x00

/* The command the LCD receives "for free" on every OLED frame (see
 * ssd1306_BeforeDcFallingEdge below). It must be harmless and must match the
 * display state we want: display ON, cursor OFF, blink OFF. */
#define LCD_GUARD_COMMAND  (LCD_DISPLAYCONTROL | LCD_DISPLAYON) /* 0x0C */

/* ---------------------------------------------------------------- delays */

void delay_us(uint32_t us)
{
  const uint32_t start = DWT->CYCCNT;
  const uint32_t ticks = us * (SystemCoreClock / 1000000U);
  while ((DWT->CYCCNT - start) < ticks) { }
}

static void dwt_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* ---------------------------------------------------------------- low level */

static void set_data_nibble(uint8_t nibble)
{
  HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, (nibble >> 0) & 1U);
  HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, (nibble >> 1) & 1U);
  HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, (nibble >> 2) & 1U);
  HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, (nibble >> 3) & 1U);
}

static void pulse_enable(void)
{
  /* E idle is LOW (= OLED D/C idle). Latch happens on the falling edge. */
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  delay_us(1);   /* PW_EH >= 450 ns */
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  delay_us(1);
}

static void write4bits(uint8_t nibble)
{
  set_data_nibble(nibble);
  pulse_enable();
}

static void lcd_write(uint8_t rs, uint8_t value)
{
  /* Wait BEFORE writing: covers the previous LCD instruction and also the
   * guard command injected by the last OLED frame (both take <= 37 us). */
  delay_us(50);
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
  write4bits(value >> 4);
  write4bits(value & 0x0F);
}

static void lcd_command(uint8_t cmd) { lcd_write(0, cmd); }
static void lcd_data(uint8_t c)      { lcd_write(1, c); }

/* ------------------------------------------------- shared PC7 (E / OLED D/C)
 * Called by the OLED driver right before it pulls PC7 low. The LCD will latch
 * whatever is on RS/DB4..7 at that moment, so we present the two nibbles of
 * LCD_GUARD_COMMAND (edge 0 = high nibble, edge 1 = low nibble).
 * Everything runs from the main loop, so this never interleaves with an
 * LCD byte in progress.
 */
void ssd1306_BeforeDcFallingEdge(uint8_t edge_index)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  set_data_nibble(edge_index == 0 ? (LCD_GUARD_COMMAND >> 4) : (LCD_GUARD_COMMAND & 0x0F));
  delay_us(1);   /* address/data set-up time before the falling edge */
}

/* ---------------------------------------------------------------- public */

void LCD_Init(void)
{
  dwt_init();

  HAL_Delay(50);   /* > 40 ms after Vcc rises */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);

  /* Reset sequence (datasheet fig. 24, "initializing by instruction") */
  write4bits(0x03); HAL_Delay(5);
  write4bits(0x03); HAL_Delay(5);
  write4bits(0x03); delay_us(200);
  write4bits(0x02); delay_us(100);   /* now in 4-bit mode */

  lcd_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  lcd_command(LCD_GUARD_COMMAND);    /* display on, cursor off, blink off */
  LCD_Clear();
  lcd_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
}

void LCD_Clear(void)
{
  lcd_command(LCD_CLEARDISPLAY);
  HAL_Delay(2);    /* clear takes 1.52 ms */
}

void LCD_SetCursor(uint8_t col, uint8_t row)
{
  static const uint8_t row_offset[LCD_ROWS] = {0x00, 0x40};
  if (row >= LCD_ROWS) row = LCD_ROWS - 1;
  lcd_command(LCD_SETDDRAMADDR | (col + row_offset[row]));
}

void LCD_Print(const char *s)
{
  while (*s) lcd_data((uint8_t)*s++);
}

void LCD_PrintLine(uint8_t row, const char *s)
{
  LCD_SetCursor(0, row);
  uint8_t n = 0;
  while (*s && n < LCD_COLS) { lcd_data((uint8_t)*s++); n++; }
  while (n++ < LCD_COLS) lcd_data(' ');
}
