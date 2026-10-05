#include "drivers/gpio.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

GPIO_TypeDef test_ports[11];
RCC_TypeDef test_rcc;

int main(void)
{
    unsigned int cases = 0U;
    for (unsigned int port = 0; port < 11U; ++port) {
        assert(Gpio_EnableClock(&test_ports[port]));
        assert((RCC->AHB1ENR & (1U << port)) != 0U);
    }
    assert(!Gpio_EnableClock(NULL));
    for (uint8_t pin = 0; pin < 16U; ++pin) {
        for (unsigned int mode = 0; mode < 4U; ++mode) {
            for (unsigned int pull = 0; pull < 3U; ++pull) {
                for (unsigned int type = 0; type < 2U; ++type) {
                    memset(GPIOD, 0xA5, sizeof(*GPIOD));
                    GPIOD->LCKR = 0U;
                    const GPIO_TypeDef before = *GPIOD;
                    const uint16_t mask = GPIO_DRIVER_PIN(pin);
                    const uint32_t mask2 = 3U << (2U * pin);
                    const uint32_t mask4 = 15U << (4U * (pin % 8U));
                    GpioConfig expected = {(GpioMode)mode, (GpioOutputType)type,
                        (GpioPull)pull, (GpioSpeed)(pin % 4U), pin};
                    GpioConfig actual;
                    assert(Gpio_Configure(GPIOD, mask, &expected, mask));
                    assert(Gpio_GetConfig(GPIOD, pin, &actual));
                    assert(actual.mode == expected.mode && actual.pull == expected.pull &&
                           actual.output_type == expected.output_type &&
                           actual.speed == expected.speed && actual.alternate == expected.alternate);
                    assert((GPIOD->MODER & ~mask2) == (before.MODER & ~mask2));
                    assert((GPIOD->PUPDR & ~mask2) == (before.PUPDR & ~mask2));
                    assert((GPIOD->OSPEEDR & ~mask2) == (before.OSPEEDR & ~mask2));
                    assert((GPIOD->OTYPER & ~(uint32_t)mask) == (before.OTYPER & ~(uint32_t)mask));
                    assert((GPIOD->AFR[pin / 8U] & ~mask4) == (before.AFR[pin / 8U] & ~mask4));
                    assert(GPIOD->AFR[1U - pin / 8U] == before.AFR[1U - pin / 8U]);
                    ++cases;
                }
            }
        }
    }
    /* BSRR is captured, not emulated: verify disjoint set/reset masks. */
    for (uint32_t value = 0; value <= 0xFFFFU; ++value) {
        Gpio_Write(GPIOD, 0xA55AU, (uint16_t)value);
        const uint32_t set = GPIOD->BSRR & 0xFFFFU;
        const uint32_t reset = GPIOD->BSRR >> 16U;
        assert(set == (value & 0xA55AU));
        assert(reset == ((~value) & 0xA55AU));
        assert((set & reset) == 0U);
    }
    GPIOD->IDR = 0x8421U;
    assert(Gpio_Read(GPIOD, 0xF000U) == 0x8000U);
    assert(Gpio_ReadPin(GPIOD, 15U));
    assert(!Gpio_ReadPin(GPIOD, 14U));
    assert(!Gpio_ReadPin(GPIOD, 16U));
    GpioConfig config = {GPIO_MODE_OUT, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_DRIVER_SPEED_LOW, 0U};
    GPIOD->LCKR = 0x10001U;
    GPIO_TypeDef before = *GPIOD;
    assert(!Gpio_Configure(GPIOD, 3U, &config, 0U));
    assert(memcmp(&before, GPIOD, sizeof(before)) == 0);
    assert(!Gpio_Configure(NULL, 1U, &config, 0U));
    assert(!Gpio_Configure(GPIOD, 0U, &config, 0U));
    assert(!Gpio_GetConfig(GPIOD, 16U, &config));
    assert(!Gpio_GetConfig(GPIOD, 0U, NULL));
    printf("PASS: %u configuration cases, 65536 BSRR values, input and lock checks\n", cases);
    return 0;
}
