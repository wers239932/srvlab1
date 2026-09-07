#include "drivers/led.h"

#include "stm32f4xx_hal.h"

/* Names GLC, YARC and YCRA follow the official SDK-1.1M schematic. */
#define LED_GPIO_PORT GPIOD
#define LED_GREEN_PIN GPIO_PIN_13
#define LED_YELLOW_PIN GPIO_PIN_14
#define LED_RED_PIN GPIO_PIN_15
#define LED_ALL_PINS (LED_GREEN_PIN | LED_YELLOW_PIN | LED_RED_PIN)

static bool green_is_on;

void Led_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* OFF is low for PD13 and equal low levels for the antiparallel VD7. */
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_ALL_PINS, GPIO_PIN_RESET);

    gpio.Pin = LED_ALL_PINS;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &gpio);

    green_is_on = false;
}

void Led_Set(LedId led, bool enabled)
{
    if (led != LED_GREEN) {
        return;
    }

    green_is_on = enabled;
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GREEN_PIN,
                      enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
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
    /* First make both terminals equal to avoid the unused fourth state. */
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_YELLOW_PIN | LED_RED_PIN,
                      GPIO_PIN_RESET);

    if (state == LED_BICOLOR_RED) {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_SET);
    } else if (state == LED_BICOLOR_YELLOW) {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_YELLOW_PIN, GPIO_PIN_SET);
    }
}
