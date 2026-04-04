#include <8051.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "pwm.h"

volatile uint16_t sys_ticks = 0;   // Global timebase for IR (1 tick = 100us)
static volatile uint8_t pwm_count = 0;    // Counter for 100Hz PWM cycle
volatile uint8_t  duty_cycle_index = 0;

__code const uint8_t duty_cycles[DS_COUNT] = {0, 1, 2, 5, 20, 50, 100};

void Timer0_Setup(void) {
// 1. GPIO Configuration (STC8G)
    P3M0 |= 0x08; P3M1 &= ~0x08; // P3.3 (LED) to Push-Pull
    
    // 2. Timer 0 Setup: 100us Heartbeat
    AUXR |= 0x80;  // Timer 0 in 1T (fast) mode
    TMOD &= 0xF0;  // Timer 0 in 16-bit non-auto-reload mode
    TH0 = 0xFB;    // High byte of 64430
    TL0 = 0xB6;    // Low byte of 64430
    
    // 3. Interrupt Enable
    ET0 = 1;       // Enable Timer 0 interrupt
    EA  = 1;       // Enable global interrupts
    TR0 = 1;       // Start Timer 0
}


// Timer 0 ISR: Fires exactly every 100us
void Timer0_Isr(void) __interrupt (1) {
    // 1. Immediate Reload (Critical for timing accuracy)
    TH0 = 0xFB; 
    TL0 = 0xB6;
    
    // 2. IR Timebase Task
    sys_ticks++; // Used by INT0 to calculate pulse widths

    // 3. Software PWM Task (100Hz base frequency)
    pwm_count++;
    if (pwm_count >= 100) {
        pwm_count = 0;
    }
   
    if(duty_cycles[duty_cycle_index] == 1) {   // Ultra Dim - Turn LED for very short time
        if (pwm_count == 0) {
            LED = 1; // Pin ON
            for(uint8_t i = 0; i < 25; i++) {
                __asm__("nop");
            }
            LED = 0; // Pin OFF
        } else {
            LED = 0; // Pin OFF
        }
    } else {
        // Toggle LED based on the duty cycle
        if (pwm_count < duty_cycles[duty_cycle_index]) {
            LED = 1; // Pin ON
        } else {
            LED = 0; // Pin OFF
        }
    }
}
