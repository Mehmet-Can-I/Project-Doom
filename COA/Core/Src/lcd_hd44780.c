/*
 * lcd_hd44780.c  -  FEATURE 2: HD44780 16x2 LCD driver, 4-bit mode
 *
 * A byte is sent as two 4-bit halves (high nibble first) on DB4..DB7.
 * The LCD reads each nibble on the FALLING edge of E. RS selects
 * command (0) or character data (1).
 */

#include "lcd_hd44780.h"
#include "main.h"

/* HD44780 instructions (datasheet table 6) */
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

/* E is shared with the OLED D/C line, so every OLED screen update also
 * clocks one byte into the LCD (see ssd1306_BeforeDcFallingEdge below).
 * We make that byte a harmless command that keeps the state we want:
 * display ON, cursor OFF, blink OFF = 0x0C. */
#define LCD_GUARD_COMMAND  (LCD_DISPLAYCONTROL | LCD_DISPLAYON)

/* ------------------------------------------------------------------ delays */

/* Busy-wait using the Cortex-M4 cycle counter (80 cycles = 1 us). */
static void delay_us(uint32_t us)
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

/* --------------------------------------------------------------- low level */

static void set_data_nibble(uint8_t nibble)
{
  HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, (nibble >> 0) & 1U);
  HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, (nibble >> 1) & 1U);
  HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, (nibble >> 2) & 1U);
  HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, (nibble >> 3) & 1U);
}

static void pulse_enable(void)
{
  /* E idles LOW (= OLED D/C idle). The LCD latches on the falling edge. */
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
  /* Wait BEFORE writing: covers the previous instruction and also the guard
   * command sent by the last OLED update (each takes <= 37 us). */
  delay_us(50);
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
  write4bits(value >> 4);
  write4bits(value & 0x0F);
}

static void lcd_command(uint8_t cmd) { lcd_write(0, cmd); }
static void lcd_data(uint8_t c)      { lcd_write(1, c); }

/* -------------------------------------------- shared PC7 (LCD E / OLED D/C)
 * Called by the OLED driver just before it pulls PC7 low. The LCD will
 * latch whatever is on RS/DB4..7 at that moment, so we present the two
 * nibbles of LCD_GUARD_COMMAND (edge 0 = high nibble, edge 1 = low nibble).
 * Everything runs from the main loop, so this never cuts an LCD byte.
 */
void ssd1306_BeforeDcFallingEdge(uint8_t edge_index)
{
  /* Windows can be one byte long: let the previous guard command finish
   * before latching the first nibble of the next one. */
  if (edge_index == 0) delay_us(50);
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  set_data_nibble(edge_index == 0 ? (LCD_GUARD_COMMAND >> 4) : (LCD_GUARD_COMMAND & 0x0F));
  delay_us(1);   /* set-up time before the falling edge */
}

/* ------------------------------------------------------------------ public */

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

void LCD_PrintLine(uint8_t row, const char *s)
{
  LCD_SetCursor(0, row);
  uint8_t n = 0;
  while (*s && n < LCD_COLS) { lcd_data((uint8_t)*s++); n++; }
  while (n++ < LCD_COLS) lcd_data(' ');   /* pad: overwrite old text */
}
