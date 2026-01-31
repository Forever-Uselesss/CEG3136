#ifndef CALC_H_
#define CALC_H_

#include <stdint.h>

void Init_Calc(void);
void Task_Calc(void);

uint32_t CalcFunc(uint32_t sel, uint32_t a, uint32_t b);
uint32_t CalcGCD (uint32_t a, uint32_t b);
uint32_t CalcFactorial (uint32_t a);
uint32_t fib(uint32_t a);
uint32_t CalcSort(uint32_t *a, uint32_t b);
uint32_t CalcAverage (uint32_t *a, uint32_t b);


#endif /* CALC_H_ */
