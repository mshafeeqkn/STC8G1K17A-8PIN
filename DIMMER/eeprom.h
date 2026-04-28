#ifndef __EEPROM_H__
#define __EEPROM_H__

extern void EEPROM_Write(uint16_t addr, uint8_t dat);
extern uint8_t EEPROM_Read(uint16_t addr);

#endif
