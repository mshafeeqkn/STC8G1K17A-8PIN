/**
 * @file    pwm.c
 * @brief   Software PWM implementation for dimming the LED strip.
 * @details Utilizes Timer 0 to generate a 100Hz PWM signal while simultaneously
 * maintaining a 100us timebase for the NEC IR decoding logic.
 */

#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "pwm.h"
#include "nec.h"

/* --- Global and Static Variables --- */
volatile uint16_t g_system_ticks_100us = 0;   // Global timebase for IR (1 tick = 100us)
static volatile uint8_t s_pwm_cycle_counter = 0;    // Counter tracking the 100Hz PWM cycle

/**
 * @brief   Initializes Timer 0 for PWM and system tick generation.
 */
void PWM_Timer0_Init(void) {
    // 1. GPIO Configuration (STC8G)
    // Configure P3.2 (LED Pin) to Push-Pull mode
    P3M0 |= 0x04;
    P3M1 &= ~0x04;

    // 2. Timer 0 Setup: Target 100us Heartbeat
    AUXR |= 0x80;  // Configure Timer 0 in 1T (fast) mode (No prescaler)
    TMOD &= 0xF0;  // Configure Timer 0 in 16-bit non-auto-reload mode
    TH0 = 0xFB;    // Preload High byte (64430 decimal)
    TL0 = 0xB6;    // Preload Low byte (64430 decimal)

    // 3. Interrupt Enablement
    ET0 = 1;       // Enable Timer 0 specific interrupt
    EA  = 1;       // Enable global interrupts
    TR0 = 1;       // Start Timer 0 counting
}

/**
 * @brief   Timer 0 Interrupt Service Routine.
 * @details Fires exactly every 100us. Handles reloading the timer, incrementing
 * the IR timebase, and manually toggling the LED pin based on duty cycle.
 */
void PWM_Timer0_ISR(void) __interrupt (1) {
    // 1. Immediate Reload (Critical for maintaining timing accuracy without auto-reload)
    TH0 = 0xFB;
    TL0 = 0xB6;

    // 2. IR Timebase Task
    g_system_ticks_100us++; // Increment tick for INT0 to calculate pulse widths

    // 3. Software PWM Task (Establishes 100Hz base frequency)
    s_pwm_cycle_counter++;
    if (s_pwm_cycle_counter >= 100) {
        s_pwm_cycle_counter = 0; // Reset cycle every 100 ticks (10ms total period)
    }

    // 4. Duty Cycle Application
    if(g_current_brightness <= 0) {
        // Special Case: For very low brightness levels (negative values and 0), 
        // we treat them as ON pulses at the start of the cycle, with pulse length
        // proportional to (128 + g_current_brightness). This makes more negative
        // values have shorter ON pulses (longer OFF time), while 0 has a longer
        // pulse than negative values, providing some brightness without being fully off.
        if (s_pwm_cycle_counter == 0) {
            COB_LED_PIN = 0; // Assert Pin High
            uint8_t pulse_length = 128 + g_current_brightness;
            for(uint8_t i = 0; i < pulse_length; i++) {
                __asm__("nop"); // Micro-delay
            }
            COB_LED_PIN = 1; // De-assert Pin Low
        } else {
            COB_LED_PIN = 1; // Ensure pin remains off for rest of cycle
        }
    } else if(g_current_brightness == INT8_MAX) {
        // Special Case: Turn off LED completely for "power off" command (represented by max int8 value)
        COB_LED_PIN = 1; // Ensure pin is off
    } else {
        // Standard Case: Toggle LED pin based on the configured duty cycle percentage
        if (s_pwm_cycle_counter < g_current_brightness) {
            COB_LED_PIN = 0; // Pin ON while within active duty cycle window
        } else {
            COB_LED_PIN = 1; // Pin OFF once duty cycle threshold is passed
        }
    }
}
