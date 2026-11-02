/*
 * rgb_led.h
 *
 * RGB LED driven by 3 PWM channels of TIM1 (complementary outputs):
 *   R -> TIM1_CH1N = PB13  (morpho CN10 pin 30)
 *   G -> TIM1_CH2N = PB14  (morpho CN10 pin 28)
 *   B -> TIM1_CH3N = PB15  (morpho CN10 pin 26)
 * PWM: 80 MHz / 80 = 1 MHz, ARR = 999 -> 1 kHz, duty 0..999.
 *
 * Wiring (common cathode): each pin -> ~220-330 ohm -> LED anode leg,
 * common cathode -> GND. For a common-anode LED set RGB_COMMON_ANODE to 1.
 *
 * !! Pins are a proposal: change them here + in MX_TIM1_Init/HAL_TIM_MspPostInit
 *    if the lab RGB module is wired elsewhere.
 */

#ifndef INC_RGB_LED_H_
#define INC_RGB_LED_H_

#include "stm32l4xx_hal.h"

#ifndef RGB_COMMON_ANODE
#define RGB_COMMON_ANODE 0
#endif

void RGB_Init(TIM_HandleTypeDef *htim);
/* 0..255 per channel */
void RGB_Set(uint8_t r, uint8_t g, uint8_t b);
/* Non-blocking blink: `count` times color on/off, `half_period_ms` each */
void RGB_Blink(uint8_t r, uint8_t g, uint8_t b, uint8_t count, uint16_t half_period_ms);
/* Call often from the main loop (drives the blink animation) */
void RGB_Update(uint32_t now_ms);

#endif /* INC_RGB_LED_H_ */
