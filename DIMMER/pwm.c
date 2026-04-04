/**
 * @file    pwm.c
 * @brief   Software PWM implementation for dimming the LED strip.
 * @details Utilizes Timer 0 to generate a 100Hz PWM signal while simultaneously
 * maintaining a 100us timebase for the NEC IR decoding logic.
 */

#include <8051.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "pwm.h"

/* --- Global and Static Variables --- */
volatile uint16_t g_system_ticks_100us = 0;   // Global timebase for IR (1 tick = 100us)
volatile uint8_t  g_current_brightness_level = 0; // Index for current duty cycle

static volatile uint8_t s_pwm_cycle_counter = 0;    // Counter tracking the 100Hz PWM cycle

// Array storing predefined duty cycle percentages (0% to 100%) in Code memory
__code const uint8_t g_brightness_levels[DIMMER_STEPS_COUNT] = {0, 1, 2, 5, 20, 50, 100};

/**
 * @brief   Initializes Timer 0 for PWM and system tick generation.
 */
void PWM_Timer0_Init(void) {
    // 1. GPIO Configuration (STC8G)
    // Configure P3.3 (LED Pin) to Push-Pull mode
    P3M0 |= 0x08;
    P3M1 &= ~0x08;

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
    if(g_brightness_levels[g_current_brightness_level] == 1) {
        // Edge Case: Ultra Dim Level
        // Turn LED on for a very short duration manually using NOPs, bypassing standard tick math
        if (s_pwm_cycle_counter == 0) {
            COB_LED_PIN = 1; // Assert Pin High
            for(uint8_t i = 0; i < 25; i++) {
                __asm__("nop"); // Micro-delay
            }
            COB_LED_PIN = 0; // De-assert Pin Low
        } else {
            COB_LED_PIN = 0; // Ensure pin remains off for rest of cycle
        }
    } else {
        // Standard Case: Toggle LED pin based on the configured duty cycle percentage
        if (s_pwm_cycle_counter < g_brightness_levels[g_current_brightness_level]) {
            COB_LED_PIN = 1; // Pin ON while within active duty cycle window
        } else {
            COB_LED_PIN = 0; // Pin OFF once duty cycle threshold is passed
        }
    }
}
