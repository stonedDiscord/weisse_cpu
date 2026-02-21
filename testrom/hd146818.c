#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>
#include <stdbool.h>

#ifndef RTC_ADD
#error "Please set the RTC_ADD base address before including this file."
#endif

/**
 * HD146818/MC146818 RTC Register Layout
 * 
 * This is the standard PC RTC chip with BCD-encoded registers.
 */

// Direct register access structure (maps to hardware registers)
struct rtc_regs_t {
    uint8_t seconds;       // 0x00: Seconds (BCD 0x00-0x59)
    uint8_t sec_alarm;     // 0x01: Seconds alarm
    uint8_t minutes;       // 0x02: Minutes (BCD 0x00-0x59)
    uint8_t min_alarm;     // 0x03: Minutes alarm
    uint8_t hours;         // 0x04: Hours (BCD 0x00-0x23 or 0x01-0x12)
    uint8_t hour_alarm;    // 0x05: Hours alarm
    uint8_t day_of_week;   // 0x06: Day of week (1-7)
    uint8_t day_of_month;  // 0x07: Day of month (BCD 0x01-0x31)
    uint8_t month;         // 0x08: Month (BCD 0x01-0x12)
    uint8_t year;          // 0x09: Year (BCD 0x00-0x99)
    uint8_t reg_a;         // 0x0A: Register A
    uint8_t reg_b;         // 0x0B: Register B
    uint8_t reg_c;         // 0x0C: Register C
    uint8_t reg_d;         // 0x0D: Register D
};

// Software state structure (same layout as hardware for HD146818)
struct rtc_state_t {
    uint8_t seconds;       // BCD: 0x00-0x59
    uint8_t sec_alarm;     // Alarm seconds
    uint8_t minutes;       // BCD: 0x00-0x59
    uint8_t min_alarm;     // Alarm minutes
    uint8_t hours;         // BCD: 0x00-0x23
    uint8_t hour_alarm;    // Alarm hours
    uint8_t day_of_week;   // 1-7
    uint8_t day_of_month;  // BCD: 0x01-0x31
    uint8_t month;         // BCD: 0x01-0x12
    uint8_t year;          // BCD: 0x00-0x99
};

// Register A bits
#define RTC_A_UIP 0x80  // Update in progress

// Register B bits
#define RTC_B_DS    0x01  // Daylight Savings Enable
#define RTC_B_24    0x02  // 24-hour format
#define RTC_B_DM    0x04  // Data Mode (0=BCD, 1=Binary)
#define RTC_B_SQ    0x08  // Square Wave Enable
#define RTC_B_UI    0x10  // Update-ended Interrupt Enable
#define RTC_B_AI    0x20  // Alarm Interrupt Enable
#define RTC_B_PI    0x40  // Periodic Interrupt Enable
#define RTC_B_SET   0x80  // Set flag (stops updates when set)

// Register addresses
#define RTC_REG_A_ADD (RTC_ADD + 0x0A)
#define RTC_REG_B_ADD (RTC_ADD + 0x0B)
#define RTC_REG_C_ADD (RTC_ADD + 0x0C)
#define RTC_REG_D_ADD (RTC_ADD + 0x0D)

// Compatibility defines
#define RTC_CTRL_D_ADD RTC_REG_A_ADD
#define RTC_CTRL_E_ADD RTC_REG_B_ADD
#define RTC_CTRL_F_ADD RTC_REG_C_ADD

#define RTC_CTRL_F_24H  RTC_B_24
#define RTC_CTRL_F_STOP RTC_B_SET

// For compatibility with existing code using rtc_a and rtc_b pointers
#define RTC_A_ADD RTC_REG_A_ADD
#define RTC_B_ADD RTC_REG_B_ADD

// Global RTC register pointer
volatile struct rtc_regs_t *rtc_regs = (struct rtc_regs_t *)RTC_ADD;

// ============================================================================
// High-level get/set functions for time values
// ============================================================================

/**
 * @brief Get seconds from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_seconds(void) {
    return rtc_regs->seconds;
}

/**
 * @brief Set seconds in RTC (BCD format 0x00-0x59)
 * @param value BCD seconds value
 */
static void rtc_set_seconds(uint8_t value) {
    rtc_regs->seconds = value;
}

/**
 * @brief Get minutes from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_minutes(void) {
    return rtc_regs->minutes;
}

/**
 * @brief Set minutes in RTC (BCD format 0x00-0x59)
 * @param value BCD minutes value
 */
static void rtc_set_minutes(uint8_t value) {
    rtc_regs->minutes = value;
}

/**
 * @brief Get hours from RTC (BCD format 0x00-0x23)
 */
static uint8_t rtc_get_hours(void) {
    return rtc_regs->hours;
}

/**
 * @brief Set hours in RTC (BCD format 0x00-0x23)
 * @param value BCD hours value
 */
static void rtc_set_hours(uint8_t value) {
    rtc_regs->hours = value;
}

/**
 * @brief Get day of month from RTC (BCD format 0x01-0x31)
 */
static uint8_t rtc_get_day(void) {
    return rtc_regs->day_of_month;
}

/**
 * @brief Set day of month in RTC (BCD format 0x01-0x31)
 * @param value BCD day value
 */
static void rtc_set_day(uint8_t value) {
    rtc_regs->day_of_month = value;
}

/**
 * @brief Get month from RTC (BCD format 0x01-0x12)
 */
static uint8_t rtc_get_month(void) {
    return rtc_regs->month;
}

/**
 * @brief Set month in RTC (BCD format 0x01-0x12)
 * @param value BCD month value
 */
static void rtc_set_month(uint8_t value) {
    rtc_regs->month = value;
}

/**
 * @brief Get year from RTC (BCD format 0x00-0x99)
 */
static uint8_t rtc_get_year(void) {
    return rtc_regs->year;
}

/**
 * @brief Set year in RTC (BCD format 0x00-0x99)
 * @param value BCD year value
 */
static void rtc_set_year(uint8_t value) {
    rtc_regs->year = value;
}

/**
 * @brief Get day of week from RTC (1-7)
 */
static uint8_t rtc_get_day_of_week(void) {
    return rtc_regs->day_of_week;
}

/**
 * @brief Set day of week in RTC (1-7)
 * @param value Day of week value
 */
static void rtc_set_day_of_week(uint8_t value) {
    rtc_regs->day_of_week = value;
}

/**
 * @brief Increment a BCD digit at given position
 * @param value Pointer to the BCD value
 * @param pos Position (0=ones, 1=tens)
 * @param max Maximum value for this position
 */
static void rtc_increment_bcd_digit(uint8_t *value, uint8_t pos, uint8_t max) {
    uint8_t tens = (*value >> 4) & 0xF;
    uint8_t ones = *value & 0xF;
    
    if (pos == 0) {
        ones = (ones + 1) % 10;
        uint8_t full = tens * 10 + ones;
        if (full > max) {
            ones = 0;
        }
        *value = (tens << 4) | ones;
    } else {
        tens = (tens + 1) % ((max / 10) + 1);
        *value = (tens << 4) | ones;
    }
}

// ============================================================================
// Increment functions for individual time components
// ============================================================================

/**
 * @brief Increment seconds ones digit (0-9)
 */
static void rtc_increment_seconds_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->seconds, 0, 59);
}

/**
 * @brief Increment seconds tens digit (0-5)
 */
static void rtc_increment_seconds_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->seconds, 1, 59);
}

/**
 * @brief Increment minutes ones digit (0-9)
 */
static void rtc_increment_minutes_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->minutes, 0, 59);
}

/**
 * @brief Increment minutes tens digit (0-5)
 */
static void rtc_increment_minutes_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->minutes, 1, 59);
}

/**
 * @brief Increment hours ones digit (0-9)
 */
static void rtc_increment_hours_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->hours, 0, 23);
}

/**
 * @brief Increment hours tens digit (0-2)
 */
static void rtc_increment_hours_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->hours, 1, 23);
}

/**
 * @brief Increment day ones digit (0-9)
 */
static void rtc_increment_day_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->day_of_month, 0, 31);
}

/**
 * @brief Increment day tens digit (0-3)
 */
static void rtc_increment_day_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->day_of_month, 1, 39);
}

/**
 * @brief Increment month ones digit (0-9)
 */
static void rtc_increment_month_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->month, 0, 12);
}

/**
 * @brief Increment month tens digit (0-1)
 */
static void rtc_increment_month_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->month, 1, 19);
}

/**
 * @brief Increment year ones digit (0-9)
 */
static void rtc_increment_year_ones(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->year, 0, 99);
}

/**
 * @brief Increment year tens digit (0-9)
 */
static void rtc_increment_year_tens(void) {
    rtc_increment_bcd_digit((uint8_t*)&rtc_regs->year, 1, 99);
}

/**
 * @brief Initialize HD146818 for normal operation
 * Sets 24-hour format and BCD mode
 */
static void rtc_init(void) {
    volatile uint8_t *reg_a = (uint8_t *)RTC_REG_A_ADD;
    volatile uint8_t *reg_b = (uint8_t *)RTC_REG_B_ADD;
    
    *reg_a = 0x21;  // 32kHz time base, divider for 1Hz
    *reg_b = RTC_B_DS | RTC_B_24;  // 24-hour format, BCD mode
}

#endif
