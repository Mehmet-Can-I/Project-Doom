/* doomgeneric_stm32.c - doomgeneric platform katmanı (NUCLEO-L476RG)
 *
 * doomgeneric (github.com/ozkl/doomgeneric) motoru şu 6 fonksiyonu ister:
 *   DG_Init, DG_DrawFrame, DG_SleepMs, DG_GetTicksMs, DG_GetKey, DG_SetWindowTitle
 *
 * ÖNEMLİ: Orijinal doomgeneric DG_ScreenBuffer'ı 32-bit (ARGB) tutar ->
 * 320x200x4 = 256 KB, L476'nın toplam 128 KB RAM'inden büyük.
 * Bu yüzden motoru 8-bit paletli framebuffer kullanacak şekilde değiştiriyoruz
 * (i_video.c içinde I_FinishUpdate'in palet dönüşümünü buraya taşıyoruz).
 * Bkz. README.md -> "Bellek bütçesi".
 */
#include "doomgeneric.h"
#include "doomkeys.h"
#include "board_config.h"
#include "keyboard.h"
#include "lcd_ili9341.h"

/* Motorun çizdiği 8-bit paletli ekran (i_video.c'de I_VideoBuffer) */
extern uint8_t *I_VideoBuffer;

/* 256 renklik palet, RGB565 ve LCD için önceden byte-swap'lanmış.
 * I_SetPalette() içinden dg_set_palette() çağrılarak güncellenir. */
static uint16_t palette565[256];

/* Çift satır tamponu: biri DMA ile giderken diğeri doldurulur */
static uint16_t line_buf[2][SCREENWIDTH];

void dg_set_palette(const uint8_t *rgb /* 256*3 bayt */)
{
    for (int i = 0; i < 256; i++) {
        uint8_t r = rgb[i * 3 + 0], g = rgb[i * 3 + 1], b = rgb[i * 3 + 2];
        uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        palette565[i] = (c >> 8) | (c << 8);     /* SPI için byte swap */
    }
}

/* ------------------------------------------------------------------ */

void DG_Init(void)
{
    lcd_init();
    kb_init();
    HAL_TIM_Base_Start_IT(&htim6);               /* klavye tarama zamanlayıcısı */
}

void DG_DrawFrame(void)
{
    /* 320x200 oyunu 320x240 ekranın ortasına yerleştir (üst/alt 20 px boş) */
    const uint16_t y_off = (LCD_HEIGHT - SCREENHEIGHT) / 2;
    lcd_set_window(0, y_off, SCREENWIDTH - 1, y_off + SCREENHEIGHT - 1);

    for (int y = 0; y < SCREENHEIGHT; y++) {
        uint16_t       *dst = line_buf[y & 1];
        const uint8_t  *src = &I_VideoBuffer[y * SCREENWIDTH];

        for (int x = 0; x < SCREENWIDTH; x++)
            dst[x] = palette565[src[x]];

        lcd_write_line_dma(dst, SCREENWIDTH);    /* önceki satırın bitmesini bekler */
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
    (void)title;                                 /* pencere yok */
}

/* ------------------------------------------------------------------ */

/* TIM6 periyodik kesme -> klavye tarama */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6) kb_scan_tick();
}
