/*
 * keypad.c
 */

#include "keypad.h"

static ADC_HandleTypeDef *kp_hadc;
static Key kp_last_stable = KEY_NONE;
static Key kp_candidate   = KEY_NONE;
static uint8_t kp_same_count = 0;

#define KP_DEBOUNCE_SAMPLES 2   /* 2 identical reads, 10 ms apart */

/* Calibrated on the connected lab shield (12-bit ADC readings):
 * RIGHT 0, UP ~745, DOWN ~1560, LEFT ~2200, SELECT ~2969, idle ~3717.
 * Thresholds sit between the measured levels; idle is below full scale. */
#define KP_RIGHT_MAX   400U
#define KP_UP_MAX     1150U
#define KP_DOWN_MAX   1900U
#define KP_LEFT_MAX   2600U
#define KP_SELECT_MAX 3350U

void Keypad_Init(ADC_HandleTypeDef *hadc)
{
  kp_hadc = hadc;
  kp_last_stable = KEY_NONE;
  kp_candidate = KEY_NONE;
  kp_same_count = 0;
  HAL_ADCEx_Calibration_Start(kp_hadc, ADC_SINGLE_ENDED);
}

uint16_t Keypad_ReadRaw(void)
{
  uint16_t value = 0xFFFF;
  HAL_ADC_Start(kp_hadc);
  if (HAL_ADC_PollForConversion(kp_hadc, 10) == HAL_OK)
    value = (uint16_t)HAL_ADC_GetValue(kp_hadc);
  HAL_ADC_Stop(kp_hadc);
  return value;
}

Key Keypad_ReadKey(void)
{
  uint16_t v = Keypad_ReadRaw();
  if (v == 0xFFFF) return KEY_NONE;
  if (v < KP_RIGHT_MAX)  return KEY_RIGHT;
  if (v < KP_UP_MAX)     return KEY_UP;
  if (v < KP_DOWN_MAX)   return KEY_DOWN;
  if (v < KP_LEFT_MAX)   return KEY_LEFT;
  if (v < KP_SELECT_MAX) return KEY_SELECT;
  return KEY_NONE;
}

Key Keypad_PollEvent(void)
{
  Key k = Keypad_ReadKey();

  if (k == kp_candidate) {
    if (kp_same_count < KP_DEBOUNCE_SAMPLES) kp_same_count++;
  } else {
    kp_candidate = k;
    kp_same_count = 1;
  }

  if (kp_same_count >= KP_DEBOUNCE_SAMPLES) {
    /* Latch one key per press until a debounced release. The ADC voltage
     * can cross other key bands (including SELECT) as an arrow is released. */
    if (kp_candidate == KEY_NONE) {
      kp_last_stable = KEY_NONE;
    } else if (kp_last_stable == KEY_NONE) {
      kp_last_stable = kp_candidate;
      return kp_last_stable;
    }
  }
  return KEY_NONE;
}
