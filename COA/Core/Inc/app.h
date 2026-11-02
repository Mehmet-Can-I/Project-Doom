/*
 * app.h
 *
 * Snake application: state machine + glue between game logic (snake.c),
 * OLED (ssd1306), LCD (lcd_hd44780), keypad (ADC) and RGB LED (PWM).
 *
 * Feature map for the course:
 *   OLED graphic screen  -> playfield               (GPIO SPI, ssd1306.c)
 *   LCD display          -> score / best / status   (GPIO 4-bit, lcd_hd44780.c)
 *   ADC                  -> keypad resistor ladder  (ADC1_IN5, keypad.c)
 *   PWM + RGB LED        -> blink when eating       (TIM1 CH1N..CH3N, rgb_led.c)
 *   Built-in green LD2   -> fruit flash / death blink (PA5 shared with OLED SCK)
 *   Interrupts           -> TIM6 1 kHz game tick, EXTI13 blue button (pause)
 *   UART                 -> debug log on ST-LINK VCP 115200 8N1 (printf)
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "main.h"

void App_Init(void);
void App_Loop(void);           /* call forever from main() */

/* Called from interrupt context */
void App_OnTick1ms(void);      /* TIM6 update, 1 kHz */
void App_OnUserButton(void);   /* B1 (PC13) falling edge */

#endif /* INC_APP_H_ */
