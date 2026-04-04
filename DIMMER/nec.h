#ifndef __NEC_H__
#define __NEC_H__

#include "config.h"

extern volatile __bit         ir_ready;
void INT0_ISR(void) __interrupt (0);
unsigned long decode_nec(void);
void Init_NEC_Decoder(void);

#endif
