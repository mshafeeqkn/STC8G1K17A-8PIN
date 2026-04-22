/**
 * @file    nec.h
 * @brief   Header for NEC IR Protocol Decoder.
 */

#ifndef __NEC_H__
#define __NEC_H__

#include "config.h"

// Flag indicating a complete IR frame has been successfully captured
extern volatile __bit g_ir_data_ready;
extern volatile uint8_t       g_ir_pulse_index;

void NEC_Decoder_Init(void);
void NEC_INT1_ISR(void) __interrupt (2);
unsigned long NEC_DecodeCommand(void);

#endif // __NEC_H__
