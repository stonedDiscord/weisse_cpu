/**
 * @file main.c
 * @brief Main program for adp Steuereinheit
 *
 * This program interacts with a type of German slot machine in use from the late 1980s-early 1990s
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

//#define EMULATOR

#if defined(BOARD4040)
#define I8279_IO    0x80
#define I8256_IO    0x90
#define RTC_ADD     0x6000
#pragma output REGISTER_SP = 0x53fc
#pragma output CRT_ORG_BSS = 0x5000
#include "hd146818.c"
#elif defined(BOARD4109)
#define I8279_IO    0x50
#define I8256_IO    0x60
#define RTC_IO      0x00
#pragma output REGISTER_SP = 0x9ff0
#pragma output CRT_ORG_BSS = 0x9000
#include "rtc62421.c"
#else // BOARD4087
#define I8279_IO    0x50
#define I8256_IO    0x60
#define RTC_ADD     0x9000
#pragma output REGISTER_SP = 0xc7f0
#pragma output CRT_ORG_BSS = 0xc000
#include "hd146818.c"
#endif

#include "8279.c"
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
void init_ppi();
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

volatile struct rtc_state_t *rtc;

#define COINS       0x70
#define COUNTERS    0x71
#define SOUND       0x72

bool timer3_flag = false;
bool blink_flag = false;
bool date_edit_mode = false;
bool time_edit_mode = false;
int8_t selected_digit = -1;
int8_t menu_item = 0;

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
    // unused
}
// timer3
void _8085_int3() {
    timer3_flag = false;
}
// tx int
void _8085_int5() {
    // unused
}
//timer5
void _8085_int7() {
    blink_flag = !blink_flag;
    // Keep this minimal - only hardware register operations
    set_timer5(250);
}

/**
 * @brief Keyboard/display controller interrupt handler RST65
 *
 * Currently a placeholder function.
 */
void _8085_int65() {
    //read_sensor_matrix();
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
    //refresh_display();
}

/**
 * @brief Sound interrupt handler RST55
 *
 * Currently a placeholder function.
 */
void _8085_int55() {
    // unused
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
    if (date_edit_mode)
        display_rtc_date();

    if (time_edit_mode)
        display_rtc_time();

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
 * @brief Initialize the 8255 PPI
 * 
 */
void init_ppi() {
    __asm
        MVI A, 0x80
        OUT 0x73
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
        dumb_delay(1);
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
    dumb_delay(1);
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
        refresh_display();
        // Print lyric to serial port
        if (track[i].lyric != 0)
        {
            const char *lyric = track[i].lyric;
            print_string(lyric);
            print_string(" ");
        }

        dumb_delay(track[i].length * 3);
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

// Servicetastatur
#define RESET      BUTTON(6, 7, 0)
#define DAUERLAUF  BUTTON(6, 6, 0)
#define SPIELZ     BUTTON(6, 5, 0)
#define SPEICHER   BUTTON(6, 4, 0)
#define HW_TEST    BUTTON(6, 3, 0)
#define AZQ        BUTTON(6, 2, 0)
#define FOUL       BUTTON(6, 1, 0)
#define GEWINN     BUTTON(6, 0, 0)

#define HOCH1      BUTTON(7, 7, 0)
#define RUNTER1    BUTTON(7, 6, 0)
#define HOCHS      BUTTON(7, 5, 0)
#define RUNTERS    BUTTON(7, 4, 0)
#define HOCH01     BUTTON(7, 3, 0)
#define RUNTER01   BUTTON(7, 2, 0)
#define MUENZ      BUTTON(7, 1, 0)
#define INIT       BUTTON(7, 0, 0)

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
    uint8_t day = rtc_get_day();
    uint8_t month = rtc_get_month();
    uint8_t year = rtc_get_year();
    
    write_both(7, (day >> 4) & 0xF);
    write_both(6, day & 0xF);

    write_both(5, 0xff);

    write_both(4, (month >> 4) & 0xF);
    write_both(3, month & 0xF);

    write_both(2, 0xff);

    write_both(1, (year >> 4) & 0xF);
    write_both(0, year & 0xF);
}

// Function to display the time from the RTC
void display_rtc_time()
{
    uint8_t hours = rtc_get_hours();
    uint8_t minutes = rtc_get_minutes();
    uint8_t seconds = rtc_get_seconds();
    
    write_both(7, (hours >> 4) & 0xF);
    write_both(6, hours & 0xF);

    write_both(5, 0xff);

    write_both(4, (minutes >> 4) & 0xF);
    write_both(3, minutes & 0xF);

    write_both(2, 0xff);

    write_both(1, (seconds >> 4) & 0xF);
    write_both(0, seconds & 0xF);
}

/**
 * @brief Navigate to next valid digit (skip separator positions 2 and 5)
 */
void navigate_digit_next() {
    do {
        selected_digit++;
        if (selected_digit > 7)
            selected_digit = 7;
    } while (selected_digit == 5 || selected_digit == 2);
}

/**
 * @brief Navigate to previous valid digit (skip separator positions 2 and 5)
 */
void navigate_digit_prev() {
    do {
        selected_digit--;
        if (selected_digit < 0)
            selected_digit = 0;
    } while (selected_digit == 5 || selected_digit == 2);
}

/**
 * @brief Handle date edit mode
 */
void handle_date_edit_mode(bool buttonl, bool buttons, bool buttonr, bool buttonret) {
    if (buttonl) {
        navigate_digit_next();
        display_rtc_date();
        refresh_display();
        dumb_delay(200);
    } else if (buttonr) {
        navigate_digit_prev();
        display_rtc_date();
        refresh_display();
        dumb_delay(200);
    }

    if (buttons) {
        switch (selected_digit) {
            case 7: rtc_increment_day_tens(); break;
            case 6: rtc_increment_day_ones(); break;
            case 4: rtc_increment_month_tens(); break;
            case 3: rtc_increment_month_ones(); break;
            case 1: rtc_increment_year_tens(); break;
            case 0: rtc_increment_year_ones(); break;
        }
        display_rtc_date();
        refresh_display();
        dumb_delay(200);
    }

    if (buttonret) {
        date_edit_mode = false;
        // Clamp date values
        uint8_t day = rtc_get_day();
        uint8_t full_day = ((day >> 4) & 0xF) * 10 + (day & 0xF);
        if (full_day < 1) rtc_set_day(0x01);
        if (full_day > 31) rtc_set_day(0x31);
        
        uint8_t month = rtc_get_month();
        uint8_t full_month = ((month >> 4) & 0xF) * 10 + (month & 0xF);
        if (full_month < 1) rtc_set_month(0x01);
        if (full_month > 12) rtc_set_month(0x12);
        
        selected_digit = -1;
        display_rtc_date();
        refresh_display();
    }
}

/**
 * @brief Handle time edit mode
 */
void handle_time_edit_mode(bool buttonl, bool buttons, bool buttonr, bool buttonret) {
    if (buttonl) {
        navigate_digit_next();
        display_rtc_time();
        refresh_display();
        dumb_delay(200);
    } else if (buttonr) {
        navigate_digit_prev();
        display_rtc_time();
        refresh_display();
        dumb_delay(200);
    }

    if (buttons) {
        switch (selected_digit) {
            case 7: rtc_increment_hours_tens(); break;
            case 6: rtc_increment_hours_ones(); break;
            case 4: rtc_increment_minutes_tens(); break;
            case 3: rtc_increment_minutes_ones(); break;
            case 1: rtc_increment_seconds_tens(); break;
            case 0: rtc_increment_seconds_ones(); break;
        }
        display_rtc_time();
        refresh_display();
        dumb_delay(200);
    }

    if (buttonret) {
        time_edit_mode = false;
        // Clamp time values
        uint8_t hours = rtc_get_hours();
        uint8_t full_hours = ((hours >> 4) & 0xF) * 10 + (hours & 0xF);
        if (full_hours > 23) rtc_set_hours(0x23);
        
        uint8_t minutes = rtc_get_minutes();
        uint8_t full_min = ((minutes >> 4) & 0xF) * 10 + (minutes & 0xF);
        if (full_min > 59) rtc_set_minutes(0x59);
        
        uint8_t seconds = rtc_get_seconds();
        uint8_t full_sec = ((seconds >> 4) & 0xF) * 10 + (seconds & 0xF);
        if (full_sec > 59) rtc_set_seconds(0x59);
        
        selected_digit = -1;
        display_rtc_time();
        refresh_display();
    }
}

/**
 * @brief Menu option: Reset
 */
void menu_reset() {
    __asm
        RST 0
    __endasm
}

/**
 * @brief Menu option: Clear all lamps
 */
void menu_clear_lamps() {
    for (uint8_t i = 0; i < 8; i++) {
        write_lamps(i, 0x00);
    }
}

/**
 * @brief Menu option: Enter date edit mode
 */
void menu_edit_date() {
    date_edit_mode = true;
    selected_digit = 7;
    display_rtc_date();
    refresh_display();
}

/**
 * @brief Menu option: Enter time edit mode
 */
void menu_edit_time() {
    time_edit_mode = true;
    selected_digit = 7;
    display_rtc_time();
    refresh_display();
}

/**
 * @brief Menu option: Play music track
 */
void menu_play_music() {
    play_track();
}

/**
 * @brief Menu option: Lamp test pattern
 */
void menu_lamp_test() {
    for (uint8_t j = 0; j < 16; j++) {
        for (uint8_t k = 0; k < 8; k++) {
            write_lamps(j, 1 << k);
            dumb_delay(200);
        }
    }
}

/**
 * @brief Menu option: All lamps on
 */
void menu_all_lamps_on() {
    for (uint8_t i = 0; i < 8; i++) {
        write_lamps(i, 0xff);
    }
}

/**
 * @brief Handle normal mode menu selection
 */
void handle_normal_mode(bool buttonl, bool buttons, bool buttonr, bool buttonret) {
    // Bounds check BEFORE any access
    if (menu_item < 0) menu_item = 0;
    if (menu_item >= 8) menu_item = 7;
    
    write_both(0, menu_item);
    write_both(1, buttonl);
    write_both(2, buttons);
    write_both(3, buttonr);
    write_both(4, buttonret);

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
            //case 0: menu_reset(); break;
            case 1: menu_all_lamps_on(); break;
            case 2: menu_edit_date(); break;
            case 3: menu_play_music(); break;
            case 4: menu_edit_time(); break;
            case 5: menu_clear_lamps(); break;
            case 6: menu_lamp_test(); break;
            case 7: menu_all_lamps_on(); break;
        }
        dumb_delay(200);
    }

    // Wrap menu item
    if (menu_item < 0) menu_item = 7;
    if (menu_item >= 8) menu_item = 0;
}

/**
 * @brief Main program entry point
 *
 * Initializes the controllers, plays the track
 * and then displays the state of the inputs on the lamp matrix
 * and echoes back any serial input
 */
int main(void) {
    init_kdc();
    init_muart();
    init_ppi();

#ifdef BOARD4109
    // RTC62421 is I/O mapped, not memory mapped - no pointer needed
#else
    rtc = (struct rtc_state_t *)RTC_ADD;
#endif
    rtc_init();  // Initialize RTC (24-hour format, start counting)

    print_string("Test ROM Initialized\n");

    _8085_int7();           // Initialize timer5 for blinking
    enable_interrupts();     // Enable interrupts - but handlers are now minimal!

    write_lamps(0, 0x16);   // light up pressable buttons
    write_lamps(3, 0xc0);   // return

    // Infinite loop to scan the keyboard
    while (1) {
        // Read sensor matrix - no interrupt protection needed now since ISR is minimal
        read_sensor_matrix();

        #ifdef EMULATOR // current mame on master has the risk buttons reversed
        bool buttonl = check_button(RUNTER01);
        bool buttons = check_button(GEWINN);
        bool buttonr = check_button(HOCH1);
        bool buttonret = check_button(INIT);
        #else
        bool buttonl = check_button(RUNTER01) | check_button(RISK_LEFT);
        bool buttons = check_button(GEWINN) | check_button(STOP_MID);
        bool buttonr = check_button(HOCH1) | check_button(RISK_RIGHT);
        bool buttonret = check_button(INIT) | check_button(RETURN);
        #endif

        if (check_button(HW_TEST)) {
            menu_play_music();
        }
        if (check_button(DAUERLAUF)) {
            menu_edit_date();
        }
        if (check_button(FOUL)) {
            menu_edit_time();
        }

        // Mode handling
        if (date_edit_mode) {
            handle_date_edit_mode(buttonl, buttons, buttonr, buttonret);
        } else if (time_edit_mode) {
            handle_time_edit_mode(buttonl, buttons, buttonr, buttonret);
        } else {
            handle_normal_mode(buttonl, buttons, buttonr, buttonret);
        }

        if (read_status() & I8256_STATUS_RBF) {
            uint8_t rcv = read_serial_char();
            print_serial_char(rcv);
        }

        // Always refresh display every loop - blink_flag is used for effects
        refresh_display();
    }
}
