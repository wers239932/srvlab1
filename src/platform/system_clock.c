#include "platform/system_clock.h"

#if HSE_VALUE != 25000000U
#error "SDK-1.1M.427 requires HSE_VALUE=25000000"
#endif

enum {
    SDK11M_PLL_M = 25,
    SDK11M_PLL_N = 360,
    SDK11M_PLL_Q = 8
};

HAL_StatusTypeDef SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    RCC_ClkInitTypeDef clocks = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscillator.HSEState = RCC_HSE_ON;
    oscillator.PLL.PLLState = RCC_PLL_ON;
    oscillator.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscillator.PLL.PLLM = SDK11M_PLL_M;
    oscillator.PLL.PLLN = SDK11M_PLL_N;
    oscillator.PLL.PLLP = RCC_PLLP_DIV2;
    oscillator.PLL.PLLQ = SDK11M_PLL_Q;

    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        return HAL_ERROR;
    }

    clocks.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clocks.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clocks.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clocks.APB1CLKDivider = RCC_HCLK_DIV4;
    clocks.APB2CLKDivider = RCC_HCLK_DIV2;

    return HAL_RCC_ClockConfig(&clocks, FLASH_LATENCY_5);
}
