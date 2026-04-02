#include <8051.h>
#include <stdint.h>

// --- STC8G Specific Register Definitions ---
__sfr __at (0x8E) AUXR;
__sfr __at (0xB2) P3M0;
__sfr __at (0xB1) P3M1;

// Bit Definitions
__sbit __at (0xB3) LED;     // Pin 8 (P3.3)
__sbit __at (0xB2) BTN;     // Pin 7 (P3.2/INT0)

// --- Global Variables ---
volatile unsigned char pwm_tick = 0;
volatile unsigned char duty_cycle = 1;

void Init_System(void) {
    // 1. GPIO Configuration
    P3M0 |= 0x08; P3M1 &= ~0x08; // LED: Push-Pull
    P3M0 &= ~0x04; P3M1 |= 0x04; // BTN: High-Impedance Input

    // 2. Timer 0 for PWM
    AUXR |= 0x80;
    TMOD &= 0xF0;
    TL0 = 0xF6; TH0 = 0xFF;
    TR0 = 1; ET0 = 1;

    // 3. Classic INT0 Configuration
    IT0 = 1;   // 1 = Falling Edge trigger, 0 = Low Level
    EX0 = 1;   // Enable External Interrupt 0
    
    EA = 1;    // Global Enable
}

void main(void) {
    Init_System();
    while (1);
}

// Timer 0 ISR (PWM)
void Timer0_Isr(void) __interrupt (1) {
    pwm_tick++;
    if (pwm_tick >= 100) pwm_tick = 0;
    LED = (pwm_tick < duty_cycle) ? 1 : 0;
}

volatile uint8_t duty_cycles[] = {1, 7, 21, 40, 70, 100};
volatile uint8_t duty_idx = 0;

// External Interrupt 0 ISR (Falling Edge)
void External0_Isr(void) __interrupt (0) {
    unsigned int i;
    for(i = 0; i < 2000; i++); // Debounce

    if (BTN == 0) { // Verify falling edge
        duty_idx = (duty_idx + 1) % 6;
        duty_cycle = duty_cycles[duty_idx];
    }
}
