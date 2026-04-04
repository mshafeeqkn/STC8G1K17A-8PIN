#ifndef __PWM_H__
#define __PWM_H__

#include "config.h"

extern volatile uint8_t  duty_cycle_index;
extern volatile uint16_t sys_ticks;

void Timer0_Setup(void);
void Timer0_Isr(void) __interrupt (1);

#endif
