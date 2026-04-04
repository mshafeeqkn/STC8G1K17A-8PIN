/**
 * @file    main.c
 * @brief   Main application entry point for the IR-controlled COB LED Dimmer.
 * @details Initializes hardware peripherals (UART, PWM Timer, IR Decoder) and
 * enters the main control loop to process received NEC remote commands.
 */

#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "uart.h"
#include "pwm.h"
#include "nec.h"

/* --- IR Command Code Definitions --- */
// Extracted upper byte (command byte) of the NEC codes mapped to brightness levels
#define IR_CMD_BRIGHTNESS_0 0x6E
#define IR_CMD_BRIGHTNESS_1 0x71
#define IR_CMD_BRIGHTNESS_2 0x6F
#define IR_CMD_BRIGHTNESS_3 0x75
#define IR_CMD_BRIGHTNESS_4 0x74
#define IR_CMD_BRIGHTNESS_5 0x7A
#define IR_CMD_BRIGHTNESS_6 0x6D

/**
 * @brief   Software blocking delay.
 * @param   ms Number of milliseconds to delay.
 */
void System_Delay_ms(unsigned int ms) {
    unsigned int i;
    while(ms--) {
        // Inner loop tuned for specific clock frequency blocking delay
        for(i = 0; i < 1200; i++) { __asm__("nop"); }
    }
}

/**
 * @brief   Main execution loop.
 */
void main(void) {
    // Configure P3.3 (COB_LED_PIN) as push-pull output for driving the MOSFET
    P3M1 &= ~(1 << 3);  // Clear Port 3 Mode 1 bit 3
    P3M0 |= (1 << 3);   // Set Port 3 Mode 0 bit 3

    // Initialize system peripherals
    UART1_Init();
    PWM_Timer0_Init();
    NEC_Decoder_Init();

    // Infinite application loop
    while(1) {
        System_Delay_ms(1000);

        // Check if a complete IR frame has been captured
        if (g_ir_data_ready) {
            unsigned long decoded_ir_code = NEC_DecodeCommand();

            // Print the full 32-bit HEX code for debugging via serial
            printf("Code: %08lX\r\n", decoded_ir_code);

            // Shift down to evaluate only the command byte (highest 8 bits)
            switch(decoded_ir_code >> 24) {
                case IR_CMD_BRIGHTNESS_0:
                    g_current_brightness_level = 0;
                    break;
                case IR_CMD_BRIGHTNESS_1:
                    g_current_brightness_level = 1;
                    break;
                case IR_CMD_BRIGHTNESS_2:
                    g_current_brightness_level = 2;
                    break;
                case IR_CMD_BRIGHTNESS_3:
                    g_current_brightness_level = 3;
                    break;
                case IR_CMD_BRIGHTNESS_4:
                    g_current_brightness_level = 4;
                    break;
                case IR_CMD_BRIGHTNESS_5:
                    g_current_brightness_level = 5;
                    break;
                case IR_CMD_BRIGHTNESS_6:
                    g_current_brightness_level = 6;
                    break;
            }

            // Critical Section: Reset IR state machine and re-enable interrupts
            IE0 = 0;            // Clear External Interrupt 0 flag
            g_ir_data_ready = 0; // Reset readiness flag
            EX0 = 1;            // Re-enable External Interrupt 0
        }
    }
}
