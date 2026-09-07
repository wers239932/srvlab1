#include "drivers/button.h"

#include "stm32f4xx_hal.h"

/* nBTN has an external 3 kohm pull-up and is grounded when pressed. */
#define BUTTON_GPIO_PORT GPIOC
#define BUTTON_GPIO_PIN GPIO_PIN_15

static bool stable_pressed;
static bool candidate_pressed;
static bool pressed_event;
static bool released_event;
static uint32_t candidate_since_ms;

static bool Button_ReadRaw(void)
{
    return HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN) == GPIO_PIN_RESET;
}

void Button_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio.Pin = BUTTON_GPIO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &gpio);

    stable_pressed = Button_ReadRaw();
    candidate_pressed = stable_pressed;
    candidate_since_ms = HAL_GetTick();
    pressed_event = false;
    released_event = false;
}

void Button_Update(uint32_t current_time_ms)
{
    const bool raw_pressed = Button_ReadRaw();

    if (raw_pressed != candidate_pressed) {
        candidate_pressed = raw_pressed;
        candidate_since_ms = current_time_ms;
        return;
    }

    if ((candidate_pressed != stable_pressed) &&
        ((uint32_t)(current_time_ms - candidate_since_ms) >= BUTTON_DEBOUNCE_MS)) {
        stable_pressed = candidate_pressed;

        if (stable_pressed) {
            pressed_event = true;
        } else {
            released_event = true;
        }
    }
}

bool Button_IsPressed(void)
{
    return stable_pressed;
}

bool Button_WasPressed(void)
{
    const bool occurred = pressed_event;
    pressed_event = false;
    return occurred;
}

bool Button_WasReleased(void)
{
    const bool occurred = released_event;
    released_event = false;
    return occurred;
}
