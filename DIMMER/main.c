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

#define IR_CMD_LOCK                             IR_CMD_TIMER
#define IR_CMD_INCR_BRIGHT                      IR_CMD_LED
#define IR_CMD_DECR_BRIGHT                      IR_CMD_SLEEP
#define EEPROM_PREV_BRIGHTNESS_ADDR             0x0000
#define DIMMER_STEPS_COUNT                      11

static uint8_t s_last_cmd = IR_CMD_POWER;

volatile int8_t g_current_brightness = 0;
volatile int8_t g_prev_brightness = 0;

// Array storing predefined duty cycle percentages (0% to 100%) in Code memory.
// Negative values indicate special handling for ultra-low brightness levels,
// where the ON time is less than 100 microseconds. These values should be
// interpreted by the PWM control logic to produce very dim output, possibly
// by using a fixed minimal pulse width instead of a percentage-based duty cycle.
__code const int8_t g_brightness_levels[DIMMER_STEPS_COUNT] = {-20, -5, -2, -1, 0, 1, 2, 5, 20, 50, 100};

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
    if(g_current_brightness != INT8_MIN ||
            (g_current_brightness == INT8_MIN && ir_cmd == IR_CMD_LOCK)
      ) {
        // Shift down to evaluate only the command byte (highest 8 bits)
        switch(ir_cmd) {
            case IR_CMD_POWER:
                g_current_brightness = INT8_MAX; // Special value to indicate power off (no light output)
                break;
            case IR_CMD_NUM1:
                g_current_brightness = g_brightness_levels[1];
                break;
            case IR_CMD_NUM2:
                g_current_brightness = g_brightness_levels[2];
                break;
            case IR_CMD_NUM3:
                g_current_brightness = g_brightness_levels[3];
                break;
            case IR_CMD_NUM4:
                g_current_brightness = g_brightness_levels[4];
                break;
            case IR_CMD_NUM5:
                g_current_brightness = g_brightness_levels[5];
                break;
            case IR_CMD_BOOST:
                g_current_brightness = g_brightness_levels[6];
                break;
            case IR_CMD_LOCK:
                if(g_current_brightness != INT8_MIN) {
                    g_prev_brightness = g_current_brightness;
                    g_current_brightness = INT8_MIN;
                } else {
                    g_current_brightness = g_prev_brightness;
                }
                break;
        }
        s_last_cmd = ir_cmd;
        EEPROM_Write(EEPROM_PREV_BRIGHTNESS_ADDR, g_current_brightness); // Save
    }

    printf("Brightness = %d\r\n", g_current_brightness);
}

/**
 * @brief   Main execution loop.
 */
void on_button_hold() {
    if(s_last_cmd == IR_CMD_INCR_BRIGHT) {
        if (g_current_brightness < g_brightness_levels[DIMMER_STEPS_COUNT - 1]) {
            g_current_brightness++;
        }
    } else if(s_last_cmd == IR_CMD_DECR_BRIGHT) {
        if (g_current_brightness > g_brightness_levels[0]) {
            g_current_brightness--;
        }
    }
}

void main(void) {
    // Configure P3.2 (COB_LED_PIN) as push-pull output for driving the MOSFET
    P3M1 &= ~(0x04);  // Clear Port 3 Mode 1 bit 2
    P3M0 |=  (0x04);  // Set Port 3 Mode 0 bit 2

    COB_LED_PIN = 1;
    UART1_Init();
    PWM_Timer0_Init();
    // After initialization, load previous value from EEPROM
    int8_t eeprom_brightness = EEPROM_Read(EEPROM_PREV_BRIGHTNESS_ADDR);
    // Sanity check: if value is invalid (e.g., -100), default to 0
    if(eeprom_brightness < g_brightness_levels[0] || 
            eeprom_brightness > g_brightness_levels[DIMMER_STEPS_COUNT - 1]) {
        g_current_brightness = 0;
    } else {
        g_current_brightness = eeprom_brightness;
    }

    // Infinite application loop
    set_on_repeat(on_button_hold);
    while(1) {
        // Check if a complete IR frame has been captured
        while (g_ir_data_ready) {
            unsigned long decoded_ir_code = NEC_DecodeCommand();
            printf("%04X\r\n", decoded_ir_code);
            process_NEC_command(decoded_ir_code >> 24);

            // Critical Section: Reset IR state machine and re-enable interrupts
            IE1 = 0;             // Clear External Interrupt 0 flag
            g_ir_data_ready = 0; // Reset readiness flag
            EX1 = 1;             // Re-enable External Interrupt 0
        }
    }
}
