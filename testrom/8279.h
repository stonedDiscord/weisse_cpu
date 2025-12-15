/**
 * @file 8279.h
 * @brief Header file for Intel 8279 keyboard/display controller interface
 *
 * @author stonedDiscord
 * @date 15.12.2025
 */

#include <stdbool.h>

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
