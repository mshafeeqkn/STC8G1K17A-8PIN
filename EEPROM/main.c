#include <8051.h>
#include <stdint.h>
#include <stdio.h>

/* --- STC8G SFR Definitions --- */
__sfr __at (0xC2) IAP_DATA;
__sfr __at (0xC3) IAP_ADDRH;
__sfr __at (0xC4) IAP_ADDRL;
__sfr __at (0xC5) IAP_CMD;
__sfr __at (0xC6) IAP_TRIG;
__sfr __at (0xC7) IAP_CONTR;
__sfr __at (0xF5) IAP_TPS;

__sbit __at(0xB2) COB_LED_PIN;
__sfr __at (0x8E) AUXR; // Auxiliary register (used for timer speed control)
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

void System_Delay_ms(unsigned int ms) {
    unsigned int i;
    while(ms--) {
        // Inner loop tuned for specific clock frequency blocking delay
        for(i = 0; i < 1200; i++) { __asm__("nop"); }
    }
}

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
void IapIdle(void) {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

char IapRead(int addr) {
    char dat;
    IAP_CONTR = 0x80;
    IAP_TPS = 12;
    IAP_CMD = 1;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    dat = IAP_DATA;
    IapIdle();
    return dat;
}

void IapProgram(int addr, char dat) {
    IAP_CONTR = 0x80;
    IAP_TPS = 12;
    IAP_CMD = 2;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_DATA = dat;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    IapIdle();
}

void IapErase(int addr) {
    IAP_CONTR = 0x80;
    IAP_TPS = 12;
    IAP_CMD = 3;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    IapIdle();
}

void main(void) {
    UART1_Init();
    P3M1 &= ~(0x04);  // Clear Port 3 Mode 1 bit 2
    P3M0 |=  (0x04);  // Set Port 3 Mode 0 bit 2
    uint8_t ch = 1;

#if 0
    IapErase(0x0000);
    IapProgram(0x0000, 0xAB);
#else
    ch = IapRead(0x0000);
#endif
    // COB_LED_PIN = ch & 0x1;
    
    while(1) { // Done
        System_Delay_ms(1000);
        printf("Shafeeque = %X\r\n", ch);
    }
}
