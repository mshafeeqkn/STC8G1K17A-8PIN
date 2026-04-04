/**
 * @file    uart.c
 * @brief   UART Driver for debug communications.
 * @details Configures Timer 1 as the baud rate generator for UART1. Includes
 * the necessary bridge function to allow printf() functionality via SDCC.
 */

#include <8051.h>
#include "config.h"

/**
 * @brief   Hardware bridge for SDCC's standard library printf().
 * @details SDCC internally calls putchar() to send individual characters.
 * @param   c The character to transmit.
 * @return  The transmitted character.
 */
int putchar(int c) {
    while (!TI);    // Block until the UART is ready (previous transmission finished)
    TI = 0;         // Manually clear the Transmit Interrupt flag
    SBUF = (char)c; // Load the new character into the Serial Buffer
    return c;
}

/**
 * @brief   Initializes UART1 for 9600 Baud at an 11.0592MHz system clock.
 */
void UART1_Init(void) {
    // 1. GPIO Configuration
    // Set P3.1 (TX) to Push-Pull output, P3.0 (RX) to High-Impedance input
    P3M1 &= ~0x02; P3M0 |= 0x02; // P3.1 TX config
    P3M1 |= 0x01;  P3M0 &= ~0x01; // P3.0 RX config

    // 2. UART Configuration
    SCON = 0x50; // Set UART1 to Mode 1 (8-bit variable baud rate), Enable Receiver

    // 3. Timer 1 Configuration (Baud Rate Generator)
    AUXR |= 0x40;  // Set Timer 1 to 1T mode (Fast mode, no prescaler)
    AUXR &= ~0x01; // Select Timer 1 as the baud rate generator for UART1

    TMOD &= 0x0F;  // Clear Timer 1 configuration bits
    TMOD |= 0x20;  // Configure Timer 1 for Mode 2 (8-bit auto-reload)

    // 4. Baud Rate Calculation
    // Target: 9600 Baud @ 11.0592 MHz system clock
    // Calculation: 256 - (11059200 / 32 / 9600) = 220 (0xDC)
    TH1 = 0xDC; // Preload high byte
    TL1 = 0xDC; // Preload low byte

    // 5. Start Peripherals
    TR1 = 1; // Start Timer 1
    TI = 1;  // CRITICAL: Pre-set TI flag so the first printf() call doesn't hang infinitely
}
