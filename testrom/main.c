/**
 * @file main.c
 * @brief Main program for adp Steuereinheit
 *
 * This program interacts with a type of German slot machine in use from the late 1980s-early 1990s
 *
 * Hardware Configuration:
 * - 0x50-0x51: 8279 keyboard/display controller
 * - 0x60-0x6f: 8256 UART
 * - 0x70-0x73: 8255
 *
 * The program initializes the controller in 16-bit display mode with encoded
 * keyboard scanning, then continuously scans for keyboard input and updates
 * the displays accordingly.
 *
 * @author stonedDiscord
 * @date 12.12.2025
 */

#include <stdint.h>
#include <stddef.h>

#define I8279_IO    0x50
#include "8279.c"

#define I8256_IO    0x60
#include "8256.c"

enum COUNTER_VALS {
    COUNTERS_START_SOUND = 0x01,
    COUNTERS_GONG        = 0x02,
    COUNTERS_MUTE        = 0x04,
    COUNTERS_UG          = 0x08,
    COUNTERS_DS          = 0x10,
    COUNTERS_US          = 0x20,
    COUNTERS_DM          = 0x40,
    COUNTERS_UM          = 0x80,
};

#include "track.c"

#define COINS       0x70
#define COUNTERS    0x71
#define SOUND       0x72

bool timer3_flag = false;
bool timer5_flag = false;

// timer2
void _8085_int1() {
    uint8_t out=0xaa;
}
// timer3
void _8085_int3() {
    timer3_flag = false;
}
// tx int
void _8085_int5() {
    uint8_t out=0xaa;
}
//timer5
void _8085_int7() {
    timer5_flag = false;
}

/**
 * @brief Keyboard/display controller interrupt handler RST65
 *
 * Currently a placeholder function.
 */
void _8085_int65() {
    uint8_t out=0xaa;
}

/**
 * @brief Scan interrupt handler RST75
 *
 * Currently a placeholder function.
 */
void _8085_int75() {
    uint8_t out=0xaa;
}

/**
 * @brief Sound interrupt handler RST55
 *
 * Currently a placeholder function.
 */
void _8085_int55() {
    uint8_t out=0xaa;
}

/**
 * @brief Set the counter outputs via port 0x71
 * Used for the sound timer and mute control
 * @param data Data to write to the counter outputs
 */
void counter_out(uint8_t data) {
    uint8_t test = data;
    __asm
        OUT COUNTERS
    __endasm;
}

/**
 * @brief Set the sound note
 *
 * @param note Note value to play
 */
void set_sound(uint8_t note) {
    uint8_t test = note;
    __asm
        OUT SOUND
    __endasm;
}

void wait_timer3(uint8_t data) {
    timer3_flag = true;
    enable_interrupts(I8256_INT_L3);
    set_timer3(data); // Set timer value as needed
    while (timer3_flag) {
        // Wait for timer3_flag to be cleared in interrupt
    }
}

/**
 * @brief Write data to indicator lamps
 *
 * @param line Lamp line number (0-7)
 * @param data Data to write to the lamp line
 */
void write_lamps(uint8_t line, uint8_t data) {
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
void write_digit(uint8_t digit, uint8_t mon, uint8_t srv) {
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | ((digit & 7) + 8));
    kdc_data_out((mon << 4) | srv);
}

/**
 * @brief Write a decimal number to the Serie display in the first 3 digits
 *
 * @param number Number to write to the lamp line
 */
void write_serie(uint8_t number) {
        // Convert data1 to decimal and display on digits 7-5
        uint8_t hundreds = number / 100;
        uint8_t tens = (number % 100) / 10;
        uint8_t ones = number % 10;
        
        write_digit(7, hundreds, hundreds);
        write_digit(6, tens, tens);
        write_digit(5, ones, ones);
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
 * 
 * Configures the MUART for 4800 baud, 8 data bits, 1 stop bit,
 * enables receiver and transmitter, sets port modes, and enables interrupts.
 * 
 */
void init_muart() {
    __asm
        MVI A, I8256_CMD1_FRQ_1K | I8256_CMD1_8085 | I8256_CMD1_STOP_1 | I8256_CMD1_CHARLEN_8
        OUT I8256_CMD1
        MVI A, I8256_CMD2_SCLK_DIV3 | 5 //4800baud
        OUT I8256_CMD2
        MVI A, I8256_CMD3_RESET | I8256_CMD3_IAE | I8256_CMD3_RXE | I8256_CMD3_SET
        OUT I8256_CMD3
        MVI A, I8256_MODE_PORT2C_OO
        OUT I8256_MODE
        MVI A, 0x70
        OUT I8256_PORT1C
        MVI A, 0xff
        OUT I8256_PORT2
        MVI A, 0x30
        OUT I8256_PORT1
        MVI A, 0x08
        OUT I8256_INTEN
        MVI A, 0xBA
        OUT I8256_INTAD
    __endasm;
}

/**
 * @brief Software delay function
 *
 * @param ms Number of milliseconds to delay
 * @note Timer 3 works at either 1kHz or 16kHz depending on configuration.
 *       This function assumes a 1kHz timer for millisecond delays.
 */
void delay(uint16_t ms) {
    uint8_t full_chunks = ms / 255;
    uint8_t remainder = ms % 255;
    
    // Wait for full 255ms chunks
    for (uint8_t i = 0; i < full_chunks; i++) {
        wait_timer3(255);
    }
    
    // Wait for the remaining milliseconds
    if (remainder > 0) {
        wait_timer3(remainder);
    }
}

/**
 * @brief Software delay function
 *
 * @param ms Number of milliseconds to delay
 * @note This is a simple busy-wait loop and not very precise
 */
void dumb_delay(uint16_t ms) {
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

void wait_tx_ready() {
    while ((read_status() & I8256_STATUS_TBE) == 0) {
        delay(1);
    }
}

/**
 * @brief Set data to be sent via MUART
 *
 * @param txdata Data to transmit
 */
void print_serial_char(uint8_t txdata) {
    wait_tx_ready();
    write_buffer(txdata);
}

uint8_t read_serial_char() {
    return read_buffer();
}

void print_string(const char* str) {
    while (*str) {
        print_serial_char(*str);
        str++;
    }
}

void play_note(uint8_t note, uint8_t octave, uint8_t duration)
{
    uint8_t l_notedata = 0;
    l_notedata |= (note & 0x0F);
    l_notedata |= ((duration & 0x03) << 4);
    l_notedata |= ((octave & 0x03) << 6);    
    set_sound(l_notedata);
    counter_out(COUNTERS_START_SOUND);
    delay(1);
    counter_out(0);
}

void play_track()
{
    for (uint16_t i = 0; i < sizeof(track) / sizeof(track[0]); i++)
    {
        play_note(track[i].note, track[i].octave, track[i].duration);
        write_digit(0, track[i].note, track[i].note);
        write_digit(1, track[i].octave, track[i].octave);
        write_digit(2, track[i].duration, track[i].duration);
        write_digit(3, track[i].length, track[i].length);
        write_digit(4, i, i);

        // Print lyric to serial port
        if (track[i].lyric != 0)
        {
            const char *lyric = track[i].lyric;
            print_string(lyric);
            print_string(" ");
        }

        delay(track[i].length * 2);
    }
}

/**
 * @brief Macro to define a button with row, column, and inversion
 *
 * @param row Row number (0-7)
 * @param col Column number (0-7)
 * @param inverted Inversion flag (0 or 1)
 */
#define BUTTON(row, col, inverted) (uint8_t)(((row & 0x07) << 4) | (col & 0x0F) | ((inverted & 0x01) << 7))

// Button definitions
#define RISK_LEFT  BUTTON(3, 1, 1)
#define RISK_RIGHT BUTTON(3, 0, 1)
/**
 * @brief Check the state of a button
 *
 * @param button Button identifier (row, column, and inversion flag)
 * @return bool State of the button (0 or 1)
 */
bool check_button(uint8_t button) {
    uint8_t row = (button >> 4) & 0x07;
    uint8_t col = button & 0x0F;
    uint8_t inverted = (button >> 7) & 0x01;
    
    uint8_t sram_data = read_sram(row);
    bool button_state = (sram_data >> col) & 0x01;
    
    if (inverted) {
        return !button_state;
    } else {
        return button_state;
    }
}

/**
 * @brief Main program entry point
 *
 * Initializes the controllers, plays the track
 * and then displays the state of the inputs on the lamp matrix
 * and echoes back any serial input
 */
void main(void) {
    int8_t i;

    init_kdc();
    init_muart();

    uint8_t keys[16];
    uint8_t data1;

    delay(80);

    play_track();

    // Infinite loop to scan the keyboard
    while (1) {

        data1 = read_sram(i);
        
        write_serie(data1);

        write_digit(0, i, i);

        if (check_button(RISK_LEFT)) {
            i--;
        } else if (check_button(RISK_RIGHT)) {
            i++;
        }

        kdc_cmd_out(I8279_END_INTERRUPT);

        if (i < 0) {
            i = 7;
        }

        if (i >= 8) {
            i = 0;
            kdc_cmd_out(I8279_CLEAR | I8279_CLEAR_FIFO);
        }

        if(read_status() & I8256_STATUS_RBF) {
            uint8_t rcv = read_serial_char();
            print_serial_char(rcv);
        }
    }
}
