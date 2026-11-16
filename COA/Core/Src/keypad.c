/*
 * keypad.c  -  FEATURE 3: buttons read through the ADC (analog resistor ladder)
 */

#include "keypad.h"

#define ADC_ERROR          0xFFFFU
#define DEBOUNCE_SAMPLES   2U      /* same key read twice, 10 ms apart */

/* Upper limit of each voltage band. A reading below `max` (and above the
 * previous band) means that key. Limits are half-way between the levels
 * measured on our shield (see keypad.h). Above 3350 = no key pressed. */
static const struct { uint16_t max; Key key; } key_bands[] = {
  {  400U, KEY_RIGHT  },   /* level    0 */
  { 1150U, KEY_UP     },   /* level ~ 745 */
  { 1900U, KEY_DOWN   },   /* level ~1560 */
  { 2600U, KEY_LEFT   },   /* level ~2200 */
  { 3350U, KEY_SELECT },   /* level ~2969, released ~3717 */
};

static ADC_HandleTypeDef *kp_hadc;
static Key     candidate   = KEY_NONE;  /* key seen in the last samples   */
static uint8_t same_count  = 0;         /* how many samples it was stable */
static Key     held_key    = KEY_NONE;  /* key already reported as press  */

void Keypad_Init(ADC_HandleTypeDef *hadc)
{
  kp_hadc    = hadc;
  candidate  = KEY_NONE;
  same_count = 0;
  held_key   = KEY_NONE;
  HAL_ADCEx_Calibration_Start(kp_hadc, ADC_SINGLE_ENDED);
}

/* Single software-triggered conversion of ADC1 channel 5 (PA0). */
uint16_t Keypad_ReadRaw(void)
{
  uint16_t value = ADC_ERROR;
  HAL_ADC_Start(kp_hadc);
  if (HAL_ADC_PollForConversion(kp_hadc, 10) == HAL_OK)
    value = (uint16_t)HAL_ADC_GetValue(kp_hadc);
  HAL_ADC_Stop(kp_hadc);
  return value;
}

/* Voltage -> key: find the band the reading falls in. */
Key Keypad_ReadKey(void)
{
  const uint16_t v = Keypad_ReadRaw();
  if (v == ADC_ERROR) return KEY_NONE;

  for (unsigned i = 0; i < sizeof key_bands / sizeof key_bands[0]; i++)
    if (v < key_bands[i].max) return key_bands[i].key;

  return KEY_NONE;
}

/* Debounce + "one event per press".
 * While a button is released the voltage slides through the other bands
 * (e.g. RIGHT -> ... -> SELECT -> released), so after a press is reported we
 * ignore everything until a stable "no key" reading is seen. */
Key Keypad_PollEvent(void)
{
  const Key k = Keypad_ReadKey();

  if (k != candidate) {          /* reading changed: restart counting */
    candidate  = k;
    same_count = 0;
  }
  if (same_count < DEBOUNCE_SAMPLES) same_count++;
  if (same_count < DEBOUNCE_SAMPLES) return KEY_NONE;   /* not stable yet */

  if (candidate == KEY_NONE) {   /* stable release: ready for next press */
    held_key = KEY_NONE;
    return KEY_NONE;
  }
  if (held_key != KEY_NONE)      /* this press was already reported */
    return KEY_NONE;

  held_key = candidate;          /* new press */
  return held_key;
}
