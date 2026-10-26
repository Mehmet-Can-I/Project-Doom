/* keyboard.h - Özel matris klavye sürücüsü (Doom tuşlarına eşlenmiş) */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* Tek bir tuş olayı: basıldı / bırakıldı + Doom tuş kodu */
typedef struct {
    uint8_t pressed;   /* 1 = basıldı, 0 = bırakıldı */
    uint8_t doomkey;   /* doomkeys.h içindeki KEY_xxx değeri */
} kb_event_t;

void kb_init(void);

/* TIM6 kesmesinden her KB_SCAN_PERIOD_MS'de çağrılır (bir satır tarar) */
void kb_scan_tick(void);

/* Kuyrukta olay varsa true döner ve *ev doldurulur (DG_GetKey kullanır) */
bool kb_pop_event(kb_event_t *ev);

#endif /* KEYBOARD_H */
