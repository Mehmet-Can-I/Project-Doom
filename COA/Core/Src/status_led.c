/*
 * status_led.c  -  FEATURE 4: LD2 blink patterns (eat / game over)
 */

#include "status_led.h"
#include "ssd1306.h"
#include "stm32l4xx_hal.h"

static uint8_t  led_on;          /* current state of LD2               */
static uint8_t  blinking;        /* 1 while a blink pattern is running */
static uint16_t blink_interval;  /* ms between two toggles             */
static uint16_t blink_duration;  /* total length of the pattern (ms)   */
static uint32_t blink_start;     /* HAL_GetTick() when it started      */

/* LD2 = PA5 = OLED clock: the OLED driver owns the pin (see status_led.h). */
static void led_set(uint8_t on)
{
  if (led_on == on) return;      /* only touch the shared wire on change */
  led_on = on;
  ssd1306_SetClockIdleHigh(on);
}

void StatusLed_Off(void)
{
  blinking = 0;
  led_set(0);
}

void StatusLed_Blink(uint16_t interval_ms, uint16_t duration_ms)
{
  blink_interval = interval_ms;
  blink_duration = duration_ms;
  blink_start    = HAL_GetTick();
  blinking       = 1;
  led_set(1);
}

void StatusLed_Update(uint32_t now_ms)
{
  if (!blinking) return;

  const uint32_t elapsed = now_ms - blink_start;
  if (elapsed >= blink_duration) {
    StatusLed_Off();                                    /* pattern finished */
  } else {
    led_set((elapsed / blink_interval) % 2U == 0);      /* on, off, on, ... */
  }
}
