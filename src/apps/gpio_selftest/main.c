#include "drivers/gpio.h"
#include "drivers/button.h"
#include "drivers/led.h"
#include "apps/led_test_app.h"
#include "platform/system_clock.h"
#include <stddef.h>

/* JTAG-readable results. Only the three LED pins are changed. */
volatile uint32_t gpio_test_checks;
volatile uint32_t gpio_test_failures;
volatile uint32_t gpio_test_done;

static void Check(bool success)
{
    ++gpio_test_checks;
    if (!success) {
        ++gpio_test_failures;
    }
}

static void TestConfig(GpioConfig config, uint16_t values)
{
    GpioConfig actual;
    Check(Gpio_Configure(GPIOD, 0xE000U, &config, values));
    for (uint8_t pin = 13U; pin < 16U; ++pin) {
        Check(Gpio_GetConfig(GPIOD, pin, &actual));
        Check(actual.mode == config.mode && actual.output_type == config.output_type &&
              actual.pull == config.pull && actual.speed == config.speed &&
              actual.alternate == config.alternate);
    }
}

static void RunTests(void)
{
    Check(Gpio_EnableClock(GPIOD));
    Check(!Gpio_EnableClock(NULL));
    const uint32_t mode = GPIOD->MODER & 0x03FFFFFFU;
    const uint32_t type = GPIOD->OTYPER & 0x1FFFU;
    const uint32_t pull = GPIOD->PUPDR & 0x03FFFFFFU;
    const uint32_t speed = GPIOD->OSPEEDR & 0x03FFFFFFU;
    const uint32_t af0 = GPIOD->AFR[0];
    const uint32_t af1 = GPIOD->AFR[1] & 0x000FFFFFU;
    const uint32_t odr = GPIOD->ODR & 0x1FFFU;
    GpioConfig config = {GPIO_MODE_OUT, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_DRIVER_SPEED_LOW, 0U};
    TestConfig(config, 0U);
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0U);
    Gpio_Write(GPIOD, 0xE000U, 0xA000U);
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0xA000U);
    Check(Gpio_Read(GPIOD, 0xE000U) == 0xA000U);
    Gpio_WritePin(GPIOD, 13U, false);
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0x8000U);
    Gpio_WritePin(GPIOD, 14U, true);
    Check(Gpio_ReadPin(GPIOD, 14U));
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0xC000U);
    TestConfig(config, 0xC000U);
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0xC000U);
    config.mode = GPIO_MODE_IN;
    config.pull = GPIO_PULL_DOWN;
    TestConfig(config, 0U);
    config.mode = GPIO_MODE_OUT;
    config.output_type = GPIO_OPEN_DRAIN;
    config.pull = GPIO_PULL_UP;
    config.speed = GPIO_DRIVER_SPEED_VERY_HIGH;
    TestConfig(config, 0U);
    Check(Gpio_ReadOutput(GPIOD, 0xE000U) == 0U);
    config.mode = GPIO_MODE_ANALOG_IN;
    config.pull = GPIO_PULL_NONE;
    TestConfig(config, 0U);
    /* Check AFR read-back while input, without connecting a peripheral. */
    config.mode = GPIO_MODE_IN;
    config.alternate = 7U;
    TestConfig(config, 0U);
    config.mode = GPIO_MODE_OUT;
    config.output_type = GPIO_PUSH_PULL;
    config.speed = GPIO_DRIVER_SPEED_LOW;
    config.alternate = 0U;
    TestConfig(config, 0U);
    const uint32_t before = GPIOD->MODER;
    Check(!Gpio_Configure(GPIOD, 0U, &config, 0U));
    config.pull = (GpioPull)3;
    Check(!Gpio_Configure(GPIOD, 0xE000U, &config, 0U));
    Check(GPIOD->MODER == before);
    Check(!Gpio_GetConfig(GPIOD, 16U, &config));
    Check((GPIOD->MODER & 0x03FFFFFFU) == mode);
    Check((GPIOD->OTYPER & 0x1FFFU) == type);
    Check((GPIOD->PUPDR & 0x03FFFFFFU) == pull);
    Check((GPIOD->OSPEEDR & 0x03FFFFFFU) == speed);
    Check(GPIOD->AFR[0] == af0);
    Check((GPIOD->AFR[1] & 0x000FFFFFU) == af1);
    Check((GPIOD->ODR & 0x1FFFU) == odr);
    gpio_test_done = 1U;
}

int main(void)
{
    HAL_Init();
    if (SystemClock_Config() != HAL_OK) {
        NVIC_SystemReset();
    }
    RunTests();
    Led_Init();
    Button_Init();
    LedTestApp_Init(HAL_GetTick());
    while (1) {
        const uint32_t now = HAL_GetTick();
        Button_Update(now);
        LedTestApp_Update(now);
    }
}
