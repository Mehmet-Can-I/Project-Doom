/* keyboard.c - 4x4 matrix scanning, debouncing and event queue
 *
 * Flow:
 *   TIM6 ISR -> kb_scan_tick() -> state change -> event pushed to ring buffer
 *   Game loop -> DG_GetKey() -> kb_pop_event()
 */
#include "keyboard.h"
#include "board_config.h"
#include "doomkeys.h"

/* ---- Key map: [row][col] -> Doom key ----
 * Edit to match your PCB layout. 0 = unused.
 */
static const uint8_t keymap[KB_ROWS][KB_COLS] = {
    /*  C0              C1               C2              C3            */
    { KEY_UPARROW,    KEY_FIRE,        KEY_USE,        KEY_ESCAPE   },  /* R0 */
    { KEY_LEFTARROW,  KEY_DOWNARROW,   KEY_RIGHTARROW, KEY_ENTER    },  /* R1 */
    { KEY_STRAFE_L,   KEY_RSHIFT,      KEY_STRAFE_R,   KEY_TAB      },  /* R2: RSHIFT=run, TAB=automap */
    { '1',            '2',             '3',            '4'          },  /* R3: weapon select */
};

static const uint16_t row_pins[KB_ROWS] = KB_ROW_PINS;
static const uint16_t col_pins[KB_COLS] = KB_COL_PINS;

/* Debounce state */
static uint8_t  stable_state[KB_ROWS][KB_COLS];   /* debounced state */
static uint8_t  counter[KB_ROWS][KB_COLS];        /* change counter */
static uint8_t  current_row;

/* ---- Event queue (ISR writes, main loop reads; single producer/single consumer) ---- */
#define KB_QUEUE_SIZE 32   /* must be a power of 2 */
static volatile kb_event_t queue[KB_QUEUE_SIZE];
static volatile uint8_t q_head, q_tail;

static void queue_push(uint8_t pressed, uint8_t key)
{
    uint8_t next = (q_head + 1) & (KB_QUEUE_SIZE - 1);
    if (next == q_tail) return;            /* full: drop the event */
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
    /* GPIO clocks and modes can also be set in CubeMX; done here to be safe */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};

    /* Rows: outputs, idle HIGH (inactive) */
    for (int r = 0; r < KB_ROWS; r++) {
        g.Pin   = row_pins[r];
        g.Mode  = GPIO_MODE_OUTPUT_PP;
        g.Pull  = GPIO_NOPULL;
        g.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(KB_ROW_PORT, &g);
        HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[r], GPIO_PIN_SET);
    }

    /* Columns: inputs with pull-up (pressed = LOW) */
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
    /* Read the columns of the row that was driven LOW on the previous tick
     * (waiting one tick lets the signal settle) */
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

    /* Advance to the next row */
    HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[current_row], GPIO_PIN_SET);
    current_row = (current_row + 1) % KB_ROWS;
    HAL_GPIO_WritePin(KB_ROW_PORT, row_pins[current_row], GPIO_PIN_RESET);
}
