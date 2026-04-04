/**
 * @file    nec.c
 * @brief   NEC Infrared Protocol Decoder logic.
 * @details Measures the time duration between falling edges on the IR receiver pin
 * using an external interrupt and a timer-driven tick counter.
 */

#include <stdio.h>
#include <stdint.h>
#include "nec.h"
#include "pwm.h"

/* --- Macro Definitions --- */
#define IR_CAPTURE_LENGTH 34 // 1 Header pulse + 32 Data pulses + 1 Stop bit

/* --- Global and Static Variables --- */
volatile __bit         g_ir_data_ready = 0;
volatile __xdata uint16_t g_ir_pulse_durations[IR_CAPTURE_LENGTH] = {0}; // Buffer in external RAM
volatile uint8_t       g_ir_pulse_index = 0;
volatile unsigned int  g_current_pulse_duration;

/**
 * @brief   Configures the External Interrupt 0 for IR signal reception.
 */
void NEC_Decoder_Init(void) {
    // INT0 Setup (P3.2 configured as IR Receiver Input)
    P3M1 |= 0x04;  // Set Port 3 Mode 1 bit 2
    P3M0 &= ~0x04; // Clear Port 3 Mode 0 bit 2 (P3.2 is now High-Z Input)

    IT0 = 1;       // Configure INT0 for Falling-edge trigger
    EX0 = 1;       // Enable External Interrupt 0
    EA  = 1;       // Enable Global Interrupts
}

/**
 * @brief   External Interrupt 0 Service Routine.
 * @details Captures the elapsed time since the last falling edge to determine
 * the length of the IR pulses.
 */
void NEC_INT0_ISR(void) __interrupt (0) {
    // Snapshot the time elapsed since the last interrupt
    g_current_pulse_duration = g_system_ticks_100us;
    g_system_ticks_100us = 0; // Reset timebase for the next measurement

    // Store the duration and advance the buffer index
    g_ir_pulse_durations[g_ir_pulse_index++] = g_current_pulse_duration;

    // Check if the expected number of pulses for a full NEC frame has been collected
    if(g_ir_pulse_index == IR_CAPTURE_LENGTH) {
        EX0 = 0;             // Temporarily disable INT0 to prevent overwrite
        g_ir_pulse_index = 0; // Reset index for next capture
        g_ir_data_ready = 1; // Signal main loop that data is ready to be decoded
    }
}

/**
 * @brief   Translates the array of pulse durations into a 32-bit NEC code.
 * @return  32-bit decoded value (Address + Inverse Address + Command + Inverse Command).
 */
unsigned long NEC_DecodeCommand(void) {
    unsigned long decoded_code = 0;
    unsigned char i;

    // Start evaluation at index 2 (skipping the idle time and the initial AGC header pulse)
    for (i = 2; i < IR_CAPTURE_LENGTH; i++) {
        // Shift bits right because the NEC protocol transmits Least Significant Bit (LSB) first
        decoded_code >>= 1;

        // Evaluate the pulse duration length
        // A gap larger than 17 ticks (~1.7ms) indicates a Logic 1.
        if (g_ir_pulse_durations[i] > 17) {
            decoded_code |= 0x80000000; // Inject Logic 1 at the MSB (will shift down)
        }
        // If duration is less than threshold, it remains Logic 0 naturally due to the shift.
    }

    return decoded_code;
}
