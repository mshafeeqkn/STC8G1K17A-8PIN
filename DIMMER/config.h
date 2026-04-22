/**
 * @file    config.h
 * @brief   Global hardware definitions and configurations for the STC8G1K08 microcontroller.
 * @details Contains Core SFR mappings, pin definitions for the 12V COB LED strip,
 * and globally shared macros.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <8051.h>

/* --- Hardware Pin Definitions --- */
// P3.2 mapped for COB LED PWM output (0xB0 + 2)
__sbit __at(0xB2) COB_LED_PIN;
/* --- Bit Definitions for SDCC Compatibility --- */
__sbit __at (0x8E) TR1; // Timer 1 Run control bit
__sbit __at (0x99) TI;  // UART Transmit Interrupt Flag


/* --- STC8G Specific Special Function Registers (SFRs) --- */
__sfr __at (0x8E) AUXR; // Auxiliary register (used for timer speed control)
__sfr __at (0xB1) P3M1; // Port 3 Mode configuration register 1
__sfr __at (0xB2) P3M0; // Port 3 Mode configuration register 0

/* --- Application Specific Macros --- */
// Total number of brightness levels available for the dimmer
#define DIMMER_STEPS_COUNT 7

#endif // __CONFIG_H__
