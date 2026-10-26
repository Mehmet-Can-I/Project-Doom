/* lcd_ili9341.c - ILI9341 sürücüsü iskeleti
 * SPI1, 8-bit, CPOL=0 CPHA=0, en hızlı prescaler (80 MHz / 2 = 40 MHz).
 * TX DMA kanalı CubeMX'te SPI1_TX'e bağlanmalı.
 */
#include "lcd_ili9341.h"
#include "board_config.h"

static volatile uint8_t dma_busy;

#define CS_LOW()   HAL_GPIO_WritePin(LCD_CS_PORT,  LCD_CS_PIN,  GPIO_PIN_RESET)
#define CS_HIGH()  HAL_GPIO_WritePin(LCD_CS_PORT,  LCD_CS_PIN,  GPIO_PIN_SET)
#define DC_CMD()   HAL_GPIO_WritePin(LCD_DC_PORT,  LCD_DC_PIN,  GPIO_PIN_RESET)
#define DC_DATA()  HAL_GPIO_WritePin(LCD_DC_PORT,  LCD_DC_PIN,  GPIO_PIN_SET)

static void lcd_cmd(uint8_t c)
{
    DC_CMD(); CS_LOW();
    HAL_SPI_Transmit(&LCD_SPI, &c, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

static void lcd_data(const uint8_t *d, uint16_t n)
{
    DC_DATA(); CS_LOW();
    HAL_SPI_Transmit(&LCD_SPI, (uint8_t *)d, n, HAL_MAX_DELAY);
    CS_HIGH();
}

void lcd_init(void)
{
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);

    lcd_cmd(0x01); HAL_Delay(5);                 /* Software reset */
    lcd_cmd(0x11); HAL_Delay(120);               /* Sleep out */

    uint8_t madctl = 0x28;                       /* Yatay (landscape), BGR */
    lcd_cmd(0x36); lcd_data(&madctl, 1);

    uint8_t pixfmt = 0x55;                       /* 16 bit/piksel */
    lcd_cmd(0x3A); lcd_data(&pixfmt, 1);

    /* TODO: gamma / power control ayarları (panel datasheet'ine göre) */

    lcd_cmd(0x29);                               /* Display ON */
    lcd_fill(0x0000);
}

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t buf[4];
    buf[0] = x0 >> 8; buf[1] = x0; buf[2] = x1 >> 8; buf[3] = x1;
    lcd_cmd(0x2A); lcd_data(buf, 4);
    buf[0] = y0 >> 8; buf[1] = y0; buf[2] = y1 >> 8; buf[3] = y1;
    lcd_cmd(0x2B); lcd_data(buf, 4);
    lcd_cmd(0x2C);                               /* Memory write başlat */
    DC_DATA(); CS_LOW();                         /* Piksel akışı için CS açık kalır */
}

void lcd_write_line_dma(const uint16_t *pixels, uint16_t count)
{
    lcd_wait_dma();
    dma_busy = 1;
    /* NOT: ILI9341 big-endian bekler. Piksel verisini önceden byte-swap edin
     * (palet tablosunu swap'lı tutmak en ucuz yol). */
    HAL_SPI_Transmit_DMA(&LCD_SPI, (uint8_t *)pixels, count * 2);
}

void lcd_wait_dma(void)
{
    while (dma_busy) { /* __WFI() de kullanılabilir */ }
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &LCD_SPI) dma_busy = 0;
}

void lcd_fill(uint16_t color)
{
    static uint16_t line[LCD_WIDTH];
    uint16_t c = (color >> 8) | (color << 8);
    for (int i = 0; i < LCD_WIDTH; i++) line[i] = c;

    lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    for (int y = 0; y < LCD_HEIGHT; y++) lcd_write_line_dma(line, LCD_WIDTH);
    lcd_wait_dma();
    CS_HIGH();
}
