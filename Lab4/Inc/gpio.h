/*
 * gpio.h
 *
 *  Created on: Sep 22, 2025
 *      Author: jthoz081
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "stm32l5xx.h"
#include <stdint.h>

// This preprocessor macro converts a GPIO register base address
// (GPIOA, GPIOB, GPIOC, etc.) to a zero-based port index (0, 1, 2, etc.)
// The address pattern can be identified from the device header file
#define GPIO_PORT_NUM(addr) (((unsigned)(addr) & 0xFC00) / 0x400)

#define ENABLE_GPIO_CLOCK(PORT) (RCC->AHB1ENR |= RCC_AHB2ENR_GPIO##PORT##EN)

// #define

// Structure representing a single GPIO pin
typedef struct {
  GPIO_TypeDef *port; // GPIOA, GPIOB, etc.
  int bit;            // Bit index 0..15
} Pin_t;

extern const Pin_t BUZZER;

typedef enum { INPUT = 0b00, OUTPUT = 0b01, ALTFUNC = 0b10, ANALOG = 0b11 } PinMode_t;

typedef enum { LOW = 0, HIGH = 1 } PinState_t;

typedef enum { FALL = 0, RISE = 1 } PinEdge_t;

typedef enum { PP = 0, OD = 1 } PinType_t;

typedef enum { S0 = 0b00, S1 = 0b01, S2 = 0b10, S3 = 0b11 } PinSpeed_t;

typedef enum { NOPUPD = 0b00, PU = 0b01, PD = 0b10 } PinPUPD_t;

extern GPIO_TypeDef IOX_GPIO_Regs;
#define GPIOX (&IOX_GPIO_Regs)

static const Pin_t StartButton = {GPIOX, 11};  // SW4
static const Pin_t SelectButton = {GPIOX, 12}; // SW5

void GPIO_Enable(Pin_t pin);
void GPIO_Mode(Pin_t pin, PinMode_t mode);
void GPIO_PortEnable(GPIO_TypeDef *port);
PinState_t GPIO_Input(Pin_t pin);
uint16_t GPIO_PortInput(GPIO_TypeDef *port);
void GPIO_Output(Pin_t pin, PinState_t state);
void GPIO_PortOutput(GPIO_TypeDef *port, uint16_t states);
void GPIO_Toggle(Pin_t pin);
void GPIO_Callback(Pin_t pin, void (*func)(void), PinEdge_t edge);
void GPIO_Config(Pin_t pin, PinType_t ot, PinSpeed_t osp, PinPUPD_t pupd);
void GPIO_AltFunc(Pin_t pin, int af);

void UpdateIOExpanders(void);

#endif /* GPIO_H_ */
