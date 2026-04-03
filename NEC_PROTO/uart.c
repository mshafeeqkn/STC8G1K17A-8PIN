#include <8051.h>
#include <stdio.h> // Required for printf

// --- STC8G Specific Registers ---
__sfr __at (0x8E) AUXR;
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

// --- Bit Definitions for SDCC ---
__sbit __at (0x8E) TR1; // Timer 1 Run control
__sbit __at (0x99) TI;  // Transmit Interrupt Flag

/**
 * 1. The Bridge for Printf
 * SDCC's printf() calls putchar() internally.
 */
int putchar(int c) {
    while (!TI);    // Wait until the previous character is sent
    TI = 0;         // Clear the flag manually
    SBUF = (char)c; // Load the new character into the buffer
    return c;
}

/**
 * 2. Optimized Initialization
 * Reviewed version of the working 11.0592MHz / 9600 Baud setup.
 */
void Init_UART(void) {
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
}
