#ifndef __PWM_H__
#define __PWM_H__

extern volatile uint8_t  duty_cycle_index;

void Timer0_Setup(void);
void Timer0_Isr(void) __interrupt (1);

#endif
