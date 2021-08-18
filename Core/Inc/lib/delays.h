#ifndef DELAYS_H
#define DELAYS_H

#include "main.h"
#define CPU_CLOCK		48000000
#define K_Const			6144

#define TEST_PORT  GPIOC, GPIO_PIN_3

void delay_ms(uint32_t nTime);
void delay_us(uint32_t nTime);
void delay_test(void);

#endif  // DELAYS_H
