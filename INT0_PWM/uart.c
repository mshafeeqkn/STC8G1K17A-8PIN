#include <8051.h>
#include <stdio.h>

// --- STC8G Specific SFR Addresses ---
__sfr __at (0x8E) AUXR;
__sfr __at (0xD6) T2H;    // Corrected from 0x10
__sfr __at (0xD7) T2L;    // Corrected from 0x11

// --- UART Setup ---
void Init_UART(void) {
    SCON = 0x50;    // 8-bit variable baud rate, enable receiver
    AUXR |= 0x01;   // UART1 uses Timer 2 as baud rate generator
    AUXR |= 0x04;   // Timer 2 in 1T mode (Fast)
    
    // For 115200 bps @ 11.0592MHz:
    // Calculation: 65536 - (11059200 / 4 / 115200) = 65512 (0xFFE8)
    T2H = 0xFF;
    T2L = 0xE8;
    
    AUXR |= 0x10;   // Start Timer 2 (T2R bit)
}

// --- putchar implementation for printf ---
// SDCC uses this function to pipe printf data to UART
int putchar(int c) {
    if (c == '\n') {
        SBUF = '\r';
        while (!(SCON & 0x02)); // Wait for TI
        SCON &= ~0x02;          // Clear TI
    }
    SBUF = (char)c;             // Cast back to char for the buffer
    while (!(SCON & 0x02));     // Wait for TI
    SCON &= ~0x02;              // Clear TI
    
    return c;                   // Standard library expects the char returned
}
