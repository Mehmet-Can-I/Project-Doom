/*
 * snake.c  -  game logic + drawing of the playfield into the OLED buffer
 *
 * Grid of 31 x 15 cells, 4x4 px each, inside a 1 px border on the 128x64 OLED.
 * The body is a ring buffer (no memmove when the snake moves): body[head]
 * is the head, older segments are behind it. occupied[][] gives an O(1)
 * "is this cell part of the snake?" test for collisions and food placement.
 */

#include "snake.h"
#include "ssd1306.h"
#include <string.h>

#define START_LEN        3
#define START_PERIOD_MS  220
#define MIN_PERIOD_MS    80
#define SPEEDUP_MS       8    /* faster by this much per food */

typedef struct { uint8_t x, y; } Cell;

static Cell     body[SNAKE_MAX_LEN];
static uint16_t head;          /* index of head in body[] */
static uint16_t length;
static uint8_t  occupied[SNAKE_GRID_H][SNAKE_GRID_W];

static Direction dir;          /* direction used by the last step */
static Direction next_dir;     /* direction requested for the next step */
static Cell      food;
static uint16_t  score;
static uint32_t  rng_state;

/* ----------------------------------------------------------------- helpers */

static uint32_t rng_next(void)        /* xorshift32 */
{
  uint32_t x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;
  return x;
}

static uint16_t tail_index(void)
{
  return (uint16_t)((head + SNAKE_MAX_LEN - (length - 1U)) % SNAKE_MAX_LEN);
}

static void place_food(void)
{
  if (length >= SNAKE_MAX_LEN) return;    /* board full: you won */
  do {
    food.x = (uint8_t)(rng_next() % SNAKE_GRID_W);
    food.y = (uint8_t)(rng_next() % SNAKE_GRID_H);
  } while (occupied[food.y][food.x]);
}

/* FEATURE 1: cell (cx,cy) -> pixels. 2 px offset = 1 px border + 1 px gap */
static void draw_cell_filled(uint8_t cx, uint8_t cy)
{
  uint8_t x = (uint8_t)(2 + cx * SNAKE_CELL_PX);
  uint8_t y = (uint8_t)(2 + cy * SNAKE_CELL_PX);
  ssd1306_FillRectangle(x, y, x + 2, y + 2, White);   /* 3x3, 1px gap */
}

static void draw_food(uint8_t cx, uint8_t cy)
{
  uint8_t x = (uint8_t)(2 + cx * SNAKE_CELL_PX);
  uint8_t y = (uint8_t)(2 + cy * SNAKE_CELL_PX);
  /* small "+" so food looks different from the body */
  ssd1306_DrawPixel(x + 1, y,     White);
  ssd1306_DrawPixel(x,     y + 1, White);
  ssd1306_DrawPixel(x + 1, y + 1, White);
  ssd1306_DrawPixel(x + 2, y + 1, White);
  ssd1306_DrawPixel(x + 1, y + 2, White);
}

/* ------------------------------------------------------------------ public */

void Snake_Reset(uint32_t seed)
{
  rng_state = seed ? seed : 0x12345678u;
  memset(occupied, 0, sizeof(occupied));

  length = START_LEN;
  head = START_LEN - 1;
  for (uint16_t i = 0; i < START_LEN; i++) {
    body[i].x = (uint8_t)(SNAKE_GRID_W / 2 - (START_LEN - 1) + i);
    body[i].y = SNAKE_GRID_H / 2;
    occupied[body[i].y][body[i].x] = 1;
  }
  dir = next_dir = DIR_RIGHT;
  score = 0;
  place_food();
}

void Snake_SetDirection(Direction d)
{
  /* Compare with the direction actually used last step, so pressing
   * UP then LEFT quickly while going RIGHT can't make the snake reverse. */
  if (((d + 2) & 3) == dir) return;
  next_dir = d;
}

SnakeEvent Snake_Step(void)
{
  dir = next_dir;

  Cell h = body[head];
  int nx = h.x, ny = h.y;
  switch (dir) {
    case DIR_UP:    ny--; break;
    case DIR_DOWN:  ny++; break;
    case DIR_LEFT:  nx--; break;
    case DIR_RIGHT: nx++; break;
  }

  /* wall */
  if (nx < 0 || ny < 0 || nx >= SNAKE_GRID_W || ny >= SNAKE_GRID_H)
    return SNAKE_EVT_DIED;

  const uint8_t eating = (nx == food.x && ny == food.y);

  /* the tail moves away this step unless we grow -> free its cell first */
  if (!eating) {
    Cell t = body[tail_index()];
    occupied[t.y][t.x] = 0;
  }

  if (occupied[ny][nx])
    return SNAKE_EVT_DIED;

  head = (uint16_t)((head + 1U) % SNAKE_MAX_LEN);
  body[head].x = (uint8_t)nx;
  body[head].y = (uint8_t)ny;
  occupied[ny][nx] = 1;

  if (eating) {
    length++;
    score++;
    place_food();
    return SNAKE_EVT_ATE;
  }
  return SNAKE_EVT_NONE;
}

/* FEATURE 1: redraw the whole playfield into the frame buffer.
 * ssd1306_UpdateScreen() then only transmits the bytes that changed. */
void Snake_Draw(void)
{
  ssd1306_Fill(Black);
  ssd1306_DrawRectangle(0, 0, SSD1306_WIDTH - 1, SSD1306_HEIGHT - 1, White);

  uint16_t idx = head;
  for (uint16_t i = 0; i < length; i++) {
    draw_cell_filled(body[idx].x, body[idx].y);
    idx = (uint16_t)((idx + SNAKE_MAX_LEN - 1U) % SNAKE_MAX_LEN);
  }
  if (length < SNAKE_MAX_LEN)
    draw_food(food.x, food.y);
}

uint16_t Snake_GetScore(void)  { return score; }

uint16_t Snake_GetStepPeriodMs(void)
{
  int32_t p = START_PERIOD_MS - (int32_t)score * SPEEDUP_MS;
  return (uint16_t)(p < MIN_PERIOD_MS ? MIN_PERIOD_MS : p);
}
