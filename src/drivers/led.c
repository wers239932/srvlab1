#include "drivers/led.h"

#include "drivers/gpio.h"

/* Names GLC, YARC and YCRA follow the official SDK-1.1M schematic. */
#define LED_GPIO_PORT GPIOD
#define LED_GREEN_PIN GPIO_DRIVER_PIN(13)
#define LED_YELLOW_PIN GPIO_DRIVER_PIN(14)
#define LED_RED_PIN GPIO_DRIVER_PIN(15)
#define LED_ALL_PINS (LED_GREEN_PIN | LED_YELLOW_PIN | LED_RED_PIN)

static bool green_is_on;

void Led_Init(void)
{
    const GpioConfig gpio = {
        GPIO_MODE_OUT, GPIO_PUSH_PULL, GPIO_PULL_NONE, GPIO_DRIVER_SPEED_LOW, 0U
    };
    (void)Gpio_EnableClock(LED_GPIO_PORT);
    (void)Gpio_Configure(LED_GPIO_PORT, LED_ALL_PINS, &gpio, 0U);

    green_is_on = false;
}

void Led_Set(LedId led, bool enabled)
{
    if (led != LED_GREEN) {
        return;
    }

    green_is_on = enabled;
    Gpio_Write(LED_GPIO_PORT, LED_GREEN_PIN, enabled ? LED_GREEN_PIN : 0U);
}

void Led_On(LedId led)
{
    Led_Set(led, true);
}

void Led_Off(LedId led)
{
    Led_Set(led, false);
}

void Led_Toggle(LedId led)
{
    if (led == LED_GREEN) {
        Led_Set(led, !green_is_on);
    }
}

void Led_SetBicolor(LedBicolorState state)
{
    const uint16_t values = state == LED_BICOLOR_RED ? LED_RED_PIN :
                            state == LED_BICOLOR_YELLOW ? LED_YELLOW_PIN : 0U;
    Gpio_Write(LED_GPIO_PORT, LED_YELLOW_PIN | LED_RED_PIN, values);
}
