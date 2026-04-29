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
#include "eeprom.h"

/* --- IR Command Code Definitions --- */
// Extracted upper byte (command byte) of the NEC codes mapped to brightness levels
#define IR_CMD_POWER    0x6E
#define IR_CMD_NUM1     0x74
#define IR_CMD_NUM2     0x6F
#define IR_CMD_NUM3     0x75
#define IR_CMD_NUM4     0x6C
#define IR_CMD_NUM5     0x77
#define IR_CMD_BOOST    0x70
#define IR_CMD_TIMER    0x69
#define IR_CMD_LED      0x68
#define IR_CMD_SLEEP    0x71

#define IR_CMD_LOCK                             IR_CMD_LED
#define EEPROM_PREV_BRIGHTNESS_ADDR             0x0000

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

static void process_NEC_command(uint8_t ir_cmd) {
    if(g_current_brightness_level != 255 ||
            (g_current_brightness_level == 255 && ir_cmd == IR_CMD_LOCK)
      ) {
        // Shift down to evaluate only the command byte (highest 8 bits)
        switch(ir_cmd) {
            case IR_CMD_POWER:
                g_current_brightness_level = 0;
                break;
            case IR_CMD_NUM1:
                g_current_brightness_level = 1;
                break;
            case IR_CMD_NUM2:
                g_current_brightness_level = 2;
                break;
            case IR_CMD_NUM3:
                g_current_brightness_level = 3;
                break;
            case IR_CMD_NUM4:
                g_current_brightness_level = 4;
                break;
            case IR_CMD_NUM5:
                g_current_brightness_level = 5;
                break;
            case IR_CMD_BOOST:
                g_current_brightness_level = 6;
                break;
            case IR_CMD_LOCK:
                if(g_current_brightness_level != 255) {
                    g_prev_brightness = g_current_brightness_level;
                    g_current_brightness_level = 255;
                } else {
                    g_current_brightness_level = g_prev_brightness;
                }
                break;
        }
        EEPROM_Write(EEPROM_PREV_BRIGHTNESS_ADDR, g_current_brightness_level); // Save
    }

    printf("Brightness = %u\r\n", g_current_brightness_level);
}

/**
 * @brief   Main execution loop.
 */
void main(void) {
    // Configure P3.2 (COB_LED_PIN) as push-pull output for driving the MOSFET
    P3M1 &= ~(0x04);  // Clear Port 3 Mode 1 bit 2
    P3M0 |=  (0x04);  // Set Port 3 Mode 0 bit 2

    COB_LED_PIN = 1;
    UART1_Init();
    PWM_Timer0_Init();
    NEC_Decoder_Init();

    // After initialization, load previous value from EEPROM
    g_current_brightness_level = EEPROM_Read(EEPROM_PREV_BRIGHTNESS_ADDR);
    // Sanity check: if value is invalid (e.g., 255), default to 0
    if(g_current_brightness_level >= DIMMER_STEPS_COUNT && g_current_brightness_level != 255) {
        g_current_brightness_level = 0;
    }

    // Infinite application loop
    while(1) {
        // System_Delay_ms(100);

        // Check if a complete IR frame has been captured
        while (g_ir_data_ready) {
            unsigned long decoded_ir_code = NEC_DecodeCommand();
            process_NEC_command(decoded_ir_code >> 24);

            // Critical Section: Reset IR state machine and re-enable interrupts
            IE1 = 0;             // Clear External Interrupt 0 flag
            g_ir_data_ready = 0; // Reset readiness flag
            EX1 = 1;             // Re-enable External Interrupt 0
        }
    }
}
