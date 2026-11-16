/*
 * snake.h
 *
 * Pure game logic + rendering into the OLED frame buffer.
 * No timers, no input, no LCD here: app.c glues everything together.
 *
 * Playfield: 128x64 OLED, 4x4 px cells -> 31 x 15 cells inside a 1 px frame.
 */

#ifndef INC_SNAKE_H_
#define INC_SNAKE_H_

#include <stdint.h>

#define SNAKE_CELL_PX   4
#define SNAKE_GRID_W    31
#define SNAKE_GRID_H    15
#define SNAKE_MAX_LEN   (SNAKE_GRID_W * SNAKE_GRID_H)

typedef enum { DIR_UP = 0, DIR_RIGHT, DIR_DOWN, DIR_LEFT } Direction;

typedef enum {
  SNAKE_EVT_NONE = 0,
  SNAKE_EVT_ATE,      /* ate the food, score increased */
  SNAKE_EVT_DIED      /* hit a wall or itself */
} SnakeEvent;

void       Snake_Reset(uint32_t seed);
/* Queue a direction change; a 180 degree turn is ignored */
void       Snake_SetDirection(Direction d);
SnakeEvent Snake_Step(void);
/* Draws frame + snake + food into the ssd1306 buffer (no UpdateScreen) */
void       Snake_Draw(void);

uint16_t   Snake_GetScore(void);
/* Milliseconds between two moves; decreases as the score grows */
uint16_t   Snake_GetStepPeriodMs(void);

#endif /* INC_SNAKE_H_ */
