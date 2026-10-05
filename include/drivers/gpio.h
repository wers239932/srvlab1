#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx.h"

#define GPIO_DRIVER_PIN(pin) ((uint16_t)(1U << (pin)))

typedef enum { GPIO_MODE_IN = 0, GPIO_MODE_OUT = 1,
               GPIO_MODE_AF = 2, GPIO_MODE_ANALOG_IN = 3 } GpioMode;
typedef enum { GPIO_PUSH_PULL = 0, GPIO_OPEN_DRAIN = 1 } GpioOutputType;
typedef enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 } GpioPull;
typedef enum { GPIO_DRIVER_SPEED_LOW = 0, GPIO_DRIVER_SPEED_MEDIUM = 1,
               GPIO_DRIVER_SPEED_HIGH = 2, GPIO_DRIVER_SPEED_VERY_HIGH = 3 } GpioSpeed;

typedef struct {
    GpioMode mode;
    GpioOutputType output_type;
    GpioPull pull;
    GpioSpeed speed;
    uint8_t alternate;
} GpioConfig;

/* Calls are serialized by the cooperative main loop; no interrupt support. */
bool Gpio_EnableClock(GPIO_TypeDef *port);
/* output_values supplies the initial ODR bits before enabling output mode. */
bool Gpio_Configure(GPIO_TypeDef *port, uint16_t pins,
                    const GpioConfig *config, uint16_t output_values);
bool Gpio_GetConfig(GPIO_TypeDef *port, uint8_t pin, GpioConfig *config);
uint16_t Gpio_Read(GPIO_TypeDef *port, uint16_t pins);
bool Gpio_ReadPin(GPIO_TypeDef *port, uint8_t pin);
uint16_t Gpio_ReadOutput(GPIO_TypeDef *port, uint16_t pins);
void Gpio_Write(GPIO_TypeDef *port, uint16_t pins, uint16_t values);
void Gpio_WritePin(GPIO_TypeDef *port, uint8_t pin, bool high);

#endif
