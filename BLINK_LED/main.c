// Core SFRs (8051 compatible)
__sbit __at(0xB3) LED;   // 0xB0 + 3 -> P3 + .3


// STC8G port mode registers
__sfr __at(0xB1) P3M1;
__sfr __at(0xB2) P3M0;

// Simple delay ~500ms at 11MHz (tune COUNT for your clock)
void delay(void) {
    unsigned int i, j;
    for(i = 0; i < 30000; i++) {
        for(j = 0; j < 100; j++) {
            ;  // NOP
        }
    }
}

void main(void) {
    // Configure P3.3 as push-pull output: P3M1.3=0, P3M0.3=1
    P3M1 &= ~(1 << 3);  // Clear P3M1.3
    P3M0 |= (1 << 3);   // Set P3M0.3

    while(1) {
        LED = 1;  // Turn on P3.3
        delay();
        LED = 0;  // Turn off P3.3
        delay();
    }
}
