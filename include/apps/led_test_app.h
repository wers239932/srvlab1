#ifndef APPS_LED_TEST_APP_H
#define APPS_LED_TEST_APP_H

#include <stdint.h>

#define LED_TEST_STEP_MS 500U
#define LED_TEST_BUTTON_PHASE_MS 2000U

void LedTestApp_Init(uint32_t current_time_ms);
void LedTestApp_Update(uint32_t current_time_ms);

#endif
