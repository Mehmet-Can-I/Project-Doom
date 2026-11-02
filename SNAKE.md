# Snake prototype — NUCLEO-L476RG (branch `snake-prototype`)

Minimal Snake: playfield on the **OLED**, score on the **LCD**, the **RGB LED** blinks
when the snake eats, and the **keypad** steers it. The project is `COA/`; open it in STM32CubeIDE and build.

## Hardware stack

| Board | Where | Used for |
|---|---|---|
| NUCLEO-L476RG | on the lab base-stand ("Nucleo-64 key") | MCU, 80 MHz (HSI + PLL) |
| SSD1309 128x64 OLED | soldered on the base-stand (Z3) | game screen |
| LCD Keypad Shield (HD44780 16x2 + 5 keys) | either Arduino slot of the base-stand | score + controls |
| RGB LED (common cathode) + 3 x 220–330 Ω | wires to morpho CN10 | eat / game-over feedback |

## Course features covered

| Feature | Implementation | Files |
|---|---|---|
| OLED graphic screen | SPI1 @10 MHz, lab SSD1306 lib (patched) | `ssd1306.c` |
| LCD display | HD44780 4-bit, µs delays (DWT) | `lcd_hd44780.c` |
| ADC | keypad resistor ladder on ADC1_IN5 (PA0) | `keypad.c` |
| PWM + RGB LEDs | TIM1 CH1N/CH2N/CH3N, 1 kHz | `rgb_led.c` |
| Interrupts | TIM6 1 kHz game tick + EXTI13 blue button (pause) | `main.c`, `app.c` |
| UART | `printf` log on ST-LINK VCP, 115200 8N1 | `main.c` (`__io_putchar`) |

Possible later additions: DMA (OLED frame over SPI DMA, or ADC in DMA mode), MEMS (tilt to steer, X-NUCLEO-IKS01A3),
the lab's custom keyboard (SX1509 on I2C1, connectors Z1/Z2 of the base-stand).

## Pin map

| Signal | Pin | Arduino / morpho |
|---|---|---|
| OLED SCK / MOSI | PA5 / PA7 (SPI1) | D13 / D11 |
| OLED CS | PB6 | D10 (also LCD backlight) |
| OLED D/C **and** LCD E | **PC7** | D9 |
| OLED RES | NRST (hard-wired) | — |
| LCD RS | PA9 | D8 |
| LCD DB4..DB7 | PB5, PB4, PB10, PA8 | D4..D7 |
| Keypad (analog) | PA0 = ADC1_IN5 | A0 |
| RGB R / G / B | PB13 / PB14 / PB15 (TIM1_CH1N..3N) | CN10 pins 30 / 28 / 26 |
| UART TX / RX | PA2 / PA3 (USART2) | ST-LINK VCP |
| Blue button | PC13 (EXTI) | B1 |

The RGB pins are a proposal: if the lab RGB module sits elsewhere, change `MX_TIM1_Init`, `HAL_TIM_MspPostInit`
and `rgb_led.h`. For a common-anode LED, set `RGB_COMMON_ANODE` to `1`.

## Shared pins between the OLED and the LCD shield

The base-stand wires both Arduino slots and the OLED to the same pins, so the two displays share some lines:

- **PC7 = OLED D/C = LCD E.** The LCD latches its data lines on every falling edge of E. The OLED driver therefore:
  keeps D/C low when idle; sends each frame in exactly **two** data bursts, which gives exactly two falling edges
  (one complete 4-bit LCD byte); and before each edge calls `ssd1306_BeforeDcFallingEdge()`. The LCD driver
  implements that function and puts the two halves of `0x0C` on its data lines. `0x0C` is "display on, cursor off",
  which is harmless. Both displays are only accessed from the main loop, never from an interrupt.
- **PA9**: the lab OLED test code drove it as the OLED reset, but on the base-stand RES is wired to NRST and PA9 is
  the LCD RS line. The OLED driver no longer touches it.
- **PB6 = OLED CS = LCD backlight (D10).** CS stays high when idle, so the backlight stays on. It dims briefly during
  each OLED frame, which is not visible.
- **PA5 = LD2 (green user LED).** PA5 is used by SPI1, so the board LED is disabled.

## Controls

| Key | Action |
|---|---|
| UP / DOWN / LEFT / RIGHT | steer |
| SELECT | start, pause/resume, replay |
| Blue button B1 | pause/resume (interrupt), or start |

Game: 31x15 grid of 4 px cells. The first move takes 220 ms, each food makes the snake 8 ms faster, down to 80 ms
per move. The RGB LED blinks green twice when the snake eats and red three times on game over.

## Code layout (`COA/Core`)

```
Src/main.c          CubeMX init (GPIO, ADC1, SPI1, TIM1, TIM6, USART2) + HAL callbacks
Src/app.c           state machine TITLE -> PLAYING <-> PAUSED -> GAME_OVER, glue
Src/snake.c         game logic (ring buffer + occupancy grid) and drawing
Src/ssd1306*.c      OLED lib from COA-LAB-DOCUMENTATION (low-level patched)
Src/lcd_hd44780.c   LCD driver
Src/keypad.c        ADC keypad + debounce
Src/rgb_led.c       PWM RGB + non-blocking blink
```

HAL drivers for ADC, SPI, TIM and UART (v1.13.6, the same version as the rest of the project) were added under
`Drivers/`. `COA.ioc` was updated to match, so *Generate Code* in CubeMX keeps the setup: all user code sits in
`USER CODE` blocks or in separate files.
