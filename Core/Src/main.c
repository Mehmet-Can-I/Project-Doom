/* main.c - Giriş noktası
 * CubeMX'in ürettiği main.c'ye sadece işaretli USER CODE bloklarını ekleyin.
 * CubeMX ayarları için README.md'ye bakın.
 */
#include "main.h"
#include "doomgeneric.h"
#include <stdio.h>

SPI_HandleTypeDef  hspi1;
DMA_HandleTypeDef  hdma_spi1_tx;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef  htim6;

/* CubeMX'in ürettiği fonksiyonlar */
void SystemClock_Config(void);   /* HSI16 + PLL -> SYSCLK 80 MHz (azami) */
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);   /* 80 MHz / (79+1) / (999+1) = 1 kHz */

/* printf -> USART2 (ST-LINK sanal COM port) */
int _write(int fd, char *p, int len)
{
    (void)fd;
    HAL_UART_Transmit(&huart2, (uint8_t *)p, len, HAL_MAX_DELAY);
    return len;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Cache ve prefetch: flash'tan kod/veri okuma performansı için kritik */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    MX_USART2_UART_Init();
    MX_TIM6_Init();

    /* USER CODE BEGIN 2 */
    printf("NUCLEO-L476RG Doom basliyor...\r\n");

    /* QSPI flash kullanılıyorsa burada memory-mapped moda alın:
     * qspi_init(); qspi_enable_memory_mapped(); */

    char *argv[] = { "doom", "-iwad", "doom1.wad", NULL };
    doomgeneric_Create(3, argv);                 /* DG_Init() burada çağrılır */

    while (1) {
        doomgeneric_Tick();                      /* oyun mantığı + DG_DrawFrame */
    }
    /* USER CODE END 2 */
}

/* Hata durumunda LED'i yakıp sonsuz döngü (I_Error buraya yönlendirilebilir) */
void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}
