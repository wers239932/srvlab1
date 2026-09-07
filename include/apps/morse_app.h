#ifndef APPS_MORSE_APP_H
#define APPS_MORSE_APP_H

#include <stdint.h>

#define MORSE_LONG_PRESS_MS 500U
#define MORSE_DOT_MS 250U
#define MORSE_DASH_MS 750U
#define MORSE_SYMBOL_GAP_MS 250U
#define MORSE_INPUT_TIMEOUT_MS 1500U
#define MORSE_FEEDBACK_MS 180U
#define MORSE_MAX_SYMBOLS 32U

typedef enum {
    MORSE_DOT = 0,
    MORSE_DASH
} MorseSymbol;

void MorseApp_Init(uint32_t current_time_ms);
void MorseApp_Update(uint32_t current_time_ms);

#endif
