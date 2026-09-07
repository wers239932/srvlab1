#ifndef DRIVERS_BUTTON_H
#define DRIVERS_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_DEBOUNCE_MS 30U

void Button_Init(void);
void Button_Update(uint32_t current_time_ms);
bool Button_IsPressed(void);
bool Button_WasPressed(void);
bool Button_WasReleased(void);

#endif
