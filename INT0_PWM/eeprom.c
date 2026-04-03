#include <stdint.h>

__sfr __at (0xC2) IAP_DATA;
__sfr __at (0xC3) IAP_ADDRH;
__sfr __at (0xC4) IAP_ADDRL;
__sfr __at (0xC5) IAP_CMD;
__sfr __at (0xC6) IAP_TRIG;
__sfr __at (0xC7) IAP_CONTR;

#define IAP_STANDBY 0x00
#define IAP_READ    0x01
#define IAP_WRITE   0x02
#define IAP_ERASE   0x03
#define IAP_ENABLE  0x80 // Assuming clock < 12MHz, check manual for Wait State bits

void IAP_Disable(void) {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0xFF;
    IAP_ADDRL = 0xFF;
}

void IAP_Erase(unsigned int addr) {
    IAP_CONTR = IAP_ENABLE;
    IAP_CMD = IAP_ERASE;
    IAP_ADDRH = addr >> 8;
    IAP_ADDRL = addr & 0xFF;
    IAP_TRIG = 0x5A; // Secret trigger sequence
    IAP_TRIG = 0xA5;
    __asm__("nop");  // Wait for hardware to finish
    IAP_Disable();
}

void IAP_Write(unsigned int addr, uint8_t dat) {
    IAP_CONTR = IAP_ENABLE;
    IAP_CMD = IAP_WRITE;
    IAP_ADDRH = addr >> 8;
    IAP_ADDRL = addr & 0xFF;
    IAP_DATA = dat;
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    __asm__("nop");
    IAP_Disable();
}

uint8_t IAP_Read(unsigned int addr) {
    uint8_t dat;
    IAP_CONTR = IAP_ENABLE;
    IAP_CMD = IAP_READ;
    IAP_ADDRH = addr >> 8;
    IAP_ADDRL = addr & 0xFF;
    IAP_TRIG = 0x5A;
    IAP_TRIG = 0xA5;
    __asm__("nop");
    dat = IAP_DATA;
    IAP_Disable();
    return dat;
}
