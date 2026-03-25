#include <stdint.h>

// ==========================================
// SFR and SBIT Definitions
// ==========================================

// TCON - Timer Control Register (Address 0x88, Bit-Addressable)
__sfr __at (0x88) TCON;
__sbit __at (0x8D) TF0;      // Timer 0 Overflow Flag
__sbit __at (0x8C) TR0;      // Timer 0 Run Control
__sbit __at (0x8B) TF1;      // Timer 1 Overflow Flag
__sbit __at (0x8E) TR1;      // Timer 1 Run Control
__sbit __at (0x88) IT0;      // INT0 Type (0: Both edges/Low, 1: Falling only)

// Timer Mode and Data Registers
__sfr __at (0x89) TMOD;      // Timer Mode
__sfr __at (0x8A) TL0;       // Timer 0 Low Byte
__sfr __at (0x8C) TH0;       // Timer 0 High Byte
__sfr __at (0x8B) TL1;       // Timer 1 Low Byte
__sfr __at (0x8D) TH1;       // Timer 1 High Byte (Byte Address 0x8D)

// Interrupt Enable
__sfr __at (0xA8) IE;
__sbit __at (0xAF) EA;       // Global Interrupt Enable
__sbit __at (0xA9) ET0;      // Timer 0 Interrupt Enable
__sbit __at (0xA8) EX0;      // External Interrupt 0 Enable

// Port 3 Configuration (STC8G Specific)
__sfr __at (0xB0) P3;
__sfr __at (0xB1) P3M1;      // Port 3 Mode 1
__sfr __at (0xB2) P3M0;      // Port 3 Mode 0
__sbit __at (0xB3) LED;      // P3.3 (Pin 8)
__sbit __at (0xB2) INT0_PIN; // P3.2 (Pin 5)

// ==========================================
// Global Variables
// ==========================================
volatile uint16_t pulse_ticks = 0;
volatile uint16_t pulse_overflows = 0;
volatile uint16_t overflow_counter = 0;
volatile uint8_t  data_ready = 0;

// ==========================================
// Interrupt Service Routines
// ==========================================

// Timer 0 ISR: Increments every 65536 machine cycles
void T0_ISR(void) __interrupt(1) {
    overflow_counter++;
}

// External Interrupt 0: Triggers on Edge Change
void INT0_ISR(void) __interrupt(0) {
    if (INT0_PIN) {
        // --- RISING EDGE ---
        // Start the stopwatch
        TH0 = 0;
        TL0 = 0;
        overflow_counter = 0;
        TF0 = 0;
        TR0 = 1; 
    } else {
        // --- FALLING EDGE ---
        // Stop the stopwatch and save results
        TR0 = 0;
        pulse_ticks = (TH0 << 8) | TL0;
        pulse_overflows = overflow_counter;
        data_ready = 1;
    }
}

// ==========================================
// Helper: Playback the measured delay
// ==========================================
void play_delay(uint16_t overflows, uint16_t ticks) {
    uint16_t i;
    
    // 1. Play back full 16-bit overflows
    for (i = 0; i < overflows; i++) {
        TH0 = 0;
        TL0 = 0;
        TF0 = 0;
        TR0 = 1;
        while (!TF0); 
        TR0 = 0;
    }
    
    // 2. Play back the remaining fractional ticks
    if (ticks > 0) {
        uint16_t reload = 65536 - ticks;
        TH0 = reload >> 8;
        TL0 = reload & 0xFF;
        TF0 = 0;
        TR0 = 1;
        while (!TF0);
        TR0 = 0;
    }
}

// ==========================================
// Main Function
// ==========================================
void main(void) {
    // 1. Configure P3.3 (LED) as Push-Pull Output
    P3M1 &= ~0x08; 
    P3M0 |=  0x08; 
    
    // 2. Configure Timer 0: 16-bit Mode, No Gate
    TMOD = 0x01; 
    
    // 3. Configure External Interrupt 0
    IT0 = 0; // Set to 0 for both rising/falling edge sensitivity
    EX0 = 1; // Enable INT0
    ET0 = 1; // Enable Timer 0 Interrupt (for overflow counting)
    EA  = 1; // Global Interrupt Enable

    LED = 0; // Ensure LED starts OFF

    while (1) {
        if (data_ready) {
            data_ready = 0;
            
            LED = 1; // Turn ON
            play_delay(pulse_overflows, pulse_ticks);
            LED = 0; // Turn OFF
        }
    }
}
