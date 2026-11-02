/*
 * keypad.c
 */

#include "keypad.h"

static ADC_HandleTypeDef *kp_hadc;
static Key kp_last_stable = KEY_NONE;
static Key kp_candidate   = KEY_NONE;
static uint8_t kp_same_count = 0;

#define KP_DEBOUNCE_SAMPLES 2   /* 2 identical reads, 10 ms apart */

void Keypad_Init(ADC_HandleTypeDef *hadc)
{
  kp_hadc = hadc;
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
  if (v < 500)  return KEY_RIGHT;
  if (v < 1000) return KEY_UP;
  if (v < 2000) return KEY_DOWN;
  if (v < 3000) return KEY_LEFT;
  if (v < 3900) return KEY_SELECT;
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

  if (kp_same_count >= KP_DEBOUNCE_SAMPLES && kp_candidate != kp_last_stable) {
    kp_last_stable = kp_candidate;
    return kp_last_stable;      /* new press (KEY_NONE on release) */
  }
  return KEY_NONE;
}
