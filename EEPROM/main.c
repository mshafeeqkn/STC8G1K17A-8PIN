#include <8051.h>
#include <stdint.h>

/* --- STC8G SFR Definitions --- */
__sfr __at (0xC0) IAP_DATA;
__sfr __at (0xC1) IAP_ADDRH;
__sfr __at (0xC2) IAP_ADDRL;
__sfr __at (0xC3) IAP_CMD;
__sfr __at (0xC4) IAP_TRIG;
__sfr __at (0xC5) IAP_CONTR;
__sfr __at (0xC6) IAP_TPS;

__sbit __at(0xB2) COB_LED_PIN;
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

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
    P3M1 &= ~(0x04);  // Clear Port 3 Mode 1 bit 2
    P3M0 |=  (0x04);  // Set Port 3 Mode 0 bit 2
    uint8_t ch = 1;

    IapErase(0x0000);
    IapProgram(0x0000, 0x11);
    ch = IapRead(0x0000) & 0x1;
    COB_LED_PIN = ch & 0x1;
    // COB_LED_PIN = 1;
    
    while(1); // Done
}
