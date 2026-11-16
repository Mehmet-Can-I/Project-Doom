/*
 * app.h
 *
 * Snake application: game state machine + glue between the modules.
 *
 * Feature map for the course:
 *   1. OLED graphic screen -> playfield              (ssd1306.c, snake.c)
 *   2. LCD display         -> score / best / status  (lcd_hd44780.c)
 *   3. ADC                 -> 5 keypad buttons on one analog pin (keypad.c)
 *   4. LED LD2             -> blinks on fruit / death (status_led.c)
 *   5. Interrupts          -> TIM6 1 kHz game tick + EXTI13 blue button (app.c)
 *
 * Extras: PWM RGB LED on TIM1 (rgb_led.c), UART debug log (printf, 115200 8N1).
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

void App_Init(void);   /* once, after the CubeMX peripheral init */
void App_Loop(void);   /* forever, from main()'s while(1)         */

#endif /* INC_APP_H_ */
