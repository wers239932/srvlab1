#include "apps/morse_app.h"

#include <stdbool.h>
#include <stddef.h>

#include "drivers/button.h"
#include "drivers/led.h"

typedef enum {
    MORSE_STATE_INPUT = 0,
    MORSE_STATE_TRANSMIT_ON,
    MORSE_STATE_TRANSMIT_GAP
} MorseAppState;

static MorseSymbol sequence[MORSE_MAX_SYMBOLS];
static size_t sequence_length;
static size_t transmit_index;
static MorseAppState state;
static bool press_active;
static bool feedback_active;
static uint32_t press_started_ms;
static uint32_t last_input_ms;
static uint32_t feedback_deadline_ms;
static uint32_t state_deadline_ms;

static bool MorseApp_DeadlineReached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static uint32_t MorseApp_SymbolDuration(MorseSymbol symbol)
{
    return symbol == MORSE_DASH ? MORSE_DASH_MS : MORSE_DOT_MS;
}

static void MorseApp_StartTransmission(uint32_t now)
{
    feedback_active = false;
    Led_SetBicolor(LED_BICOLOR_OFF);

    transmit_index = 0U;
    Led_On(LED_GREEN);
    state_deadline_ms = now + MorseApp_SymbolDuration(sequence[0]);
    state = MORSE_STATE_TRANSMIT_ON;
}

static void MorseApp_FinishTransmission(uint32_t now)
{
    sequence_length = 0U;
    transmit_index = 0U;
    last_input_ms = now;
    state = MORSE_STATE_INPUT;
}

static void MorseApp_HandleInput(uint32_t now)
{
    const bool was_pressed = Button_WasPressed();
    const bool was_released = Button_WasReleased();

    if (was_pressed && !press_active) {
        press_active = true;
        press_started_ms = now;
        feedback_active = false;
        Led_SetBicolor(LED_BICOLOR_OFF);
    }

    if (was_released && press_active) {
        const uint32_t press_duration_ms = now - press_started_ms;
        const MorseSymbol symbol = press_duration_ms >= MORSE_LONG_PRESS_MS
                                       ? MORSE_DASH
                                       : MORSE_DOT;

        press_active = false;
        last_input_ms = now;

        if (sequence_length < MORSE_MAX_SYMBOLS) {
            sequence[sequence_length++] = symbol;
            Led_SetBicolor(symbol == MORSE_DOT
                               ? LED_BICOLOR_YELLOW
                               : LED_BICOLOR_RED);
            feedback_deadline_ms = now + MORSE_FEEDBACK_MS;
            feedback_active = true;
        } else {
            /* Further symbols are ignored until this full sequence is sent. */
            feedback_active = false;
            Led_SetBicolor(LED_BICOLOR_OFF);
        }
    }

    if (feedback_active && MorseApp_DeadlineReached(now, feedback_deadline_ms)) {
        feedback_active = false;
        Led_SetBicolor(LED_BICOLOR_OFF);
    }

    if ((sequence_length > 0U) && !press_active &&
        ((uint32_t)(now - last_input_ms) >= MORSE_INPUT_TIMEOUT_MS)) {
        MorseApp_StartTransmission(now);
    }

}

void MorseApp_Init(uint32_t current_time_ms)
{
    sequence_length = 0U;
    transmit_index = 0U;
    state = MORSE_STATE_INPUT;
    press_active = Button_IsPressed();
    feedback_active = false;
    press_started_ms = current_time_ms;
    last_input_ms = current_time_ms;
    feedback_deadline_ms = current_time_ms;
    state_deadline_ms = current_time_ms;

    Led_Off(LED_GREEN);
    Led_SetBicolor(LED_BICOLOR_OFF);
}

void MorseApp_Update(uint32_t current_time_ms)
{
    switch (state) {
    case MORSE_STATE_INPUT:
        MorseApp_HandleInput(current_time_ms);
        break;

    case MORSE_STATE_TRANSMIT_ON:
        (void)Button_WasPressed();
        (void)Button_WasReleased();

        if (MorseApp_DeadlineReached(current_time_ms, state_deadline_ms)) {
            Led_Off(LED_GREEN);
            ++transmit_index;
            state_deadline_ms = current_time_ms + MORSE_SYMBOL_GAP_MS;
            state = MORSE_STATE_TRANSMIT_GAP;
        }
        break;

    case MORSE_STATE_TRANSMIT_GAP:
        (void)Button_WasPressed();
        (void)Button_WasReleased();

        if (MorseApp_DeadlineReached(current_time_ms, state_deadline_ms)) {
            if (transmit_index < sequence_length) {
                Led_On(LED_GREEN);
                state_deadline_ms = current_time_ms +
                                    MorseApp_SymbolDuration(sequence[transmit_index]);
                state = MORSE_STATE_TRANSMIT_ON;
            } else {
                MorseApp_FinishTransmission(current_time_ms);
            }
        }
        break;

    default:
        MorseApp_FinishTransmission(current_time_ms);
        break;
    }
}
