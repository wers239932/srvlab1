#include "apps/led_test_app.h"

#include <stdbool.h>

#include "drivers/button.h"
#include "drivers/led.h"

typedef enum {
    LED_TEST_GREEN = 0,
    LED_TEST_RED,
    LED_TEST_YELLOW,
    LED_TEST_OFF,
    LED_TEST_BUTTON,
    LED_TEST_PHASE_COUNT
} LedTestPhase;

static LedTestPhase phase;
static uint32_t phase_started_ms;

static uint32_t LedTestApp_PhaseDuration(LedTestPhase current_phase)
{
    return current_phase == LED_TEST_BUTTON
               ? LED_TEST_BUTTON_PHASE_MS
               : LED_TEST_STEP_MS;
}

static void LedTestApp_ApplyPhase(void)
{
    Led_Off(LED_GREEN);
    Led_SetBicolor(LED_BICOLOR_OFF);

    switch (phase) {
    case LED_TEST_GREEN:
        Led_On(LED_GREEN);
        break;
    case LED_TEST_RED:
        Led_SetBicolor(LED_BICOLOR_RED);
        break;
    case LED_TEST_YELLOW:
        Led_SetBicolor(LED_BICOLOR_YELLOW);
        break;
    case LED_TEST_BUTTON:
        Led_Set(LED_GREEN, Button_IsPressed());
        break;
    case LED_TEST_OFF:
    case LED_TEST_PHASE_COUNT:
    default:
        break;
    }
}

void LedTestApp_Init(uint32_t current_time_ms)
{
    phase = LED_TEST_GREEN;
    phase_started_ms = current_time_ms;
    LedTestApp_ApplyPhase();
}

void LedTestApp_Update(uint32_t current_time_ms)
{
    const bool was_pressed = Button_WasPressed();
    const bool was_released = Button_WasReleased();

    if (phase == LED_TEST_BUTTON) {
        if (was_pressed) {
            Led_On(LED_GREEN);
        }
        if (was_released) {
            Led_Off(LED_GREEN);
        }
    }

    if ((uint32_t)(current_time_ms - phase_started_ms) >=
        LedTestApp_PhaseDuration(phase)) {
        phase = (LedTestPhase)(((unsigned int)phase + 1U) %
                               (unsigned int)LED_TEST_PHASE_COUNT);
        phase_started_ms = current_time_ms;
        LedTestApp_ApplyPhase();
    }
}
