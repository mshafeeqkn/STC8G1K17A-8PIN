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

#define IR_CMD_LOCK     IR_CMD_LED


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
    // Configure P3.2 (COB_LED_PIN) as push-pull output for driving the MOSFET
    P3M1 &= ~(0x04);  // Clear Port 3 Mode 1 bit 2
    P3M0 |=  (0x04);  // Set Port 3 Mode 0 bit 2

    COB_LED_PIN = 1;
    EEPROM_Write(0x0000, 10); // Save
    EEPROM_Write(0x0001, 11); // Save
    EEPROM_Write(0x0002, 12); // Save
    EEPROM_Write(0x0003, 6); // Save
    COB_LED_PIN = 0;
    UART1_Init();
    while(1) {
        System_Delay_ms(1000);
        printf("Data written: %X\r\n", EEPROM_Read(0x0000));
        printf("Data written: %X\r\n", EEPROM_Read(0x0001));
        printf("Data written: %X\r\n", EEPROM_Read(0x0002));
        printf("Data written: %X\r\n", EEPROM_Read(0x0003));
    }
}
