#ifndef __EEPROM_H__
#define __EEPROM_H__

void IAP_Disable(void);
void IAP_Erase(unsigned int addr);
void IAP_Write(unsigned int addr, uint8_t dat);
uint8_t IAP_Read(unsigned int addr);

#endif
