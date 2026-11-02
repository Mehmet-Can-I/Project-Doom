/*
 * rgb_led.c
 */

#include "rgb_led.h"

static TIM_HandleTypeDef *rgb_htim;

static struct {
  uint8_t r, g, b;
  uint8_t toggles_left;     /* 2 per blink (on + off) */
  uint16_t half_period;
  uint32_t next_ms;
  uint8_t is_on;
  uint8_t started;
} blink;

static uint32_t to_ccr(uint8_t v)
{
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(rgb_htim);
  uint32_t ccr = ((uint32_t)v * (arr + 1U)) / 255U;
#if RGB_COMMON_ANODE
  ccr = (arr + 1U) - ccr;
#endif
  return ccr;
}

void RGB_Init(TIM_HandleTypeDef *htim)
{
  rgb_htim = htim;
  RGB_Set(0, 0, 0);
  HAL_TIMEx_PWMN_Start(rgb_htim, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(rgb_htim, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(rgb_htim, TIM_CHANNEL_3);
}

void RGB_Set(uint8_t r, uint8_t g, uint8_t b)
{
  __HAL_TIM_SET_COMPARE(rgb_htim, TIM_CHANNEL_1, to_ccr(r));
  __HAL_TIM_SET_COMPARE(rgb_htim, TIM_CHANNEL_2, to_ccr(g));
  __HAL_TIM_SET_COMPARE(rgb_htim, TIM_CHANNEL_3, to_ccr(b));
}

void RGB_Blink(uint8_t r, uint8_t g, uint8_t b, uint8_t count, uint16_t half_period_ms)
{
  blink.r = r; blink.g = g; blink.b = b;
  blink.toggles_left = (uint8_t)(count * 2U);
  blink.half_period = half_period_ms;
  blink.is_on = 0;
  blink.started = 0;   /* first toggle on next RGB_Update() */
}

void RGB_Update(uint32_t now_ms)
{
  if (blink.toggles_left == 0) return;

  if (!blink.started) {
    blink.started = 1;
    blink.next_ms = now_ms;
  }
  if ((int32_t)(now_ms - blink.next_ms) < 0) return;

  blink.is_on = !blink.is_on;
  if (blink.is_on) RGB_Set(blink.r, blink.g, blink.b);
  else             RGB_Set(0, 0, 0);

  blink.toggles_left--;
  blink.next_ms = now_ms + blink.half_period;
}
