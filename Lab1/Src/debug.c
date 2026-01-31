/*
 * debug.c
 *
 *  Created on: Sep 22, 2025
 *      Author: jthoz081
 */

#include "stm32l5xx.h"

int __io_putchar(int c) {
	ITM_SendChar(c);
	return c;
}
