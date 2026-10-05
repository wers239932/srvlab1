#include "drivers/gpio.h"

#include <stddef.h>

static int Gpio_PortIndex(GPIO_TypeDef *port)
{
    GPIO_TypeDef *const ports[] = {
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF,
        GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK
    };
    for (unsigned int i = 0; i < sizeof(ports) / sizeof(ports[0]); ++i) {
        if (ports[i] == port) {
            return (int)i;
        }
    }
    return -1;
}

bool Gpio_EnableClock(GPIO_TypeDef *port)
{
    const int index = Gpio_PortIndex(port);
    if (index < 0) {
        return false;
    }
    RCC->AHB1ENR |= 1U << (unsigned int)index;
    /* Read-back supplies the peripheral-clock startup delay. */
    (void)RCC->AHB1ENR;
    return true;
}

void Gpio_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t values)
{
    const uint32_t set = (uint32_t)(values & pins);
    const uint32_t reset = (uint32_t)(pins & (uint16_t)~values);
    /* One bus write changes all selected pins; other pins are untouched. */
    port->BSRR = set | (reset << 16U);
}

void Gpio_WritePin(GPIO_TypeDef *port, uint8_t pin, bool high)
{
    if (pin < 16U) {
        const uint16_t mask = GPIO_DRIVER_PIN(pin);
        Gpio_Write(port, mask, high ? mask : 0U);
    }
}

uint16_t Gpio_Read(GPIO_TypeDef *port, uint16_t pins)
{
    return (uint16_t)port->IDR & pins;
}

bool Gpio_ReadPin(GPIO_TypeDef *port, uint8_t pin)
{
    return pin < 16U && Gpio_Read(port, GPIO_DRIVER_PIN(pin)) != 0U;
}

uint16_t Gpio_ReadOutput(GPIO_TypeDef *port, uint16_t pins)
{
    return (uint16_t)port->ODR & pins;
}

bool Gpio_GetConfig(GPIO_TypeDef *port, uint8_t pin, GpioConfig *config)
{
    if (Gpio_PortIndex(port) < 0 || pin >= 16U || config == NULL) {
        return false;
    }
    const uint32_t shift = 2U * pin;
    config->mode = (GpioMode)((port->MODER >> shift) & 3U);
    config->output_type = (GpioOutputType)((port->OTYPER >> pin) & 1U);
    config->pull = (GpioPull)((port->PUPDR >> shift) & 3U);
    config->speed = (GpioSpeed)((port->OSPEEDR >> shift) & 3U);
    config->alternate = (uint8_t)((port->AFR[pin / 8U] >> (4U * (pin % 8U))) & 15U);
    return true;
}

bool Gpio_Configure(GPIO_TypeDef *port, uint16_t pins,
                    const GpioConfig *config, uint16_t output_values)
{
    if (Gpio_PortIndex(port) < 0 || pins == 0U || config == NULL ||
        (unsigned int)config->mode > 3U ||
        (unsigned int)config->output_type > 1U ||
        (unsigned int)config->pull > 2U ||
        (unsigned int)config->speed > 3U || config->alternate > 15U) {
        return false;
    }
    /* Reject the whole operation before changing any locked pin. */
    if ((port->LCKR & (1U << 16U)) != 0U && (port->LCKR & pins) != 0U) {
        return false;
    }

    uint32_t mask2 = 0U;
    uint32_t mode = 0U;
    uint32_t type = 0U;
    uint32_t pull = 0U;
    uint32_t speed = 0U;
    uint32_t disconnect = 0U;
    uint32_t af_mask[2] = {0U, 0U};
    uint32_t af_value[2] = {0U, 0U};
    const uint32_t old_mode = port->MODER;
    const uint32_t old_type = port->OTYPER;

    for (uint32_t pin = 0U; pin < 16U; ++pin) {
        if ((pins & (1U << pin)) == 0U) {
            continue;
        }
        const uint32_t shift = 2U * pin;
        const uint32_t previous = (old_mode >> shift) & 3U;
        mask2 |= 3U << shift;
        mode |= (uint32_t)config->mode << shift;
        type |= (uint32_t)config->output_type << pin;
        pull |= (uint32_t)config->pull << shift;
        speed |= (uint32_t)config->speed << shift;
        af_mask[pin / 8U] |= 15U << (4U * (pin % 8U));
        af_value[pin / 8U] |= (uint32_t)config->alternate << (4U * (pin % 8U));

        /* Keep an existing compatible output connected. Disconnect an AF
         * source or an output whose mode/type changes before editing controls.
         * Input means high impedance, never an unintended actively driven bit. */
        if (previous == GPIO_MODE_AF ||
            (previous == GPIO_MODE_OUT &&
             (config->mode != GPIO_MODE_OUT ||
              ((old_type >> pin) & 1U) != (uint32_t)config->output_type))) {
            disconnect |= 3U << shift;
        }
    }

    if (disconnect != 0U) {
        port->MODER = old_mode & ~disconnect;
    }
    if (config->mode == GPIO_MODE_OUT) {
        Gpio_Write(port, pins, output_values);
    }
    port->OTYPER = (port->OTYPER & ~(uint32_t)pins) | type;
    port->OSPEEDR = (port->OSPEEDR & ~mask2) | speed;
    port->PUPDR = (port->PUPDR & ~mask2) | pull;
    for (unsigned int i = 0U; i < 2U; ++i) {
        if (af_mask[i] != 0U) {
            port->AFR[i] = (port->AFR[i] & ~af_mask[i]) | af_value[i];
        }
    }
    /* Full field replacement permits repeated initialization to any mode. */
    port->MODER = (port->MODER & ~mask2) | mode;
    return true;
}
