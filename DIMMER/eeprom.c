#include <stdint.h>
#include <stdio.h>
#include "config.h"

#define IAP_ADDR_BRIGHTNESS 0x0000 // EEPROM start address
#include <stdint.h>
#include "config.h"
#include "eeprom.h"

// Wait times for 11.0592MHz (IAP_TPS is clock dependent)
#define IAP_TPS_VALUE 12 

static void EEPROM_Idle(void) {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

static void EEPROM_Erase(int addr) {
    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 3;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    EEPROM_Idle();
}

uint8_t EEPROM_Read(uint16_t addr) {
    char dat;
    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 1;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    dat = IAP_DATA;
    EEPROM_Idle();
    return dat;
}

void EEPROM_Write(uint16_t addr, uint8_t dat) {
    EEPROM_Erase(addr);

    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 2;
    IAP_ADDRL = (uint8_t)addr;
    IAP_ADDRH = (uint8_t)(addr >> 8);
    IAP_DATA = dat;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    EEPROM_Idle();
}
