#include <stdio.h>
#include <stdint.h>
#include "nec.h"
#include "pwm.h"

volatile __bit         ir_ready = 0;
volatile __xdata uint16_t data[34] = {0};
volatile uint8_t index = 0;
volatile uint8_t num_capture = 34;
volatile unsigned int  ir_duration;

void Init_NEC_Decoder(void) {
    // INT0 Setup (P3.2 - IR Input)
    P3M1 |= 0x04; P3M0 &= ~0x04; // P3.2 High-Z Input
    IT0 = 1;       // Falling edge trigger
    EX0 = 1;       // Enable INT0
    EA  = 1;       // Global Enable
}

void INT0_ISR(void) __interrupt (0) {
    ir_duration = sys_ticks;
    sys_ticks = 0;
    data[index++] = ir_duration;
    if(index == num_capture) {
        EX0 = 0;
        index = 0;
        ir_ready = 1;
    }
}

unsigned long decode_nec(void) {
    unsigned long code = 0;
    unsigned char i;

    // We start at index 2 (skipping idle and header)
    for (i = 2; i < num_capture; i++) {
        // Shift existing bits to make room for the new one
        // We shift RIGHT because NEC is typically LSB-first
        code >>= 1;
        if (data[i] > 17) {
            // It's a Logic 1 (~2067 ticks)
            code |= 0x80000000;
        }
        // If it's < 1500, it stays 0.
    }
    return code;
}
