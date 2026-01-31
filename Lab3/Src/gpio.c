/*
 * gpio.c
 *
 *  Created on: Sep 22, 2025
 *      Author: jthoz081
 */

// General-purpose input/output driver
#include "gpio.h"
#include "i2c.h"
#include "stm32l5xx.h"
#include <stddef.h>
// --------------------------------------------------------
// Initialization
// --------------------------------------------------------

const Pin_t BUZZER = {GPIOA, 0};

// Enable the GPIO port peripheral clock for the specified pin
void GPIO_Enable(Pin_t pin) { GPIO_PortEnable(pin.port); }

void GPIO_PortEnable(GPIO_TypeDef *port) {
  if (port == GPIOX) {
    I2C_Enable(LeafyI2C);
  } else
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN << GPIO_PORT_NUM(port);
}
// Set the operating mode of a GPIO pin:
// Input (IN), Output (OUT), Alternate Function (AF), or Analog (ANA)
void GPIO_Mode(Pin_t pin, PinMode_t mode) { pin.port->MODER = ((pin.port->MODER) & ~(0b11 << (pin.bit * 2))) | (mode << (pin.bit * 2)); }

void GPIO_Config(Pin_t pin, PinType_t ot, PinSpeed_t osp, PinPUPD_t pupd) {
  pin.port->OTYPER = ((pin.port->OTYPER) & ~(0b11 << (pin.bit))) | (ot << (pin.bit));

  pin.port->OSPEEDR = ((pin.port->OSPEEDR) & ~(0b11 << (pin.bit * 2))) | (osp << (pin.bit * 2));

  pin.port->PUPDR = ((pin.port->PUPDR) & ~(0b11 << (pin.bit * 2))) | (pupd << (pin.bit * 2));
}
void GPIO_AltFunc(Pin_t pin, int af) {
  GPIO_TypeDef *GPIOx = pin.port;
  uint32_t idx = (pin.bit < 8U) ? 0U : 1U;          // AFRL for 0..7, AFRH for 8..15
  uint32_t shift = ((uint32_t)pin.bit & 0x7U) * 4U; // 4 bits per pin

  GPIOx->AFR[idx] &= ~(0xFUL << shift);              // clear the old AF
  GPIOx->AFR[idx] |= ((uint32_t)af & 0xFU) << shift; // WRITE THE NEW AF
}

// --------------------------------------------------------
// Pin observation and control
// --------------------------------------------------------
// Observe the value of an input pin
PinState_t GPIO_Input(const Pin_t pin) { return ((pin.port->IDR) >> pin.bit) & (0b1); }

uint16_t GPIO_PortInput(GPIO_TypeDef *port) { return port->IDR; }

void GPIO_Output(Pin_t pin, const PinState_t state) {

  if (pin.port == GPIOX) {
    if (state == HIGH) {
      pin.port->ODR |= 1 << pin.bit;
    } else {
      pin.port->ODR &= ~(1 << pin.bit);
    }
  } else {
    if (state == HIGH) {
      pin.port->BSRR = (0b1 << pin.bit);
    } else {
      pin.port->BSRR = (0b1 << (pin.bit + 16));
    }
  }
}

void GPIO_PortOutput(GPIO_TypeDef *port, uint16_t states) { port->ODR = states; }

void GPIO_Toggle(Pin_t pin) { pin.port->ODR ^= (0b1 << pin.bit); }

// --------------------------------------------------------
// I/O expander management
// --------------------------------------------------------
// Emulated GPIO registers for I/O expander
GPIO_TypeDef IOX_GPIO_Regs = {0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, {0, 0}, 0, 0, 0};
// Transmit/receive data buffers
static uint8_t IOX_txData = 0xFF;
static uint8_t IOX_rxData = 0xFF;
// I2C transfer structures bus addr data size stop busy next
static I2C_Xfer_t IOX_LEDs = {&LeafyI2C, 0x70, &IOX_txData, 1, 1, 0, NULL};
static I2C_Xfer_t IOX_PBs = {&LeafyI2C, 0x73, &IOX_rxData, 1, 1, 0, NULL};

void UpdateIOExpanders(void) {
  // Copy to/from data buffers, with polarity inversion
  IOX_txData = ~(GPIOX->ODR & 0xFF); // LEDs in bits 7:0
  GPIOX->IDR = (~IOX_rxData) << 8;   // PBs in bits 15:8
  // Keep requesting transfers to/from I/O expanders
  if (!IOX_LEDs.busy)
    I2C_Request(&IOX_LEDs);
  if (!IOX_PBs.busy)
    I2C_Request(&IOX_PBs);
}

// --------------------------------------------------------
// Interrupt handling
// --------------------------------------------------------
// Array of callback function pointers
// Bits 0 to 15 (each can select one port GPIOA to GPIOH)
// Rising and falling edge triggers for each
static void (*callbacks[16][2])(void);
// Register a function to be called when an interrupt occurs,
// enable interrupt generation, and enable interrupt vector
void GPIO_Callback(Pin_t pin, void (*func)(void), PinEdge_t edge) {
  callbacks[pin.bit][edge] = func;
  // Enable interrupt generation
  if (edge == RISE)
    EXTI->RTSR1 |= 1 << pin.bit;
  else
    // FALL
    EXTI->FTSR1 |= 1 << pin.bit;
  EXTI->EXTICR[pin.bit / 4] |= GPIO_PORT_NUM(pin.port) << (8 * (pin.bit % 4));
  EXTI->IMR1 |= 1 << pin.bit;
  // Enable interrupt vector
  NVIC->IPR[EXTI0_IRQn + pin.bit] = 0;
  __COMPILER_BARRIER();
  NVIC->ISER[(EXTI0_IRQn + pin.bit) / 32] = 1 << ((EXTI0_IRQn + pin.bit) % 32);
  __COMPILER_BARRIER();
}
// Interrupt handler for all GPIO pins
void GPIO_IRQHandler(int i) {
  // Clear pending IRQ
  NVIC->ICPR[(EXTI0_IRQn + i) / 32] = 1 << ((EXTI0_IRQn + i) % 32);
  // Detect rising edge
  if (EXTI->RPR1 & (1 << i)) {
    EXTI->RPR1 = (1 << i); // Service interrupt
    callbacks[i][RISE]();
    // Invoke callback function
  }
  // Detect falling edge
  if (EXTI->FPR1 & (1 << i)) {
    EXTI->FPR1 = (1 << i); // Service interrupt
    callbacks[i][FALL]();
    // Invoke callback function
  }
}
// Dispatch all GPIO IRQs to common handler function
void EXTI0_IRQHandler() { GPIO_IRQHandler(0); }
void EXTI1_IRQHandler() { GPIO_IRQHandler(1); }
void EXTI2_IRQHandler() { GPIO_IRQHandler(2); }
void EXTI3_IRQHandler() { GPIO_IRQHandler(3); }
void EXTI4_IRQHandler() { GPIO_IRQHandler(4); }
void EXTI5_IRQHandler() { GPIO_IRQHandler(5); }
void EXTI6_IRQHandler() { GPIO_IRQHandler(6); }
void EXTI7_IRQHandler() { GPIO_IRQHandler(7); }
void EXTI8_IRQHandler() { GPIO_IRQHandler(8); }
void EXTI9_IRQHandler() { GPIO_IRQHandler(9); }
void EXTI10_IRQHandler() { GPIO_IRQHandler(10); }
void EXTI11_IRQHandler() { GPIO_IRQHandler(11); }
void EXTI12_IRQHandler() { GPIO_IRQHandler(12); }
void EXTI13_IRQHandler() { GPIO_IRQHandler(13); }
void EXTI14_IRQHandler() { GPIO_IRQHandler(14); }
void EXTI15_IRQHandler() { GPIO_IRQHandler(15); }
