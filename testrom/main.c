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

#include "hd146818.c"

// Function prototypes
void enable_interrupts();
void _8085_int1();
void _8085_int3();
void _8085_int5();
void _8085_int7();
void _8085_int65();
void read_sensor_matrix();
void _8085_int75();
void _8085_int55();
void counter_out(uint8_t data);
void set_sound(uint8_t note);
void wait_timer3(uint8_t data);
void write_lamps(uint8_t line, uint8_t data);
void write_money(uint8_t digit, uint8_t value);
void write_service(uint8_t digit, uint8_t value);
void write_both(uint8_t digit, uint8_t value);
void refresh_display();
void write_serie(uint8_t number);
void init_kdc();
void init_muart();
void delay(uint16_t ms);
void dumb_delay(uint16_t ms);
void wait_tx_ready();
void print_serial_char(uint8_t txdata);
uint8_t read_serial_char();
void print_string(const char* str);
void play_note(uint8_t note, uint8_t octave, uint8_t duration);
void play_track();
bool check_button(uint8_t button);
void display_rtc_date();
void display_rtc_time();

volatile struct rtc_state *rtc;
volatile uint8_t *rtc_a;
volatile uint8_t *rtc_b;

#define COINS       0x70
#define COUNTERS    0x71
#define SOUND       0x72

bool timer3_flag = false;
bool blink_flag = false;
bool date_edit_mode = false;
bool time_edit_mode = false;
int8_t selected_digit = -1;

uint8_t sensor_ram[8];
uint8_t sensor_row = 0;

uint8_t money_display[8];
uint8_t service_display[8];

/**
 * @brief Enable Interrupts
 *
 * @param data Interrupt array
 */
void enable_interrupts() {
    __asm
        EI
    __endasm;
}

// timer2
void _8085_int1() {
    uint8_t out=0xaa;
    enable_interrupts();
}
// timer3
void _8085_int3() {
    timer3_flag = false;
    enable_interrupts();
}
// tx int
void _8085_int5() {
    uint8_t out=0xaa;
    enable_interrupts();
}
//timer5
void _8085_int7() {
    blink_flag = !blink_flag;
    refresh_display();
    set_timer5(250);
    enable_muart_interrupts(I8256_INT_L7);
}

/**
 * @brief Keyboard/display controller interrupt handler RST65
 *
 * Currently a placeholder function.
 */
void _8085_int65() {
    read_sensor_matrix();
    enable_interrupts();
}

void read_sensor_matrix() {
    for (uint8_t row = 0; row < 8; row++) {
        sensor_ram[row] = read_sram(row);
    }
}

/**
 * @brief Scan interrupt handler RST75
 *
 * Currently a placeholder function.
 */
void _8085_int75() {
    refresh_display();
    enable_interrupts();
}

/**
 * @brief Sound interrupt handler RST55
 *
 * Currently a placeholder function.
 */
void _8085_int55() {
    uint8_t out=0xaa;
    enable_interrupts();
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
    set_timer3(data); // Set timer value as needed
    enable_muart_interrupts(I8256_INT_L3);
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
void write_money(uint8_t digit, uint8_t value) {
    money_display[digit] = value;
}

void write_service(uint8_t digit, uint8_t value) {
    service_display[digit] = value;
}

void write_both(uint8_t digit, uint8_t value) {
    money_display[digit] = value;
    service_display[digit] = value;
}

void update_blink()
{
    // Clear all blink flags first
    for (uint8_t digit = 0; digit < 8; digit++) {
        money_display[digit] &= 0x0F;
        service_display[digit] &= 0x0F;
    }

    // Set the blink flag for the selected digit
    if (selected_digit >= 0 && selected_digit < 8) {
    money_display[selected_digit] |= 0xF0;
    service_display[selected_digit] |= 0xF0;
    }    
}

void refresh_display() {
    update_blink();
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | I8279_RW_AUTO_INCREMENT | 8);
    for (uint8_t digit = 0; digit < 8; digit++) {

        // blink logic: if any of the upper nibbles are set, blink the digit
        uint8_t money_digit, service_digit;

        if ((money_display[digit] >> 4) && blink_flag)
           money_digit = 0xf0;
        else
           money_digit = money_display[digit] << 4;


        if ((service_display[digit] >> 4) && blink_flag)
           service_digit = 0x0f;
        else
           service_digit = service_display[digit] & 0x0f;
        
        kdc_data_out(money_digit | service_digit);
    }
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
        
        write_money(7, hundreds);
        write_money(6, tens);
        write_money(5, ones);
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
        write_both(0, track[i].note);
        write_both(1, track[i].octave);
        write_both(2, track[i].duration);
        write_both(3, track[i].length);
        write_both(4, i);
        // Print lyric to serial port
        if (track[i].lyric != 0)
        {
            const char *lyric = track[i].lyric;
            print_string(lyric);
            print_string(" ");
        }

        delay(track[i].length * 3);
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
#define STOP_MID   BUTTON(3, 3, 1)
#define RISK_RIGHT BUTTON(3, 0, 1)
#define RETURN     BUTTON(0, 3, 1)
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
    
    bool button_state = (sensor_ram[row] >> col) & 0x01;
    
    if (inverted) {
        return !button_state;
    } else {
        return button_state;
    }
}

void display_rtc_date()
{
    write_both(7, (rtc->day_of_month / 10));
    write_both(6, (rtc->day_of_month % 10));

    write_both(5, 0xff);

    write_both(4, (rtc->month / 10));
    write_both(3, (rtc->month % 10));

    write_both(2, 0xff);

    write_both(1, (rtc->year / 10));
    write_both(0, (rtc->year % 10));
}

// Function to display the date from the RTC
void display_rtc_time()
{
    write_both(7, (rtc->hours / 10));
    write_both(6, (rtc->hours % 10));

    write_both(5, 0xff);

    write_both(4, (rtc->minutes / 10));
    write_both(3, (rtc->minutes % 10));

    write_both(2, 0xff);

    write_both(1, (rtc->seconds / 10));
    write_both(0, (rtc->seconds % 10));
}

/**
 * @brief Main program entry point
 *
 * Initializes the controllers, plays the track
 * and then displays the state of the inputs on the lamp matrix
 * and echoes back any serial input
 */
int main(void) {
    int8_t menu_item=0;

    init_kdc();
    init_muart();

    rtc = (struct rtc_state *)0x9000;
    rtc_a = (uint8_t *)0x900a;
    rtc_b = (uint8_t *)0x900b;
    
    *rtc_b = 0x03;
    
    _8085_int7();
    enable_interrupts();

    bool buttonl = false;
    bool buttons = false;
    bool buttonr = false;
    bool buttonret = false;

    // Infinite loop to scan the keyboard
    while (1) {
        read_sensor_matrix();

        bool buttonl = check_button(RISK_LEFT);
        bool buttons = check_button(STOP_MID);
        bool buttonr = check_button(RISK_RIGHT);
        bool buttonret = check_button(RETURN);

        // Date edit mode handling
        if (date_edit_mode) {
            // Navigate between date digits (only valid digits: 7,6,4,3,1,0)
            if (buttonl) {
                do {
                    selected_digit++;
                    if (selected_digit > 7)
                        selected_digit = 7;
                } while (selected_digit == 5 || selected_digit == 2);  // Skip separator positions
                display_rtc_date();
                refresh_display();
                dumb_delay(200);
            } else if (buttonr) {
                do {
                    selected_digit--;
                    if (selected_digit < 0)
                        selected_digit = 0;
                } while (selected_digit == 5 || selected_digit == 2);  // Skip separator positions
                display_rtc_date();
                refresh_display();
                dumb_delay(200);
            }

            // Change selected digit value
            if (buttons) {
                switch (selected_digit) {
                    case 7:  // Day tens place
                        rtc->day_of_month = ((rtc->day_of_month / 10 + 1) % 3) * 10 + (rtc->day_of_month % 10);
                        if (rtc->day_of_month > 31)
                            rtc->day_of_month = 01;
                        break;
                    case 6:  // Day ones place
                        rtc->day_of_month = (rtc->day_of_month / 10) * 10 + ((rtc->day_of_month % 10 + 1) % 10);
                        if (rtc->day_of_month > 31)
                            rtc->day_of_month = (rtc->day_of_month / 10) * 10 + 0;
                        break;
                    case 4:  // Month tens place
                        rtc->month = ((rtc->month / 10 + 1) % 2) * 10 + (rtc->month % 10);
                        if (rtc->month > 12)
                            rtc->month = 01;
                        break;
                    case 3:  // Month ones place
                        rtc->month = (rtc->month / 10) * 10 + ((rtc->month % 10 + 1) % 10);
                        if (rtc->month > 12)
                            rtc->month = (rtc->month / 10) * 10 + 0;
                        break;
                    case 1:  // Year tens place
                        rtc->year = ((rtc->year / 10 + 1) % 10) * 10 + (rtc->year % 10);
                        break;
                    case 0:  // Year ones place
                        rtc->year = (rtc->year / 10) * 10 + ((rtc->year % 10 + 1) % 10);
                        break;
                }
                display_rtc_date();
                refresh_display();
                dumb_delay(200);
            }

            // Exit edit mode
            if (buttonret) {
                date_edit_mode = false;
                // Clamp date values to valid ranges
                if (rtc->day_of_month < 1) rtc->day_of_month = 1;
                if (rtc->day_of_month > 31) rtc->day_of_month = 31;
                if (rtc->month < 1) rtc->month = 1;
                if (rtc->month > 12) rtc->month = 12;
                if (rtc->year > 99) rtc->year = 99;
                *rtc_a = 0x21;
                selected_digit = -1;
                display_rtc_date();  // Refresh without blinking
                refresh_display();
            }
        } else if (time_edit_mode) {
            // Navigate between time digits (only valid digits: 7,6,4,3,1,0)
            if (buttonl) {
                do {
                    selected_digit++;
                    if (selected_digit > 7)
                        selected_digit = 7;
                } while (selected_digit == 5 || selected_digit == 2);  // Skip separator positions
                display_rtc_time();
                refresh_display();
                dumb_delay(200);
            } else if (buttonr) {
                do {
                    selected_digit--;
                    if (selected_digit < 0)
                        selected_digit = 0;
                } while (selected_digit == 5 || selected_digit == 2);  // Skip separator positions
                display_rtc_time();
                refresh_display();
                dumb_delay(200);
            }

            // Change selected digit value
            if (buttons) {
                switch (selected_digit) {
                    case 7:  // Hours tens place
                        rtc->hours = ((rtc->hours / 10 + 1) % 3) * 10 + (rtc->hours % 10);
                        if (rtc->hours > 23)
                            rtc->hours -= 20;
                        break;
                    case 6:  // Hours ones place
                        rtc->hours = (rtc->hours / 10) * 10 + ((rtc->hours % 10 + 1) % 10);
                        if (rtc->hours > 23)
                            rtc->hours = (rtc->hours / 10) * 10;
                        break;
                    case 4:  // Minutes tens place
                        rtc->minutes = ((rtc->minutes / 10 + 1) % 6) * 10 + (rtc->minutes % 10);
                        if (rtc->minutes > 59)
                            rtc->minutes = 00;
                        break;
                    case 3:  // Minutes ones place
                        rtc->minutes = (rtc->minutes / 10) * 10 + ((rtc->minutes % 10 + 1) % 10);
                        if (rtc->minutes > 59)
                            rtc->minutes = (rtc->minutes / 10) * 10;
                        break;
                    case 1:  // Seconds tens place
                        rtc->seconds = ((rtc->seconds / 10 + 1) % 6) * 10 + (rtc->seconds % 10);
                        if (rtc->seconds > 59)
                            rtc->seconds = 00;
                        break;
                    case 0:  // Seconds ones place
                        rtc->seconds = (rtc->seconds / 10) * 10 + ((rtc->seconds % 10 + 1) % 10);
                        if (rtc->seconds > 59)
                            rtc->seconds = (rtc->seconds / 10) * 10;
                        break;
                }
                display_rtc_time();
                refresh_display();
                dumb_delay(200);
            }

            // Exit edit mode
            if (buttonret) {
                time_edit_mode = false;
                // Clamp time values to valid ranges
                if (rtc->hours > 23) rtc->hours = 23;
                if (rtc->minutes > 59) rtc->minutes = 59;
                if (rtc->seconds > 59) rtc->seconds = 59;
                *rtc_a = 0x21;
                selected_digit = -1;
                display_rtc_time();  // Refresh without blinking
                refresh_display();
            }
        } else {
            write_serie(sensor_ram[menu_item]);
            write_both(0, menu_item);
            write_both(1, 0xff);
            write_both(2, 0xff);
            write_both(3, 0xff);
            write_both(4, 0xff);

            // Normal mode operations
            if (buttonl) {
                menu_item--;
                dumb_delay(200);
            } else if (buttonr) {
                menu_item++;
                dumb_delay(200);
            }

            if (buttonret) {
                menu_item = 0;
                dumb_delay(200);
            }

            if (buttons) {
                switch (menu_item) {
                    case 2:
                        date_edit_mode = true;
                        //*rtc_a = 0x70; //stop clock
                        selected_digit = 7;
                        display_rtc_date();
                        refresh_display();
                        break;
                    case 3:
                        time_edit_mode = true;
                        //*rtc_a = 0x70;
                        selected_digit = 7;
                        display_rtc_time();
                        refresh_display();
                        break;
                    case 5:
                        play_track();
                        break;
                    case 7:
                        write_lamps(0,0xff);
                        write_lamps(1,0xff);
                        write_lamps(2,0xff);
                        write_lamps(3,0xff);
                        write_lamps(4,0xff);
                        write_lamps(5,0xff);
                        write_lamps(6,0xff);
                        write_lamps(7,0xff);
                        break;
                }
                dumb_delay(200);
            }
        }

        if (menu_item < 0) {
            menu_item = 7;
        }

        if (menu_item >= 8) {
            menu_item = 0;
        }

        if(read_status() & I8256_STATUS_RBF) {
            uint8_t rcv = read_serial_char();
            print_serial_char(rcv);
        }
    }
}
