/* doomgeneric_stm32.c - doomgeneric platform layer (NUCLEO-L476RG)
 *
 * The doomgeneric engine (github.com/ozkl/doomgeneric) requires these 6 functions:
 *   DG_Init, DG_DrawFrame, DG_SleepMs, DG_GetTicksMs, DG_GetKey, DG_SetWindowTitle
 *
 * IMPORTANT: Stock doomgeneric keeps DG_ScreenBuffer as 32-bit (ARGB) ->
 * 320x200x4 = 256 KB, larger than the L476's total 128 KB of RAM.
 * So we modify the engine to use an 8-bit palettized framebuffer
 * (moving the palette conversion from I_FinishUpdate in i_video.c to here).
 * See README.md -> "Memory budget".
 */
#include "doomgeneric.h"
#include "doomkeys.h"
#include "board_config.h"
#include "keyboard.h"
#include "lcd_ili9341.h"

/* 8-bit palettized screen drawn by the engine (I_VideoBuffer in i_video.c) */
extern uint8_t *I_VideoBuffer;

/* 256-color palette in RGB565, pre-byte-swapped for the LCD.
 * Updated by calling dg_set_palette() from I_SetPalette(). */
static uint16_t palette565[256];

/* Double line buffer: one is sent via DMA while the other is filled */
static uint16_t line_buf[2][SCREENWIDTH];

void dg_set_palette(const uint8_t *rgb /* 256*3 bytes */)
{
    for (int i = 0; i < 256; i++) {
        uint8_t r = rgb[i * 3 + 0], g = rgb[i * 3 + 1], b = rgb[i * 3 + 2];
        uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        palette565[i] = (c >> 8) | (c << 8);     /* byte swap for SPI */
    }
}

/* ------------------------------------------------------------------ */

void DG_Init(void)
{
    lcd_init();
    kb_init();
    HAL_TIM_Base_Start_IT(&htim6);               /* keyboard scan timer */
}

void DG_DrawFrame(void)
{
    /* Center the 320x200 game on the 320x240 screen (20 px blank top/bottom) */
    const uint16_t y_off = (LCD_HEIGHT - SCREENHEIGHT) / 2;
    lcd_set_window(0, y_off, SCREENWIDTH - 1, y_off + SCREENHEIGHT - 1);

    for (int y = 0; y < SCREENHEIGHT; y++) {
        uint16_t       *dst = line_buf[y & 1];
        const uint8_t  *src = &I_VideoBuffer[y * SCREENWIDTH];

        for (int x = 0; x < SCREENWIDTH; x++)
            dst[x] = palette565[src[x]];

        lcd_write_line_dma(dst, SCREENWIDTH);    /* waits for the previous line */
    }
    lcd_wait_dma();
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void DG_SleepMs(uint32_t ms)
{
    HAL_Delay(ms);
}

uint32_t DG_GetTicksMs(void)
{
    return HAL_GetTick();
}

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
    kb_event_t ev;
    if (!kb_pop_event(&ev)) return 0;
    *pressed = ev.pressed;
    *doomKey = ev.doomkey;
    return 1;
}

void DG_SetWindowTitle(const char *title)
{
    (void)title;                                 /* no window */
}

/* ------------------------------------------------------------------ */

/* TIM6 periodic interrupt -> keyboard scan */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) kb_scan_tick();
}
