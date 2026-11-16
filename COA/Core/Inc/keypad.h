/*
 * keypad.h  -  FEATURE 3: the 5 shield buttons read as ONE analog signal (ADC)
 *
 * The LCD Keypad Shield does not give one GPIO per button. All five buttons
 * sit on a resistor ladder (voltage divider) connected to a single pin,
 * A0 = PA0 = ADC1_IN5. Each button shorts a different point of the ladder to
 * GND, so each button produces a different voltage on A0:
 *
 *        button   | ADC reading (12-bit, 0..4095) measured on our shield
 *        ---------+---------------------------------------------------
 *        RIGHT    |    0
 *        UP       | ~ 745
 *        DOWN     | ~1560
 *        LEFT     | ~2200
 *        SELECT   | ~2969
 *        released | ~3717
 *
 * The firmware converts this voltage with ADC1 and decides which button is
 * pressed by comparing the value with thresholds placed half-way between two
 * neighbouring levels (see keypad.c).
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

/* One ADC conversion: raw 12-bit value (0..4095), or 0xFFFF on ADC error */
uint16_t Keypad_ReadRaw(void);

/* Button held right now (no debounce) */
Key      Keypad_ReadKey(void);

/* Debounced "a button was just pressed" event. Call it every 10 ms.
 * Returns each press only once; KEY_NONE when nothing new happened. */
Key      Keypad_PollEvent(void);

#endif /* INC_KEYPAD_H_ */
