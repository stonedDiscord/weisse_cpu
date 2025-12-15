/**
 * @file main.c
 * @brief Main program for 8279 keyboard/display controller interface
 *
 * This program provides an interface to the Intel 8279 keyboard/display controller,
 * which manages a keyboard matrix and 7-segment displays. The controller is
 * connected via I/O ports and provides functions for reading keyboard input,
 * controlling indicator lamps, and driving 7-segment displays.
 *
 * Hardware Configuration:
 * - 0x50-0x51: 8279 keyboard/display controller
 * - 0x60-0x6f: 8256 (if present)
 * - 0x72: Sound output
 *
 * The program initializes the controller in 16-bit display mode with encoded
 * keyboard scanning, then continuously scans for keyboard input and updates
 * the displays accordingly.
 *
 * @author stonedDiscord
 * @date 12.12.2025
 */

#include <stdint.h>
#include "8279.h"

#define KDC_DATA 0x50
#define KDC_CMD 0x51

/**
 * @brief Send command data to the 8279 keyboard/display controller
 *
 * @param data Command byte to send to the controller
 */
void kdc_cmd_out(uint8_t data) {
    uint8_t test = data;
	__asm
        OUT KDC_CMD
    __endasm;
}

/**
 * @brief Send data to the 8279 keyboard/display controller
 *
 * @param data Data byte to send to the controller
 */
void kdc_data_out(uint8_t data) {
    uint8_t test = data;
	__asm
        OUT KDC_DATA
    __endasm;
}

/**
 * @brief Read status data from the 8279 keyboard/display controller
 *
 * @return uint8_t Status byte received from the controller
 */
uint8_t kdc_cmd_in() {
    uint8_t out;
	__asm
        POP HL
        IN KDC_CMD
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

/**
 * @brief Read data from the 8279 keyboard/display controller
 *
 * @return uint8_t Data byte received from the controller
 */
uint8_t kdc_data_in() {
    uint8_t out;
	__asm
        POP HL
        IN KDC_DATA
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

/**
 * @brief Read data from the 8279's sensor RAM
 *
 * @param addr Address in sensor RAM (0-7)
 * @return uint8_t Data read from sensor RAM
 */
uint8_t readSram(uint8_t addr) {
    kdc_cmd_out(I8279_READ_SENSOR_RAM | (addr & 7));
    return kdc_data_in();
}

/**
 * @brief Write data to indicator lamps
 *
 * @param line Lamp line number (0-7)
 * @param data Data to write to the lamp line
 */
void writeLamps(uint8_t line, uint8_t data) {
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | (line & 7));
    kdc_data_out(data);
}

/**
 * @brief Write data to 7-segment display digits
 *
 * @param digit Digit position (0-7)
 * @param mon Digit on the money display
 * @param srv Digit on the service display
 */
void writeDigits(uint8_t digit, uint8_t mon, uint8_t srv) {
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | ((digit & 7) + 8));
    kdc_data_out((mon << 4) | srv);
}

/**
 * @brief Initialize the 8279 keyboard/display controller
 *
 * Sets up the controller in 16-bit display mode, left entry,
 * encoded keyboard mode with sensor matrix, configures clock divider,
 * clears all displays and FIFOs, and ends interrupt mode.
 *
 * @note Uses clock divider of 30 for the Merkur Board
 */
void init_kdc() {
    kdc_cmd_out(I8279_MODE_SET |
                I8279_MODE_DISPLAY_16BIT |
                I8279_MODE_DISPLAY_LE |
                I8279_MODE_KEYBOARD_ENCODED |
                I8279_MODE_KEYBOARD_SENSOR_MATRIX);
    set_kdc_clock(30);
    kdc_cmd_out(I8279_CLEAR | I8279_CLEAR_ALL);
    kdc_cmd_out(I8279_END_INTERRUPT);
}

/**
 * @brief Software delay function
 *
 * @param ms Number of milliseconds to delay
 * @note This is a simple busy-wait loop and not precise
 */
void delay(uint16_t ms) {
    uint16_t i;
    for (i = 0; i < ms; i++) {
        ;
    }
}

/**
 * @brief Set the 8279 keyboard/display controller clock divider
 *
 * @param divider Clock divider value (2-31)
 *
 * Sets the clock frequency for the 8279 controller. 
 *
 * @note Values outside the valid range will be clamped
 */
void set_kdc_clock(uint8_t divider) {
    // Ensure divider is within valid range (2-31)
    if (divider < 2) divider = 2;
    if (divider > 31) divider = 31;
    
    kdc_cmd_out(I8279_CLOCK_DIVIDER_SET | divider);
}

/**
 * @brief Main program entry point
 *
 * Initializes the 8279 controller, lights all indicator lamps,
 * displays test pattern on 7-segment displays, and enters
 * an infinite loop that continuously scans keyboard input
 * and updates displays accordingly.
 */
void main(void) {
    uint8_t i;

    init_kdc();

    // Lamps
    for (i = 0; i < 8; i++) {
        writeLamps(i,0xFF);
    }

    // 7-Segment displays
    for (i = 0; i < 8; i++) {
        writeDigits(i,0x03,0x04);
    }

    uint8_t last_key = 0;

    // Infinite loop to scan the keyboard
    while (1) {
        delay(150);
        last_key = readSram(i);
        delay(150);
        writeDigits(i, last_key >> 4, last_key & 0x0F);
        delay(150);
        writeLamps(i, last_key);
        delay(150);
     
        kdc_cmd_out(I8279_END_INTERRUPT);

        i++;
        if (i >= 8) {
            i = 0;
        }
    }
}
