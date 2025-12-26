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
#include "8256.h"
#include "8279.h"

enum IO71 {
    IO71_START_SOUND = 0x01,
    IO71_GONG = 0x02,
    IO71_MUTE = 0x04,
    IO71_UG = 0x08,
    IO71_DS = 0x10,
    IO71_US = 0x20,
    IO71_DM = 0x40,
    IO71_UM = 0x80,
};

typedef enum note {
    NOTE_INVALID = 0,
    NOTE_C = 1,
    NOTE_B = 2,
    NOTE_AS = 3,
    NOTE_A = 4,
    NOTE_GS = 5,
    NOTE_G = 6,
    NOTE_FS = 7,
    NOTE_F = 8,
    NOTE_E = 9,
    NOTE_DS = 10,
    NOTE_D = 11,
    NOTE_CS = 12,
} note_t;

typedef enum octave {
    A8 = 0,
    A7 = 1,
    A6 = 2,
    A5 = 3,
} octave_t;

typedef enum duration {
    DURATION_EIGHTH = 0,
    DURATION_QUARTER = 1,
    DURATION_HALF = 2,
    DURATION_WHOLE = 3,
} duration_t;

struct noteData
{
    note_t note: 4;
    octave_t octave: 2;
    duration_t duration: 2;
    uint16_t length;
};

#include "track.c"

#define KDC_DATA 0x50
#define KDC_CMD 0x51

#define MUART   0x60

#define SOUND   0x72

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

void powerOuts(uint8_t data) {
    uint8_t test = data;
	__asm
        OUT 0x71
    __endasm;
}

/**
 * @brief Play a sound note
 *
 * @param note Note value to play
 */
void playSound(uint8_t note) {
    uint8_t test = note;
	__asm
        OUT SOUND
    __endasm;
}

void playNote(uint8_t note, uint8_t octave, uint8_t duration)
{
    uint8_t notedata = (note & 0x0F);
    notedata |= ((octave & 0x03) << 4);
    notedata |= ((duration & 0x03) << 6);
    playSound(notedata);
}

/**
 * @brief Read data from the 8256 MUART's Port 1
 *
 * @return uint8_t Data read from port
 */
uint8_t readPort1() {
    uint8_t out=0xaa;
	__asm
        POP HL
        IN (MUART + I8256_PORT1)
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

/**
 * @brief Read data from the 8256 MUART's Port 2
 * @return uint8_t Data read from port
 */
uint8_t readPort2() {
    uint8_t out=0xaa;
    __asm
        POP HL
        IN (MUART + I8256_PORT2)
        MOV L,A
        PUSH HL
    __endasm;
    return out;
}

/**
 * @brief Read data from the 8279's display RAM
 *
 * @param addr Address in display RAM (0-15)
 * @return uint8_t Data read from display RAM
 */
uint8_t readDram(uint8_t addr) {
    kdc_cmd_out(I8279_READ_DISPLAY_RAM | (addr & 0x0F));
    return kdc_data_in();
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
 * @brief Initialize the 8256 MUART
 */
void init_muart() {
	__asm
        MVI A, I8256_CMD1_FRQ_1K | I8256_CMD1_8085 | I8256_CMD1_STOP_1 | I8256_CMD1_CHARLEN_8
        OUT (MUART + I8256_CMD1)
        MVI A, (I8256_CMD2_SCLK_DIV3 | 5) //4800baud
        OUT (MUART + I8256_CMD2)
        MVI A, (I8256_CMD3_RESET | I8256_CMD3_IAE | I8256_CMD3_RXE | I8256_CMD3_SET)
        OUT (MUART + I8256_CMD3)
        MVI A, (I8256_MODE_PORT2C_OO)
        OUT (MUART + I8256_MODE)
        MVI A, 0x70
        OUT (MUART + I8256_PORT1C)
        MVI A, 0xff
        OUT (MUART + I8256_PORT2)
        MVI A, 0x30
        OUT (MUART + I8256_PORT1)
        MVI A, 0xBA
        OUT (MUART + I8256_INTAD)
    __endasm;
}

/**
 * @brief Software delay function
 *
 * @param ms Number of milliseconds to delay
 * @note This is a simple busy-wait loop and not very precise
 */
void delay(uint16_t ms) {
    uint16_t i;
    for (i = 0; i < ms; i++) {
        for (uint8_t j = 0; j < 28; j++) {
        __asm
            NOP
            NOP
            NOP
        __endasm
        }
    }
}

void startSound() {
    powerOuts(IO71_START_SOUND);
    delay(1);
    playSound(0);
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
    init_muart();

    uint8_t keys[16];
    uint8_t port1;
    uint8_t port2;

    for (uint16_t i=0; i<sizeof(track)/sizeof(track[0]); i++) {
        delay(track[i].length);
        playNote(track[i].note, track[i].octave, track[i].duration);
        startSound();
        writeDigits(0, track[i].note, track[i].note);
        writeDigits(1, track[i].octave, track[i].octave);
        writeDigits(2, track[i].duration, track[i].duration);
        writeDigits(3, track[i].length, track[i].length);
        writeDigits(4, i, i);
    }

    // Infinite loop to scan the keyboard
    while (1) {

        delay(45);
        keys[i] = readSram(i);

        writeDigits(0, keys[0] & 0x0F,  keys[1] & 0x0F);
        writeDigits(1, keys[0] >> 4,    keys[1] >> 4);

        writeDigits(2, keys[2] & 0x0F,  keys[3] & 0x0F);
        writeDigits(3, keys[2] >> 4,    keys[3] >> 4);

        port1 = readPort1();
        port2 = readPort2();

        writeLamps(6, port1);
        writeDigits(4, port1 & 0x0F,  port1 & 0x0F);
        writeDigits(5, port1 >> 4,    port1 >> 4);

        writeLamps(7, port2);
        writeDigits(6, port2 & 0x0F,  port2 & 0x0F);
        writeDigits(7, port2 >> 4,    port2 >> 4);

        if (i<6)
            writeLamps(i, keys[i]);
        
        kdc_cmd_out(I8279_END_INTERRUPT);

        i++;
        if (i >= 16) {
            i = 0;
            kdc_cmd_out(I8279_CLEAR | I8279_CLEAR_FIFO);
        }
    }
}
