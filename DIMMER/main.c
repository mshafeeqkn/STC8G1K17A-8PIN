#include <stdint.h>
#include <stdio.h> // Required for printf
#include "config.h"
#include "uart.h"
#include "pwm.h"
#include "nec.h"

void delay_ms(unsigned int ms) {
    unsigned int i;
    while(ms--) {
        for(i = 0; i < 1200; i++) { __asm__("nop"); }
    }
}

void main(void) {
    // Configure P3.3 as push-pull output: P3M1.3=0, P3M0.3=1
    P3M1 &= ~(1 << 3);  // Clear P3M1.3
    P3M0 |= (1 << 3);   // Set P3M0.3

    UART1_Init();
    Timer0_Setup();
    Init_NEC_Decoder();
    while(1) {
        delay_ms(1000); 
        if (ir_ready) {
            unsigned long code = decode_nec();
            printf("Code: %08lX\r\n", code);
            switch(code >> 24) {
                case 0x6E:
                    duty_cycle_index = 0;
                    break;
                case 0x71:
                    duty_cycle_index = 1;
                    break;
                case 0x6F:
                    duty_cycle_index = 2;
                    break;
                case 0x75:
                    duty_cycle_index = 3;
                    break;
                case 0x74:
                    duty_cycle_index = 4;
                    break;
                case 0x7A:
                    duty_cycle_index = 5;
                    break;
                case 0x6D:
                    duty_cycle_index = 6;
                    break;
            }
            IE0 = 0;
            ir_ready = 0;
            EX0 = 1;
        }
    }
}
