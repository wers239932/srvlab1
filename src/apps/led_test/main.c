#include "apps/led_test_app.h"
#include "drivers/button.h"
#include "drivers/led.h"
#include "platform/system_clock.h"

int main(void)
{
    HAL_Init();

    if (SystemClock_Config() != HAL_OK) {
        NVIC_SystemReset();
    }

    Led_Init();
    Button_Init();
    LedTestApp_Init(HAL_GetTick());

    while (1) {
        const uint32_t now = HAL_GetTick();

        Button_Update(now);
        LedTestApp_Update(now);
    }
}
