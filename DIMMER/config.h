#ifndef __CONFIG_H__

__sbit __at(0xB3) LED;   // 0xB0 + 3 -> P3 + .3

// Core SFRs (8051 compatible)
// --- STC8G Specific Registers ---
__sfr __at (0x8E) AUXR;
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

// --- Bit Definitions for SDCC ---
__sbit __at (0x8E) TR1; // Timer 1 Run control
__sbit __at (0x99) TI;  // Transmit Interrupt Flag


#define   DS_COUNT      7

#endif
