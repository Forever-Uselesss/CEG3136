/*
 * main.c
 *
 *  Created on: Sep 15, 2025
 *      Author: jthoz081
 */

#include "stm32l5xx.h"
#include "stm32l552xx.h"
#include "field_access.h"
#include "gpio.h"

#define RCC RCC_NS
#define LD2_OFF_MSK (GPIO_BSRR_BR7)
#define LD2_ON_MSK (GPIO_BSRR_BS7)
#define B1_IN_MSK (GPIO_IDR_ID13)

uint32_t iterationCount;

void delay(int count) {
	for (unsigned int n = 0; n < count; n++) {
	}
}

int main(void) {
	//enable peiph clocl
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

	//PB7 OUT PUT MODE
	MODIFY_FIELD(GPIOB -> MODER, GPIO_MODER_MODE7, ESF_GPIO_MODER_OUTPUT);

	//ENABLE PERI CLOCK
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

	//CONFIGUE PC13 INP MODE
	MODIFY_FIELD(GPIOC -> MODER, GPIO_MODER_MODE13, ESF_GPIO_MODER_INPUT);

	//TURN OFF LD2
	GPIOB->BSRR = LD2_OFF_MSK;

	iterationCount = 0;

	while (1) {
		delay(3E5);
		if (!(GPIOC->IDR & B1_IN_MSK)) {
			GPIOB->BSRR = LD2_ON_MSK;
		}
		delay(3e5);
		GPIOB->BSRR = LD2_OFF_MSK;
		iterationCount++;
	}
}

