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

// RAM bounds for the RAM size self-test.
//   RAM_BASE      - first RAM address (matches CRT_ORG_BSS)
//   RAM_SCAN_MAX  - one past the last address to probe (kept below any MMIO)
//   RAM_STACK_TOP - top of the stack (matches REGISTER_SP); probes near here
//                   are skipped so the test never clobbers its own stack frame
#if defined(BOARD4040)
#define RAM_BASE      0x5000
#define RAM_SCAN_MAX  0x6000   // RTC is mapped at 0x6000
#define RAM_STACK_TOP 0x53fc
#elif defined(BOARD4109)
#define RAM_BASE      0x9000
#define RAM_SCAN_MAX  0xA000
#define RAM_STACK_TOP 0x9ff0
#else // BOARD4087
#define RAM_BASE      0xc000
#define RAM_SCAN_MAX  0x10000
#define RAM_STACK_TOP 0xc7f0
#endif

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
void disable_interrupts();
void _8085_int1();
void _8085_int3();
void _8085_int5();
void _8085_int7();
void _8085_int65();
void read_sensor_matrix();
void calibrate_buttons();
void scan_buttons();
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
void print_hex8(uint8_t v);
void menu_8256_test();
void menu_8279_test();
void menu_ram_test();
void menu_rtc_test();
void play_note(uint8_t note, uint8_t octave, uint8_t duration);
void play_track();
bool check_button(uint8_t button);
bool check_button_edge(uint8_t button);
bool test_cancelled();
void display_rtc_date();
void display_rtc_time();

volatile struct rtc_state_t *rtc;

#define COINS       0x70
#define COUNTERS    0x71
#define SOUND       0x72

volatile bool timer3_flag = false;
volatile bool blink_flag = false;
bool date_edit_mode = false;
bool time_edit_mode = false;
int8_t selected_digit = -1;
int8_t menu_item = 0;

uint8_t sensor_ram[8];        // raw current sample
uint8_t sensor_prev[8];       // previous raw sample (for debounce)
uint8_t sensor_debounced[8];  // confirmed stable state
uint8_t sensor_baseline[8];   // boot "rest" state - presses are deviations from this
uint8_t pressed_prev[8];      // pressed state from previous scan (for edge detection)
uint8_t button_edge[8];       // bits that went pressed since last scan (rising edge)
uint8_t sensor_row = 0;

// Software blink: toggle blink_flag every BLINK_PERIOD main-loop iterations.
// Tune to taste - higher = slower blink. (Timer 5 ISR blink is not enabled.)
#define BLINK_PERIOD 25
uint16_t blink_counter = 0;

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

/**
 * @brief Disable Interrupts (8085 interrupt delivery off)
 */
void disable_interrupts() {
    __asm
        DI
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
 * @brief Sample the button matrix at boot and remember the rest state
 *
 * Reads the sensor matrix twice and stores the result as the baseline.
 * At runtime a button counts as pressed when its debounced state differs
 * from this baseline, so the wiring polarity (active-high vs active-low)
 * is detected automatically per machine/emulator layout.
 *
 * @note A button physically held during boot becomes its own baseline and
 *       reads as released until it is released and pressed again.
 */
void calibrate_buttons() {
    read_sensor_matrix();
    for (uint8_t row = 0; row < 8; row++) {
        sensor_prev[row] = sensor_ram[row];
    }
    dumb_delay(50);
    read_sensor_matrix();
    for (uint8_t row = 0; row < 8; row++) {
        sensor_baseline[row]  = sensor_ram[row];
        sensor_debounced[row] = sensor_ram[row];
        sensor_prev[row]      = sensor_ram[row];
        pressed_prev[row]     = 0;   // nothing pressed relative to baseline yet
        button_edge[row]      = 0;
    }
}

/**
 * @brief Sample the button matrix with software debouncing
 *
 * Only bits that read the same as the previous scan are committed to the
 * debounced state, so a button must be stable for two consecutive scans
 * before its change is registered.
 */
void scan_buttons() {
    read_sensor_matrix();
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t stable = ~(sensor_ram[row] ^ sensor_prev[row]); // bits unchanged this scan
        sensor_debounced[row] = (sensor_debounced[row] & ~stable)
                              | (sensor_ram[row]       &  stable);
        sensor_prev[row] = sensor_ram[row];

        // rising-edge: bits pressed now (vs baseline) that were not pressed last scan
        uint8_t pressed = sensor_debounced[row] ^ sensor_baseline[row];
        button_edge[row] = pressed & ~pressed_prev[row];
        pressed_prev[row] = pressed;
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
    uint16_t full_chunks = ms / 255;
    uint8_t remainder = ms % 255;

    // Wait for full 255ms chunks
    for (uint16_t i = 0; i < full_chunks; i++) {
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

/**
 * @brief Print a byte as two hex digits over the serial port
 *
 * @param v Byte to print
 */
void print_hex8(uint8_t v) {
    const char hex[] = "0123456789ABCDEF";
    print_serial_char(hex[(v >> 4) & 0x0F]);
    print_serial_char(hex[v & 0x0F]);
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
    uint8_t mask = 1 << (button & 0x0F);

    // Pressed = debounced state deviates from the boot baseline.
    // Polarity is handled by calibration, so the inverted bit is ignored.
    return ((sensor_debounced[row] ^ sensor_baseline[row]) & mask) != 0;
}

/**
 * @brief Check whether a button was just pressed (rising edge)
 *
 * Returns true for one scan only, on the transition from released to pressed.
 * Use this for menu navigation so a single physical press = a single step.
 *
 * @param button Button identifier (row, column, and inversion flag)
 * @return bool true on the scan where the button became pressed
 */
bool check_button_edge(uint8_t button) {
    uint8_t row = (button >> 4) & 0x07;
    uint8_t mask = 1 << (button & 0x0F);

    return (button_edge[row] & mask) != 0;
}

/**
 * @brief Poll the return button so long-running tests can be cancelled
 *
 * Rescans the button matrix and reports a fresh press of the return / INIT
 * button. Call this inside any blocking loop in a self-test so the user can
 * always bail out instead of having to power-cycle.
 *
 * @return bool true if the user just pressed return
 */
bool test_cancelled() {
    scan_buttons();
    return check_button_edge(INIT) || check_button_edge(RETURN);
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
 * @brief Menu option: 8256 MUART timer self-test
 *
 * Exercises Timer 3 of the 8256 in three ways and reports over the serial
 * port (and on the 7-segment display):
 *   1. Read-back: the timer must be readable and counting down.
 *   2. Frequency: counts per RTC second, expected ~0x400 (1.024 kHz mode).
 *   3. Interrupt: the Level 3 timer interrupt must fire (bounded by the RTC
 *      so this can never hang, unlike a bare wait_timer3).
 *
 * The display shows "8256" then the measured counts/second as 4 hex digits.
 */
void menu_8256_test() {
    // Label "8256" on the displays
    write_both(7, 8); write_both(6, 2); write_both(5, 5); write_both(4, 6);
    write_both(3, 0); write_both(2, 0); write_both(1, 0); write_both(0, 0);
    refresh_display();

    print_string("\n8256 timer test\n");

    // --- Parallel I/O test (before the timer tests) ---
    // Drive both ports fully as outputs and zero them, then flip both ports
    // to inputs and read them back. The read-back state is shown on the lamps
    // (Port 1 on lamp line 0, Port 2 on lamp line 1) and logged over serial.
    set_port1_control(0x00);                 // Port 1: all pins outputs
    set_muart_mode(I8256_MODE_PORT2C_OO);    // Port 2: both nibbles outputs
    set_port1(0x00);                         // zero the outputs
    set_port2(0x00);

    set_port1_control(0xFF);                 // Port 1: all pins inputs
    set_muart_mode(I8256_MODE_PORT2C_II);    // Port 2: both nibbles inputs
    uint8_t p1 = read_port1();
    uint8_t p2 = read_port2();
    write_lamps(0, p1);
    write_lamps(1, p2);

    print_string("parallel in: port1 "); print_hex8(p1);
    print_string(" port2 "); print_hex8(p2);
    print_serial_char('\n');

    // Restore the normal MUART port configuration for the rest of the ROM
    set_muart_mode(I8256_MODE_PORT2C_OO);
    set_port1_control(0x70);
    set_port2(0xFF);
    set_port1(0x30);

    // --- Test 1: timer is readable and counting down ---
    set_timer3(0xFF);
    uint8_t t0 = read_timer3();
    dumb_delay(30);
    uint8_t t1 = read_timer3();
    dumb_delay(30);
    uint8_t t2 = read_timer3();
    bool count_ok = ((uint8_t)(t0 - t1) != 0) && ((uint8_t)(t1 - t2) != 0);

    print_string("readback ");
    print_hex8(t0); print_serial_char(' ');
    print_hex8(t1); print_serial_char(' ');
    print_hex8(t2);
    print_string(count_ok ? "  countdown OK\n" : "  countdown FAIL\n");

    // --- Test 2: frequency vs RTC (counts per 1 second) ---
    uint8_t s = rtc_get_seconds();
    while (rtc_get_seconds() == s) {      // sync to a second edge
        if (test_cancelled()) { print_string("cancelled\n"); return; }
    }
    s = rtc_get_seconds();

    set_timer3(0xFF);
    uint8_t last = read_timer3();
    uint16_t counts = 0;
    while (rtc_get_seconds() == s) {
        uint8_t now = read_timer3();
        counts += (uint8_t)(last - now);     // 8-bit modular: handles wrap
        if (now < 0x10) {                    // keep it running (one-shot or auto-reload)
            set_timer3(0xFF);
            now = 0xFF;
        }
        last = now;
        if (test_cancelled()) { print_string("cancelled\n"); return; }
    }
    bool freq_ok = (counts > 0x320) && (counts < 0x500);  // ~0x400 +/- margin

    print_string("freq ~");
    print_hex8(counts >> 8); print_hex8(counts & 0xFF);
    print_string(" counts/s (expect ~0400) ");
    print_string(freq_ok ? "OK\n" : "FAIL\n");

    // Show the measured count on the displays as 4 hex digits
    write_both(3, (counts >> 12) & 0x0F);
    write_both(2, (counts >>  8) & 0x0F);
    write_both(1, (counts >>  4) & 0x0F);
    write_both(0,  counts        & 0x0F);
    refresh_display();

    // --- Test 3: timer raises its interrupt request (polled, cannot hang) ---
    // We deliberately do NOT enable 8085 interrupt delivery here. Enabling the
    // timer-3 interrupt and relying on the ISR (as wait_timer3 does) can storm
    // the CPU if the MUART's interrupt request is never acknowledged, which
    // starves the main loop and hangs the whole ROM regardless of any RTC
    // bound. Instead we arm L3 in the MUART mask only (no EI) and poll the
    // status INT bit, so a broken interrupt path reports FAIL instead of
    // hanging.
    disable_interrupts();                    // no 8085 vectoring while we poll
    arm_muart_interrupts(I8256_INT_L3);      // enable L3 in MUART mask, no EI
    set_timer3(200);                         // ~200 ms at 1.024 kHz
    uint8_t prev = rtc_get_seconds();
    uint8_t ticks = 0;
    bool int_ok = false;
    while (ticks < 2) {                      // give it up to ~2 RTC seconds
        if (read_status() & I8256_STATUS_INT) { int_ok = true; break; }
        if (test_cancelled()) {
            arm_muart_interrupts(0x00);      // clean up before bailing out
            enable_interrupts();
            print_string("cancelled\n");
            return;
        }
        uint8_t cur = rtc_get_seconds();
        if (cur != prev) { prev = cur; ticks++; }
    }
    arm_muart_interrupts(0x00);              // mask all MUART ints (clears pending delivery)
    enable_interrupts();                     // restore 8085 delivery for the other ISRs
    print_string(int_ok ? "interrupt OK\n" : "interrupt FAIL\n");

    print_string((count_ok && freq_ok && int_ok) ? "8256 PASS\n" : "8256 FAIL\n");

    // Hold the result on the display, but let return cut it short
    for (uint16_t i = 0; i < 3000 && !test_cancelled(); i++) {
        dumb_delay(1);
    }
}

/**
 * @brief Menu option: 8279 display-RAM self-test
 *
 * Saves the 16 bytes of display RAM, then writes several patterns (including
 * an address-unique pattern that catches stuck address lines) and reads them
 * back, plus an auto-increment burst. The original contents are restored and
 * the result is reported over serial and on the display. Cancelable with the
 * return button.
 */
void menu_8279_test() {
    print_string("\n8279 display RAM test\n");

    // Preserve the current display RAM so the test is non-destructive
    uint8_t saved[16];
    for (uint8_t a = 0; a < 16; a++) saved[a] = read_dram(a);

    bool ok = true;
    static const uint8_t patterns[4] = { 0x00, 0xFF, 0xAA, 0x55 };
    for (uint8_t p = 0; p < 4; p++) {
        // XOR with the address gives every cell a distinct value
        for (uint8_t a = 0; a < 16; a++) write_dram(a, patterns[p] ^ a);
        for (uint8_t a = 0; a < 16; a++) {
            uint8_t expect = patterns[p] ^ a;
            uint8_t rd = read_dram(a);
            if (rd != expect) {
                ok = false;
                print_string("addr "); print_hex8(a);
                print_string(" wrote "); print_hex8(expect);
                print_string(" read "); print_hex8(rd);
                print_serial_char('\n');
            }
        }
        if (test_cancelled()) {
            for (uint8_t a = 0; a < 16; a++) write_dram(a, saved[a]);
            refresh_display();
            print_string("cancelled\n");
            return;
        }
    }

    // Auto-increment write: one command, 16 sequential data writes
    kdc_cmd_out(I8279_WRITE_DISPLAY_RAM | I8279_RW_AUTO_INCREMENT | 0);
    for (uint8_t a = 0; a < 16; a++) kdc_data_out(0xF0 | a);
    for (uint8_t a = 0; a < 16; a++) {
        if (read_dram(a) != (uint8_t)(0xF0 | a)) ok = false;
    }

    // Restore the display
    for (uint8_t a = 0; a < 16; a++) write_dram(a, saved[a]);
    refresh_display();

    print_string(ok ? "8279 PASS\n" : "8279 FAIL\n");
    for (uint16_t i = 0; i < 2000 && !test_cancelled(); i++) dumb_delay(1);
}

/**
 * @brief Menu option: RAM size / end detection
 *
 * Probes upward from RAM_BASE in 256-byte steps. Each probe is non-destructive
 * (save, write 0xA5, read back, restore) and guarded by DI so an interrupt
 * cannot run while a byte is borrowed. A marker at RAM_BASE detects address
 * aliasing (mirrored RAM): if writing a high address changes the marker, the
 * real RAM has wrapped and we stop. Probes inside the live stack window are
 * skipped (assumed present) so the scan never corrupts its own frame.
 *
 * The detected top address and size are shown over serial and the size in KB
 * is shown on the display. Cancelable with the return button.
 */
void menu_ram_test() {
    print_string("\nRAM size test\n");

    volatile uint8_t* base = (volatile uint8_t*)RAM_BASE;

    disable_interrupts();
    uint8_t base_save = base[0];
    base[0] = 0x5A;                 // aliasing marker
    enable_interrupts();

    uint16_t top = RAM_BASE;        // highest address verified present
    for (uint32_t a = RAM_BASE + 0x100; a < (uint32_t)RAM_SCAN_MAX; a += 0x100) {
        uint16_t addr = (uint16_t)a;

        // Skip the live stack window so we never overwrite our own frame
        if (addr >= (uint16_t)(RAM_STACK_TOP - 0x100) && addr <= RAM_STACK_TOP) {
            top = addr;
            continue;
        }

        volatile uint8_t* p = (volatile uint8_t*)addr;
        disable_interrupts();
        uint8_t save = *p;
        *p = 0xA5;
        uint8_t rd = *p;
        bool alias = (base[0] != 0x5A);   // did this write disturb the marker?
        *p = save;
        enable_interrupts();

        if (rd != 0xA5 || alias) break;   // not RAM, or mirror of low RAM
        top = addr;

        if (test_cancelled()) { print_string("cancelled\n"); break; }
    }

    disable_interrupts();
    base[0] = base_save;
    enable_interrupts();

    uint16_t size = (top - RAM_BASE) + 0x100;   // rounded to the probe step
    uint8_t kb = (uint8_t)(size >> 10);

    print_string("base "); print_hex8(RAM_BASE >> 8); print_hex8(RAM_BASE & 0xFF);
    print_string(" top "); print_hex8(top >> 8); print_hex8(top & 0xFF);
    print_string(" size "); print_hex8(size >> 8); print_hex8(size & 0xFF);
    print_string(" ("); print_hex8(kb); print_string(" KB)\n");

    // Show the size in KB on the display (low two hex digits)
    write_both(7, 0x4); write_both(6, 0xa);   // crude "rA" label
    write_both(1, (kb >> 4) & 0x0F);
    write_both(0, kb & 0x0F);
    refresh_display();
    for (uint16_t i = 0; i < 2000 && !test_cancelled(); i++) dumb_delay(1);
}

/**
 * @brief Menu option: RTC seconds-advance self-test
 *
 * Confirms the real-time clock is actually running by waiting for the seconds
 * register to change, bounded by a generous timeout so a dead clock reports
 * FAIL instead of hanging. The 8256 timer tests rely on the RTC advancing, so
 * this isolates "is the RTC alive" from "is the timer alive". Cancelable.
 */
void menu_rtc_test() {
    print_string("\nRTC seconds-advance test\n");

    uint8_t s0 = rtc_get_seconds();
    bool ok = false;
    for (uint16_t i = 0; i < 1000; i++) {        // ~several seconds worst case
        if (rtc_get_seconds() != s0) { ok = true; break; }
        if (test_cancelled()) { print_string("cancelled\n"); return; }
        dumb_delay(30);
    }
    uint8_t s1 = rtc_get_seconds();

    print_string("seconds "); print_hex8(s0);
    print_string(" -> "); print_hex8(s1);
    print_serial_char('\n');
    print_string(ok ? "RTC PASS\n" : "RTC FAIL\n");

    write_both(1, (s1 >> 4) & 0x0F);
    write_both(0, s1 & 0x0F);
    refresh_display();
    for (uint16_t i = 0; i < 2000 && !test_cancelled(); i++) dumb_delay(1);
}

/**
 * @brief Handle normal mode menu selection
 */
void handle_normal_mode(bool buttonl, bool buttons, bool buttonr, bool buttonret) {
    // Bounds check BEFORE any access
    if (menu_item < 0) menu_item = 0;
    if (menu_item >= 12) menu_item = 11;
    
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
            case 8: menu_8256_test(); break;
            case 9: menu_8279_test(); break;
            case 10: menu_ram_test(); break;
            case 11: menu_rtc_test(); break;
        }
        dumb_delay(200);
    }

    // Wrap menu item
    if (menu_item < 0) menu_item = 11;
    if (menu_item >= 12) menu_item = 0;
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

    calibrate_buttons();    // Sample button rest state for auto polarity detection

    _8085_int7();           // Initialize timer5 for blinking
    enable_interrupts();     // Enable interrupts - but handlers are now minimal!

    write_lamps(0, 0x16);   // light up pressable buttons
    write_lamps(3, 0xc0);   // return

    // Infinite loop to scan the keyboard
    while (1) {
        // Sample + debounce buttons - no interrupt protection needed since ISR is minimal
        scan_buttons();

        // Edge-triggered: one step per physical press (fixes navigation double-stepping)
        #ifdef EMULATOR // current mame on master has the risk buttons reversed
        bool buttonl = check_button_edge(RUNTER01);
        bool buttons = check_button_edge(GEWINN);
        bool buttonr = check_button_edge(HOCH1);
        bool buttonret = check_button_edge(INIT);
        #else
        bool buttonl = check_button_edge(RUNTER01) | check_button_edge(RISK_LEFT);
        bool buttons = check_button_edge(GEWINN) | check_button_edge(STOP_MID);
        bool buttonr = check_button_edge(HOCH1) | check_button_edge(RISK_RIGHT);
        bool buttonret = check_button_edge(INIT) | check_button_edge(RETURN);
        #endif

        if (check_button_edge(HW_TEST)) {
            menu_play_music();
        }
        if (check_button_edge(DAUERLAUF)) {
            menu_edit_date();
        }
        if (check_button_edge(FOUL)) {
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

        // Software blink: timer5 ISR is not enabled, so drive blink_flag here
        if (++blink_counter >= BLINK_PERIOD) {
            blink_counter = 0;
            blink_flag = !blink_flag;
        }

        // Always refresh display every loop - blink_flag is used for effects
        refresh_display();
    }
}
