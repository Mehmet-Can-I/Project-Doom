/* keyboard.h - Custom matrix keyboard driver (mapped to Doom keys) */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* A single key event: pressed / released + Doom key code */
typedef struct {
    uint8_t pressed;   /* 1 = pressed, 0 = released */
    uint8_t doomkey;   /* KEY_xxx value from doomkeys.h */
} kb_event_t;

void kb_init(void);

/* Called from the TIM6 interrupt every KB_SCAN_PERIOD_MS (scans one row) */
void kb_scan_tick(void);

/* Returns true and fills *ev if an event is queued (used by DG_GetKey) */
bool kb_pop_event(kb_event_t *ev);

#endif /* KEYBOARD_H */
