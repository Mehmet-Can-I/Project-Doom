/*
 * status_led.h  -  FEATURE 4: board LED LD2 blinks on game events
 *
 *   - snake eats a fruit -> one 200 ms flash
 *   - snake dies         -> blinks every 200 ms for 5 s
 *
 * Hardware constraint: on the Nucleo-L476RG the green LED LD2 is on PA5,
 * and PA5 is ALSO the clock line (SCLK) of the OLED screen. LD2 is lit when
 * PA5 is HIGH. So the LED cannot be driven with a simple HAL_GPIO_WritePin():
 * a random edge on PA5 would be taken by the OLED as a clock pulse.
 * Instead we ask the OLED driver to leave its clock line idle HIGH (LED on)
 * or idle LOW (LED off) between transfers: ssd1306_SetClockIdleHigh().
 *
 * The blinking is non-blocking: StatusLed_Update() is called every main-loop
 * iteration and decides from HAL_GetTick() if the LED must be on or off.
 */

#ifndef INC_STATUS_LED_H_
#define INC_STATUS_LED_H_

#include <stdint.h>

/* Start blinking: LED toggles every `interval_ms`, for `duration_ms` total */
void StatusLed_Blink(uint16_t interval_ms, uint16_t duration_ms);
/* Stop any blinking and switch LD2 off */
void StatusLed_Off(void);
/* Call from the main loop */
void StatusLed_Update(uint32_t now_ms);

#endif /* INC_STATUS_LED_H_ */
