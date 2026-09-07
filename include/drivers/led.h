#ifndef DRIVERS_LED_H
#define DRIVERS_LED_H

#include <stdbool.h>

typedef enum {
    LED_GREEN = 0
} LedId;

typedef enum {
    LED_BICOLOR_OFF = 0,
    LED_BICOLOR_RED,
    LED_BICOLOR_YELLOW
} LedBicolorState;

void Led_Init(void);
void Led_On(LedId led);
void Led_Off(LedId led);
void Led_Set(LedId led, bool enabled);
void Led_Toggle(LedId led);
void Led_SetBicolor(LedBicolorState state);

#endif
