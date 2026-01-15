/**
 * @file 8279.h
 * @brief Header file for Intel 8279 keyboard/display controller interface
 *
 * @author stonedDiscord
 * @date 15.12.2025
 */
#ifndef HEADER_8279
#define HEADER_8279

#include <stdbool.h>

#ifndef I8279_IO
#error "Please set the I8279_IO address before including this file."
#endif


#define I8279_DATA    I8279_IO
#define I8279_CMD     I8279_DATA + 1

// Function prototypes
void kdc_cmd_out(uint8_t data);
void kdc_data_out(uint8_t data);
uint8_t kdc_cmd_in();
uint8_t kdc_data_in();
void set_kdc_clock(uint8_t divider);
uint8_t read_dram(uint8_t addr);
uint8_t read_sram(uint8_t addr);

#define I8279_MODE_SET 0x00
#define I8279_MODE_DISPLAY_8BIT 0x00
#define I8279_MODE_DISPLAY_16BIT 0x08
#define I8279_MODE_DISPLAY_LE 0x00
#define I8279_MODE_DISPLAY_RE 0x10
#define I8279_MODE_KEYBOARD_ENCODED 0x00
#define I8279_MODE_KEYBOARD_DECODED 0x01
#define I8279_MODE_KEYBOARD_2KEY_LOCKOUT 0x00
#define I8279_MODE_KEYBOARD_NKEY_ROLLOVER 0x02
#define I8279_MODE_KEYBOARD_SENSOR_MATRIX 0x04
#define I8279_MODE_KEYBOARD_STROBE 0x06

#define I8279_CLOCK_DIVIDER_SET 0x20

#define I8279_RW_AUTO_INCREMENT 0x10
#define I8279_READ_SENSOR_RAM 0x40
#define I8279_READ_DISPLAY_RAM 0x60
#define I8279_WRITE_DISPLAY_RAM 0x80
#define I8279_DISPLAY_BLANKING 0xA0

#define I8279_CLEAR 0xC0
#define I8279_CLEAR_DISPLAY 0x10
#define I8279_CLEAR_DISPLAY_ZEROES 0x00
#define I8279_CLEAR_DISPLAY_AB 0x08
#define I8279_CLEAR_DISPLAY_FF 0x0C
#define I8279_CLEAR_FIFO 0x02
#define I8279_CLEAR_ALL 0x01

#define I8279_END_INTERRUPT 0xE0
#define I8279_ERROR_MODE 0x10

/**
 * @brief FIFO status register structure for 8279 controller
 *
 * Contains the status information of the 8279's FIFO buffer including
 * character count, error conditions, and buffer full status.
 */
struct FIFO_STATUS {
    unsigned char count : 3;      ///< Number of characters in FIFO (0-8)
    bool full  : 1;               ///< FIFO full flag
    bool underrun : 1;            ///< FIFO underrun error flag
    bool overrun  : 1;            ///< FIFO overrun error flag
    bool senserr : 1;             ///< Sensor RAM error flag
    bool displayerr : 1;          ///< Display RAM error flag
};

/**
 * @brief Keyboard data structure for 8279 controller
 *
 * Contains the keyboard scan data including keycode, scan line,
 * and modifier key status (shift and control).
 */
struct KB_DATA {
    unsigned char keycode : 3;    ///< Key code (0-7)
    unsigned char scan: 3;        ///< Scan line number (0-7)
    bool shift: 1;                ///< Shift key pressed flag
    bool cntl: 1;                 ///< Control key pressed flag
};

/**
 * @brief Send command data to the 8279 keyboard/display controller
 *
 * @param data Command byte to send to the controller
 */
void kdc_cmd_out(uint8_t data) {
    uint8_t test = data;
    __asm
        OUT I8279_CMD
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
        OUT I8279_DATA
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
        IN I8279_CMD
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
        IN I8279_DATA
        MOV L,A
        PUSH HL
    __endasm;
    return out;
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
 * @brief Read data from the 8279's display RAM
 *
 * @param addr Address in display RAM (0-15)
 * @return uint8_t Data read from display RAM
 */
uint8_t read_dram(uint8_t addr) {
    kdc_cmd_out(I8279_READ_DISPLAY_RAM | (addr & 0x0F));
    return kdc_data_in();
}

/**
 * @brief Read data from the 8279's sensor RAM
 *
 * @param addr Address in sensor RAM (0-7)
 * @return uint8_t Data read from sensor RAM
 */
uint8_t read_sram(uint8_t addr) {
    kdc_cmd_out(I8279_READ_SENSOR_RAM | (addr & 7));
    return kdc_data_in();
}

#endif
