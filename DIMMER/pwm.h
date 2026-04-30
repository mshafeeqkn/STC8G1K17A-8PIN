/**
 * @file    pwm.h
 * @brief   Header for Software PWM generation.
 */

#ifndef __PWM_H__
#define __PWM_H__

#include "config.h"

// Expose the current brightness level index to main application
extern volatile uint8_t  g_current_brightness_level;
extern volatile uint8_t  g_prev_brightness_level;

// Expose the 100us system tick counter for use by the IR decoder timebase
extern volatile uint16_t g_system_ticks_100us;

void PWM_Timer0_Init(void);
void PWM_Timer0_ISR(void) __interrupt (1);

#endif // __PWM_H__
