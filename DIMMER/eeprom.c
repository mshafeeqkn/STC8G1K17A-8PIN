#include <stdint.h>
#include <stdio.h>
#include "config.h"

#include "config.h"
#include "eeprom.h"
// Wait times for 11.0592MHz (IAP_TPS is clock dependent)
#define IAP_TPS_VALUE 12 

// Sets the IAP (In-Application Programming) interface to idle state after EEPROM operations.
// This function should be called after any EEPROM read, write, or erase to ensure the interface is reset.
static void EEPROM_Idle(void) {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

/**
 * @brief Erase a byte in EEPROM at the specified address.
 *
 * Configures and triggers the IAP erase operation for the given EEPROM
 * address, then waits for the operation to complete.
 *
 * @param addr EEPROM address to erase.
 */
void EEPROM_Erase(uint16_t addr) {
    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 3;
    IAP_ADDRL = (uint8_t)(addr & 0xFF);
    IAP_ADDRH = (uint8_t)((addr >> 8) & 0xFF);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    EEPROM_Idle();
}

/**
 * @brief Read a byte from EEPROM at the specified address.
 *
 * Configures the IAP (In-Application Programming) interface for read mode,
 * triggers the read operation at the given address, and returns the data byte.
 *
 * @param addr EEPROM address to read (16-bit).
 * @return The data byte read from the specified EEPROM address.
 */
uint8_t EEPROM_Read(uint16_t addr) {
    char dat;
    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 1;
    IAP_ADDRL = (uint8_t)(addr & 0xFF);
    IAP_ADDRH = (uint8_t)((addr >> 8) & 0xFF);
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    dat = IAP_DATA;
    EEPROM_Idle();
    return dat;
}


/**
 * @brief Write a byte to EEPROM at the specified address.
 * 
 * Erases the target EEPROM location, configures the in-application
 * programming (IAP) registers, writes the provided data byte, and
 * triggers the write sequence.
 *
 * @param addr EEPROM address to write (16-bit).
 * @param dat  Data byte to write to EEPROM.
 */
void EEPROM_Write(uint16_t addr, uint8_t dat) {
    EEPROM_Erase(addr);

    IAP_CONTR = 0x80;
    IAP_TPS = IAP_TPS_VALUE;
    IAP_CMD = 2;
    IAP_ADDRL = (uint8_t)(addr & 0xFF);
    IAP_ADDRH = (uint8_t)((addr >> 8) & 0xFF);
    IAP_DATA = dat;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    __asm__("nop"); // Micro-delay
    EEPROM_Idle();
}
