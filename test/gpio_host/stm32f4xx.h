#ifndef GPIO_HOST_STM32_H
#define GPIO_HOST_STM32_H
#include <stdint.h>
typedef struct {
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO_TypeDef;
typedef struct { volatile uint32_t AHB1ENR; } RCC_TypeDef;
extern GPIO_TypeDef test_ports[11];
extern RCC_TypeDef test_rcc;
#define GPIOA (&test_ports[0])
#define GPIOB (&test_ports[1])
#define GPIOC (&test_ports[2])
#define GPIOD (&test_ports[3])
#define GPIOE (&test_ports[4])
#define GPIOF (&test_ports[5])
#define GPIOG (&test_ports[6])
#define GPIOH (&test_ports[7])
#define GPIOI (&test_ports[8])
#define GPIOJ (&test_ports[9])
#define GPIOK (&test_ports[10])
#define RCC (&test_rcc)
#endif
