/*
 * keypad.h
 *
 * 5 push-buttons of the LCD Keypad Shield, read through ONE analog input:
 * the buttons short different points of a resistor ladder to GND, so each
 * key gives a different voltage on A0 = PA0 = ADC1_IN5.
 *
 * ADC thresholds in keypad.c are calibrated on the connected lab shield.
 * Its released reading is about 3717, rather than the full-scale 4095;
 * SELECT reads about 2969, so the demo's thresholds do not apply directly.
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "stm32l4xx_hal.h"

typedef enum {
  KEY_NONE = 0,
  KEY_RIGHT,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_SELECT
} Key;

void     Keypad_Init(ADC_HandleTypeDef *hadc);
/* Raw 12-bit ADC value (0..4095), or 0xFFFF on ADC error */
uint16_t Keypad_ReadRaw(void);
/* Key currently held (no debounce) */
Key      Keypad_ReadKey(void);
/* Debounced "key was just pressed" event. Call it every ~10 ms.
 * One event per press; all keys must be released before the next event.
 * Returns KEY_NONE when nothing new happened. */
Key      Keypad_PollEvent(void);

#endif /* INC_KEYPAD_H_ */
