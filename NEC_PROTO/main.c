#include <8051.h>
#include <stdint.h>
#include <stdio.h>

#include "uart.h"


void delay_ms(unsigned int ms) {
    unsigned int i;
    while(ms--) {
        for(i = 0; i < 1200; i++) { __asm__("nop"); }
    }
}

void main(void) {
    Init_UART();
    while (1) {
        printf("Shafeeque\n\n\r");
        delay_ms(1000);
    }
}
