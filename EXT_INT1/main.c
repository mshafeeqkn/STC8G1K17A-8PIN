/* Register Definitions */
__sfr __at (0xB1) P3M1;
__sfr __at (0xB2) P3M0;

/* Bit Definitions */
__sbit __at (0xB0+2) LED; /* P3.2 */
__sbit __at (0xA8+7) EA;  /* Global Interrupt Enable */
__sbit __at (0xAA+0) EX1; /* INT1 Enable */
__sbit __at (0x8A) IT1; /* INT0 Type: 1=Falling Edge */

/* ISR for INT0 (P3.2) */
void int1_isr(void) __interrupt (2) {
    LED = !LED; 
}

void main(void) {
    /* 1. Configure P3.2 (INT0) as High-Impedance Input */
    /* Bit 2: M1=1, M0=0 */
    P3M1 |= 0x08;
    P3M0 &= ~0x08;

    /* 2. Configure P3.3 (LED) as Push-Pull Output */
    /* Bit 3: M1=0, M0=1 */
    P3M1 &= ~0x04;
    P3M0 |= 0x04;

    /* 3. Setup Interrupt */
    IT1 = 1;  // Falling edge trigger
    EX1 = 1;  // Enable INT0
    EA  = 1;  // Enable Global Interrupts

    while (1) {
        // CPU waits here
    }
}

