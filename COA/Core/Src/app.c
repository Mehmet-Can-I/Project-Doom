/*
 * app.c  -  Snake application: game state machine + glue between modules
 *
 *   FEATURE 1  OLED screen   -> playfield            (snake.c + ssd1306.c)
 *   FEATURE 2  LCD screen    -> score / best / state (lcd_hd44780.c)
 *   FEATURE 3  ADC keypad    -> direction + SELECT   (keypad.c)
 *   FEATURE 4  LED LD2       -> blinks on eat / death (status_led.c)
 *   FEATURE 5  Interrupts    -> TIM6 game tick + EXTI blue button (this file)
 */

#include "app.h"
#include "snake.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "lcd_hd44780.h"
#include "keypad.h"
#include "status_led.h"
#include "rgb_led.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim6;

typedef enum { ST_TITLE, ST_PLAYING, ST_PAUSED, ST_GAME_OVER } GameState;

/* Variables shared with interrupt handlers -> volatile */
static volatile GameState state = ST_TITLE;
static volatile uint8_t   step_pending;         /* TIM6 ISR: "move the snake now" */
static volatile uint8_t   button_pressed;       /* EXTI ISR: blue button pressed  */
static volatile uint16_t  step_period_ms = 200; /* snake speed                    */
static volatile uint16_t  ms_since_step;

static uint16_t best_score;
static uint32_t last_key_poll_ms;

/* ======================================================= FEATURE 5: interrupts
 * TIM6: 80 MHz / (79+1) / (999+1) = 1 kHz update interrupt.
 * Counts milliseconds and raises `step_pending` when the snake must move.
 * The game speed (step_period_ms) shrinks as the score grows.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM6) return;
  if (state != ST_PLAYING) return;             /* clock frozen while paused */

  if (++ms_since_step >= step_period_ms) {
    ms_since_step = 0;
    step_pending  = 1;
  }
}

/* EXTI line 13: blue USER button B1 (PC13), falling edge.
 * Interrupts the game at any moment. The ISR only sets a flag (with a 200 ms
 * software debounce); the slow LCD/OLED work is done in the main loop. */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t last_press_ms;
  if (GPIO_Pin != B1_Pin) return;

  const uint32_t now = HAL_GetTick();
  if (now - last_press_ms > 200) {
    last_press_ms  = now;
    button_pressed = 1;
  }
}

/* Changes state and restarts the step timer, atomically w.r.t. TIM6 ISR. */
static void enter_state(GameState s)
{
  __disable_irq();
  state         = s;
  ms_since_step = 0;
  step_pending  = 0;
  __enable_irq();
}

/* ============================================================= screen output */

static void oled_center_text(uint8_t y, const char *s, SSD1306_Font_t font)
{
  uint8_t w = (uint8_t)(strlen(s) * font.width);
  ssd1306_SetCursor((uint8_t)((SSD1306_WIDTH - w) / 2), y);
  ssd1306_WriteString((char *)s, font, White);
}

/* FEATURE 2: LCD line 1 = score, line 2 = best score (or a status text) */
static void lcd_show_score(void)
{
  char line[24];
  snprintf(line, sizeof line, "Score: %u", Snake_GetScore());
  LCD_PrintLine(0, line);
  snprintf(line, sizeof line, "Best:  %u", best_score);
  LCD_PrintLine(1, line);
}

static void lcd_show_paused(void)
{
  char line[24];
  snprintf(line, sizeof line, "Score: %u", Snake_GetScore());
  LCD_PrintLine(0, line);
  LCD_PrintLine(1, "    PAUSED");
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

static void show_game_over(void)
{
  char line[24];

  /* OLED: last frame of the game with a "GAME OVER" box on top */
  Snake_Draw();
  ssd1306_FillRectangle(14, 14, 113, 49, Black);
  ssd1306_DrawRectangle(14, 14, 113, 49, White);
  oled_center_text(18, "GAME OVER", Font_11x18);
  snprintf(line, sizeof line, "score %u", Snake_GetScore());
  oled_center_text(38, line, Font_6x8);
  ssd1306_UpdateScreen();

  /* LCD */
  snprintf(line, sizeof line, "Score:%u Best:%u", Snake_GetScore(), best_score);
  LCD_PrintLine(0, line);
  LCD_PrintLine(1, "SELECT = replay");
}

/* ============================================================ game actions */

static void start_game(void)
{
  /* Random seed: time + ADC noise + CPU cycle counter -> new game each time */
  uint32_t seed = HAL_GetTick() ^ ((uint32_t)Keypad_ReadRaw() << 16) ^ DWT->CYCCNT;
  Snake_Reset(seed);

  step_period_ms = Snake_GetStepPeriodMs();
  enter_state(ST_PLAYING);

  RGB_Set(0, 0, 0);
  StatusLed_Off();
  Snake_Draw();
  ssd1306_UpdateScreen();
  lcd_show_score();
  printf("[snake] new game, seed=%08lx\r\n", (unsigned long)seed);
}

static void pause_game(void)
{
  enter_state(ST_PAUSED);
  lcd_show_paused();
  RGB_Set(0, 0, 40);                      /* dim blue while paused */
}

static void resume_game(void)
{
  RGB_Set(0, 0, 0);
  lcd_show_score();
  enter_state(ST_PLAYING);
}

static void on_fruit_eaten(void)
{
  StatusLed_Blink(200, 200);              /* FEATURE 4: one 200 ms flash */
  RGB_Blink(0, 255, 0, 2, 80);            /* green x2                    */
  step_period_ms = Snake_GetStepPeriodMs();  /* snake gets faster        */
  lcd_show_score();                       /* FEATURE 2: new score        */
  printf("[snake] score=%u\r\n", Snake_GetScore());
}

static void on_game_over(void)
{
  enter_state(ST_GAME_OVER);
  if (Snake_GetScore() > best_score) best_score = Snake_GetScore();
  StatusLed_Blink(200, 5000);             /* FEATURE 4: blink 5 s        */
  RGB_Blink(255, 0, 0, 3, 150);           /* red x3                      */
  show_game_over();
  printf("[snake] game over, score=%u best=%u\r\n", Snake_GetScore(), best_score);
}

/* SELECT (keypad) and the blue button (EXTI) do the same thing:
 * start / restart a game, or pause / resume the running one. */
static void on_select(void)
{
  switch (state) {
    case ST_PLAYING: pause_game();  break;
    case ST_PAUSED:  resume_game(); break;
    default:         start_game();  break;   /* title or game over */
  }
}

/* FEATURE 3: keypad event -> game action */
static void handle_key(Key k)
{
  if (k == KEY_SELECT) {
    on_select();
    return;
  }
  if (state != ST_PLAYING) return;        /* arrows only matter in game */

  switch (k) {
    case KEY_UP:    Snake_SetDirection(DIR_UP);    break;
    case KEY_DOWN:  Snake_SetDirection(DIR_DOWN);  break;
    case KEY_LEFT:  Snake_SetDirection(DIR_LEFT);  break;
    case KEY_RIGHT: Snake_SetDirection(DIR_RIGHT); break;
    default: break;
  }
}

/* ===================================================================== main */

void App_Init(void)
{
  LCD_Init();              /* before the OLED: they share PC7 (LCD E / OLED D/C) */
  ssd1306_Init();
  Keypad_Init(&hadc1);
  RGB_Init(&htim1);

  printf("\r\n[snake] boot - NUCLEO-L476RG snake\r\n");

  /* quick RGB self-test: R, G, B */
  RGB_Set(255, 0, 0); HAL_Delay(150);
  RGB_Set(0, 255, 0); HAL_Delay(150);
  RGB_Set(0, 0, 255); HAL_Delay(150);
  RGB_Set(0, 0, 0);

  show_title();
  HAL_TIM_Base_Start_IT(&htim6);          /* FEATURE 5: start 1 kHz game tick */
}

void App_Loop(void)
{
  const uint32_t now = HAL_GetTick();

  /* FEATURE 3: sample the analog keypad every 10 ms */
  if (now - last_key_poll_ms >= 10) {
    last_key_poll_ms = now;
    Key k = Keypad_PollEvent();
    if (k != KEY_NONE) handle_key(k);
  }

  /* FEATURE 5: blue button, flagged by the EXTI interrupt */
  if (button_pressed) {
    button_pressed = 0;
    on_select();
  }

  /* FEATURE 5: snake step, flagged by the TIM6 interrupt */
  if (step_pending && state == ST_PLAYING) {
    step_pending = 0;
    SnakeEvent e = Snake_Step();
    if (e == SNAKE_EVT_DIED) {
      on_game_over();
    } else {
      if (e == SNAKE_EVT_ATE) on_fruit_eaten();
      Snake_Draw();                       /* FEATURE 1: redraw playfield */
      ssd1306_UpdateScreen();
    }
  }

  RGB_Update(now);
  StatusLed_Update(HAL_GetTick());
}
