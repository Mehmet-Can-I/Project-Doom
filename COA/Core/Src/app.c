/*
 * app.c
 */

#include "app.h"
#include "snake.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "lcd_hd44780.h"
#include "keypad.h"
#include "rgb_led.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

typedef enum { ST_TITLE, ST_PLAYING, ST_PAUSED, ST_GAME_OVER } AppState;

static volatile AppState state = ST_TITLE;
static volatile uint8_t  step_pending;       /* set by TIM6 ISR */
static volatile uint8_t  button_pressed;     /* set by EXTI ISR */
static volatile uint16_t step_period_ms = 200;
static volatile uint16_t ms_since_step;

static uint16_t best_score;
static uint32_t last_key_poll_ms;

/* ----------------------------------------------------------------- screens */

static void oled_center_text(uint8_t y, const char *s, SSD1306_Font_t font)
{
  uint8_t w = (uint8_t)(strlen(s) * font.width);
  ssd1306_SetCursor((uint8_t)((SSD1306_WIDTH - w) / 2), y);
  ssd1306_WriteString((char *)s, font, White);
}

static void show_title(void)
{
  ssd1306_Fill(Black);
  ssd1306_DrawRectangle(0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1, White);
  oled_center_text(12, "SNAKE", Font_11x18);
  oled_center_text(40, "press SELECT", Font_6x8);
  ssd1306_UpdateScreen();

  LCD_PrintLine(0, "  SNAKE  NUCLEO");
  LCD_PrintLine(1, "SELECT to start");
}

static void lcd_show_score(const char *status)
{
  char line[24];
  snprintf(line, sizeof line, "Score: %u", Snake_GetScore());
  LCD_PrintLine(0, line);
  if (status)
    LCD_PrintLine(1, status);
  else {
    snprintf(line, sizeof line, "Best:  %u", best_score);
    LCD_PrintLine(1, line);
  }
}

static void show_game_over(void)
{
  char line[24];
  Snake_Draw();
  ssd1306_FillRectangle(14, 14, 113, 49, Black);
  ssd1306_DrawRectangle(14, 14, 113, 49, White);
  oled_center_text(18, "GAME OVER", Font_11x18);
  snprintf(line, sizeof line, "score %u", Snake_GetScore());
  oled_center_text(38, line, Font_6x8);
  ssd1306_UpdateScreen();

  snprintf(line, sizeof line, "Score:%u Best:%u", Snake_GetScore(), best_score);
  LCD_PrintLine(0, line);
  LCD_PrintLine(1, "SELECT = replay");
}

/* ------------------------------------------------------------- transitions */

static void start_game(void)
{
  /* Seed: ADC noise + time of the key press -> different game each time */
  uint32_t seed = HAL_GetTick() ^ ((uint32_t)Keypad_ReadRaw() << 16) ^ DWT->CYCCNT;
  Snake_Reset(seed);

  __disable_irq();
  step_period_ms = Snake_GetStepPeriodMs();
  ms_since_step = 0;
  step_pending = 0;
  state = ST_PLAYING;
  __enable_irq();

  RGB_Set(0, 0, 0);
  Snake_Draw();
  ssd1306_UpdateScreen();
  lcd_show_score(NULL);
  printf("[snake] new game, seed=%08lx\r\n", (unsigned long)seed);
}

static void game_over(void)
{
  state = ST_GAME_OVER;
  if (Snake_GetScore() > best_score) best_score = Snake_GetScore();
  RGB_Blink(255, 0, 0, 3, 150);              /* red x3 */
  show_game_over();
  printf("[snake] game over, score=%u best=%u\r\n", Snake_GetScore(), best_score);
}

static void toggle_pause(void)
{
  if (state == ST_PLAYING) {
    state = ST_PAUSED;
    lcd_show_score("    PAUSED");
    RGB_Set(0, 0, 40);                       /* dim blue while paused */
  } else if (state == ST_PAUSED) {
    RGB_Set(0, 0, 0);
    lcd_show_score(NULL);
    __disable_irq();
    ms_since_step = 0;
    state = ST_PLAYING;
    __enable_irq();
  }
}

/* ------------------------------------------------------------------ input */

static void handle_key(Key k)
{
  switch (state) {
    case ST_TITLE:
    case ST_GAME_OVER:
      if (k == KEY_SELECT) start_game();
      break;

    case ST_PLAYING:
      switch (k) {
        case KEY_UP:     Snake_SetDirection(DIR_UP);    break;
        case KEY_DOWN:   Snake_SetDirection(DIR_DOWN);  break;
        case KEY_LEFT:   Snake_SetDirection(DIR_LEFT);  break;
        case KEY_RIGHT:  Snake_SetDirection(DIR_RIGHT); break;
        case KEY_SELECT: toggle_pause();                break;
        default: break;
      }
      break;

    case ST_PAUSED:
      if (k == KEY_SELECT) toggle_pause();
      break;
  }
}

/* -------------------------------------------------------------- interrupts */

void App_OnTick1ms(void)
{
  if (state != ST_PLAYING) return;
  if (++ms_since_step >= step_period_ms) {
    ms_since_step = 0;
    step_pending = 1;
  }
}

void App_OnUserButton(void)
{
  button_pressed = 1;   /* handled in the main loop (no LCD/OLED in ISR) */
}

/* ------------------------------------------------------------------- main */

void App_Init(void)
{
  LCD_Init();              /* before the OLED: see shared PC7 notes */
  ssd1306_Init();
  Keypad_Init(&hadc1);
  RGB_Init(&htim1);

  printf("\r\n[snake] boot - NUCLEO-L476RG snake prototype\r\n");

  /* quick RGB self-test: R, G, B */
  RGB_Set(255, 0, 0); HAL_Delay(150);
  RGB_Set(0, 255, 0); HAL_Delay(150);
  RGB_Set(0, 0, 255); HAL_Delay(150);
  RGB_Set(0, 0, 0);

  show_title();
}

void App_Loop(void)
{
  const uint32_t now = HAL_GetTick();

  /* keypad: sample every 10 ms (debounced inside) */
  if (now - last_key_poll_ms >= 10) {
    last_key_poll_ms = now;
    Key k = Keypad_PollEvent();
    if (k != KEY_NONE) handle_key(k);
  }

  if (button_pressed) {
    button_pressed = 0;
    if (state == ST_PLAYING || state == ST_PAUSED) toggle_pause();
    else start_game();
  }

  if (step_pending) {
    step_pending = 0;
    SnakeEvent e = Snake_Step();
    if (e == SNAKE_EVT_DIED) {
      game_over();
    } else {
      if (e == SNAKE_EVT_ATE) {
        RGB_Blink(0, 255, 0, 2, 80);         /* green x2 */
        step_period_ms = Snake_GetStepPeriodMs();
        lcd_show_score(NULL);
        printf("[snake] score=%u\r\n", Snake_GetScore());
      }
      Snake_Draw();
      ssd1306_UpdateScreen();
    }
  }

  RGB_Update(now);
}
