/* keyboard.c - 4x4 matris tarama, debounce ve olay kuyruğu
 *
 * Akış:
 *   TIM6 ISR -> kb_scan_tick() -> durum değişimi -> ring buffer'a olay
 *   Oyun döngüsü -> DG_GetKey() -> kb_pop_event()
 */
#include "keyboard.h"
#include "board_config.h"
#include "doomkeys.h"

/* ---- Tuş haritası: [satır][sütun] -> Doom tuşu ----
 * Kendi PCB yerleşiminize göre düzenleyin. 0 = kullanılmıyor.
 */
static const uint8_t keymap[KB_ROWS][KB_COLS] = {
    /*  C0              C1               C2              C3            */
    { KEY_UPARROW,    KEY_FIRE,        KEY_USE,        KEY_ESCAPE   },  /* R0 */
    { KEY_LEFTARROW,  KEY_DOWNARROW,   KEY_RIGHTARROW, KEY_ENTER    },  /* R1 */
    { KEY_STRAFE_L,   KEY_RSHIFT,      KEY_STRAFE_R,   KEY_TAB      },  /* R2: RSHIFT=koş, TAB=harita */
    { '1',            '2',             '3',            '4'          },  /* R3: silah seçimi */
};

static const uint16_t row_pins[KB_ROWS] = KB_ROW_PINS;
static const uint16_t col_pins[KB_COLS] = KB_COL_PINS;

/* Debounce durumu */
static uint8_t  stable_state[KB_ROWS][KB_COLS];   /* kararlı durum */
static uint8_t  counter[KB_ROWS][KB_COLS];        /* değişim sayacı */
static uint8_t  current_row;

/* ---- Olay kuyruğu (ISR yazar, ana döngü okur; tek üretici/tek tüketici) ---- */
#define KB_QUEUE_SIZE 32   /* 2'nin kuvveti olmalı */
static volatile kb_event_t queue[KB_QUEUE_SIZE];
static volatile uint8_t q_head, q_tail;

static void queue_push(uint8_t pressed, uint8_t key)
{
    uint8_t next = (q_head + 1) & (KB_QUEUE_SIZE - 1);
    if (next == q_tail) return;            /* dolu: olayı düşür */
    queue[q_head].pressed = pressed;
    queue[q_head].doomkey = key;
    q_head = next;
}

bool kb_pop_event(kb_event_t *ev)
{
    if (q_tail == q_head) return false;
    ev->pressed = queue[q_tail].pressed;
    ev->doomkey = queue[q_tail].doomkey;
    q_tail = (q_tail + 1) & (KB_QUEUE_SIZE - 1);
    return true;
}

void kb_init(void)
{
    /* GPIO saatleri ve modlar CubeMX'te de ayarlanabilir; burada garanti altına alıyoruz */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};

    /* Satırlar: çıkış, varsayılan HIGH (pasif) */
    for (int r = 0; r < KB_ROWS; r++) {
        g.Pin   = row_pins[r];
        g.Mode  = GPIO_MODE_OUTPUT_PP;
        g.Pull  = GPIO_NOPULL;
        g.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(KB_ROW_PORT, &g);
        HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[r], GPIO_PIN_SET);
    }

    /* Sütunlar: giriş, pull-up (basılı = LOW) */
    for (int c = 0; c < KB_COLS; c++) {
        g.Pin  = col_pins[c];
        g.Mode = GPIO_MODE_INPUT;
        g.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(KB_COL_PORT, &g);
    }

    current_row = 0;
    HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[0], GPIO_PIN_RESET);
}

void kb_scan_tick(void)
{
    /* Önceki tick'te LOW çekilen satırın sütunlarını oku
     * (bir tick beklemek sinyalin oturmasını sağlar) */
    for (int c = 0; c < KB_COLS; c++) {
        uint8_t raw = (HAL_GPIO_ReadPin(KB_COL_PORT, col_pins[c]) == GPIO_PIN_RESET);

        if (raw != stable_state[current_row][c]) {
            if (++counter[current_row][c] >= KB_DEBOUNCE_SCANS) {
                stable_state[current_row][c] = raw;
                counter[current_row][c] = 0;
                uint8_t key = keymap[current_row][c];
                if (key) queue_push(raw, key);
            }
        } else {
            counter[current_row][c] = 0;
        }
    }

    /* Sonraki satıra geç */
    HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[current_row], GPIO_PIN_SET);
    current_row = (current_row + 1) % KB_ROWS;
    HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[current_row], GPIO_PIN_RESET);
}
