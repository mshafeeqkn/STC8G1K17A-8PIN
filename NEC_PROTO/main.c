#include <8051.h>
#include <stdio.h>
#include <stdint.h>

// --- System Registers ---
__sfr __at (0x8E) AUXR;
__sfr __at (0xA2) P_SW1;
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

// --- IR Variable Definitions ---
volatile unsigned int  ir_duration;
volatile unsigned long ir_code;
volatile unsigned char ir_bits;
volatile __bit         ir_ready = 0;

// --- UART & Timer Control Bits ---
__sbit __at (0x8E) TR1; // Timer 1 Run
__sbit __at (0x99) TI;  // UART Transmit Flag

/**
 * Printf Bridge (9600 Baud Logic)
 */

void Init_System(void) {
    // 1. UART1: 11.0592MHz @ 9600 Baud (Timer 1, 1T Mode)
    // GPIO: P3.1 (TX) to Push-Pull, P3.0 (RX) to High-Impedance
    P3M1 &= ~0x02; P3M0 |= 0x02;
    P3M1 |= 0x01;  P3M0 &= ~0x01;

    // UART1: 8-bit variable baud (Mode 1)
    SCON = 0x50;

    // Timer 1: Set to 1T mode (Fast) and use for UART1
    AUXR |= 0x40;
    AUXR &= ~0x01;

    // Timer 1: Mode 2 (8-bit auto-reload)
    TMOD &= 0x0F;
    TMOD |= 0x20;

    // Baud Rate: 9600 @ 11.0592 MHz
    // 256 - (11059200 / 32 / 9600) = 220 (0xDC)
    TH1 = 0xDC;
    TL1 = 0xDC;

    TR1 = 1; // Start Timer 1
    TI = 1;  // CRITICAL: Set TI to 1 so the first printf doesn't hang!


    // 2. IR Setup: Timer 0 (12T mode)
    // 1 tick = 12 / 11059200 = 1.085us
    AUXR &= ~0x80; // Timer 0 in 12T mode
    TMOD &= 0xF0;  // 16-bit Timer 0
    TH0 = 0; TL0 = 0;
    TR0 = 0;

    // 3. INT0 Setup (P3.2 - IR Input)
    P3M1 |= 0x04; P3M0 &= ~0x04; // P3.2 High-Z Input
    IT0 = 1;       // Falling edge trigger
    EX0 = 1;       // Enable INT0
    EA  = 1;       // Global Enable
}

void delay_ms(unsigned int ms) {
    unsigned int i;
    while(ms--) {
        for(i = 0; i < 1200; i++) { __asm__("nop"); }
    }
}

/**
 * INT0 ISR: Measures falling-edge to falling-edge distance.
 * Header (9ms low + 4.5ms high) = 13.5ms (~12,440 ticks)
 */
volatile __xdata uint16_t data[34] = {0};
volatile uint8_t index = 0;
volatile uint8_t num_capture = 34;

void INT0_ISR(void) __interrupt (0) {
    TR0 = 0; // Stop timer before reading
    ir_duration = (TH0 << 8) | TL0;
    TH0 = 0; TL0 = 0;
    TR0 = 1; // Restart timer for next gap
    data[index++] = ir_duration;
    if(index == num_capture) {
        EX0 = 0;
        index = 0;
        TR0 = 0; // Stop timer until next burst
        TH0 = 0; TL0 = 0;
        ir_ready = 1;
    }
}

/**
 * Decodes the 34-element gap array into a 32-bit HEX code.
 * data[0] = noise/idle
 * data[1] = header (~12460)
 * data[2..33] = 32 bits of data
 */
unsigned long decode_nec(unsigned int *buffer) {
    unsigned long code = 0;
    unsigned char i;

    // We start at index 2 (skipping idle and header)
    for (i = 2; i < num_capture; i++) {
        // Shift existing bits to make room for the new one
        // We shift RIGHT because NEC is typically LSB-first
        code >>= 1;
        if (buffer[i] > 1500) {
            // It's a Logic 1 (~2067 ticks)
            code |= 0x80000000;
        }
        // If it's < 1500, it stays 0.
    }
    return code;
}

void main(void) {
    Init_System();
    printf("NEC Decoder Online\r\n", ((TH0 << 8) | TL0));
    printf("Waiting for Atomberg Remote...\r\n\n");

    while (1) {
        if (ir_ready) {
            unsigned long code = decode_nec(data);
            printf("Code: %08lX\r\n", code);
            IE0 = 0;
            ir_ready = 0;
            EX0 = 1;
        }
        delay_ms(500);
    }
}
