#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>
#include <stdbool.h>

#ifndef RTC_ADD
#error "Please set the RTC_ADD base address before including this file."
#endif

/**
 * RTC62421 Register Layout
 * 
 * Address Register      Description
 * 0x00   1 seconds     Seconds ones (0-9)
 * 0x01   10 seconds    Seconds tens (0-5)
 * 0x02   1 minutes     Minutes ones (0-9)
 * 0x03   10 minutes    Minutes tens (0-5)
 * 0x04   1 hours       Hours ones (0-9)
 * 0x05   10 hours      Hours tens (0-2)
 * 0x06   1 days        Days ones (0-9)
 * 0x07   10 days       Days tens (0-3)
 * 0x08   1 months      Months ones (0-9)
 * 0x09   10 months     Months tens (0-1)
 * 0x0A   1 years       Years ones (0-9)
 * 0x0B   10 years      Years tens (0-9)
 * 0x0C   weeks         Day of week (0-6)
 * 0x0D   control D     Control register D
 * 0x0E   control E     Control register E
 * 0x0F   control F     Control register F
 *
 * Note: This RTC only has 4 data lines, so the upper nibble is ignored.
 */

// Direct register access structure (maps to hardware registers)
struct rtc_regs_t {
    uint8_t sec_ones;      // 0x00: Seconds ones
    uint8_t sec_tens;      // 0x01: Seconds tens
    uint8_t min_ones;      // 0x02: Minutes ones
    uint8_t min_tens;      // 0x03: Minutes tens
    uint8_t hour_ones;     // 0x04: Hours ones
    uint8_t hour_tens;     // 0x05: Hours tens
    uint8_t day_ones;      // 0x06: Days ones
    uint8_t day_tens;      // 0x07: Days tens
    uint8_t month_ones;    // 0x08: Months ones
    uint8_t month_tens;    // 0x09: Months tens
    uint8_t year_ones;     // 0x0A: Years ones
    uint8_t year_tens;     // 0x0B: Years tens
    uint8_t day_of_week;   // 0x0C: Day of week
    uint8_t ctrl_d;        // 0x0D: Control register D
    uint8_t ctrl_e;        // 0x0E: Control register E
    uint8_t ctrl_f;        // 0x0F: Control register F
};

// Software state structure (BCD format compatible with existing code)
struct rtc_state_t {
    uint8_t seconds;       // BCD: 0x00-0x59
    uint8_t sec_alarm;     // Not used by RTC62421
    uint8_t minutes;       // BCD: 0x00-0x59
    uint8_t min_alarm;     // Not used by RTC62421
    uint8_t hours;         // BCD: 0x00-0x23
    uint8_t hour_alarm;    // Not used by RTC62421
    uint8_t day_of_week;   // 0-6
    uint8_t day_of_month;  // BCD: 0x01-0x31
    uint8_t month;         // BCD: 0x01-0x12
    uint8_t year;          // BCD: 0x00-0x99
};

/**
 * Control register D bits (d3, d2, d1, d0)
 * 
 * d0 - HOLD:  Hold flag, when set time update is paused for reading
 * d1 - BUSY:  Busy flag, indicates RTC is updating
 * d2 - IRQ:   IRQ flag, indicates interrupt occurred
 * d3 - 30ADJ: 30 second adjustment, when set rounds seconds to nearest minute
 */
#define RTC_CTRL_D_HOLD   0x01  // d0: Hold time update for reading
#define RTC_CTRL_D_BUSY   0x02  // d1: Busy flag (read-only)
#define RTC_CTRL_D_IRQ    0x04  // d2: IRQ flag
#define RTC_CTRL_D_30ADJ  0x08  // d3: 30 second adjustment

/**
 * Control register E bits (d3, d2, d1, d0)
 * 
 * d0 - MASK:  Mask flag for interrupt
 * d1 - ITRPT: Interrupt mode select
 * d2 - T0:    Timer select bit 0
 * d3 - T1:    Timer select bit 1
 */
#define RTC_CTRL_E_MASK   0x01  // d0: Mask flag
#define RTC_CTRL_E_ITRPT  0x02  // d1: Interrupt mode
#define RTC_CTRL_E_T0     0x04  // d2: Timer select bit 0
#define RTC_CTRL_E_T1     0x08  // d3: Timer select bit 1

/**
 * Control register F bits (d3, d2, d1, d0)
 * 
 * d0 - RESET: Reset flag
 * d1 - STOP:  Stop counting when set
 * d2 - 24/12: 24-hour (1) or 12-hour (0) format
 * d3 - TEST:  Test mode (should be 0 for normal operation)
 */
#define RTC_CTRL_F_RESET  0x01  // d0: Reset
#define RTC_CTRL_F_STOP   0x02  // d1: Stop counting
#define RTC_CTRL_F_24H    0x04  // d2: 24-hour format
#define RTC_CTRL_F_TEST   0x08  // d3: Test mode

// Register addresses
#define RTC_CTRL_D_ADD (RTC_ADD + 0x0D)
#define RTC_CTRL_E_ADD (RTC_ADD + 0x0E)
#define RTC_CTRL_F_ADD (RTC_ADD + 0x0F)

// Compatibility defines for existing code that expects HD146818-like interface
#define RTC_A_UIP    RTC_CTRL_D_BUSY  // Map to BUSY flag

// These are kept for code compatibility but map to RTC62421 equivalents
#define RTC_B_DS     0x01  // Not directly applicable
#define RTC_B_24     RTC_CTRL_F_24H   // 24-hour format
#define RTC_B_DM     0x04  // BCD mode is always used
#define RTC_B_SQ     0x08  // Not directly applicable
#define RTC_B_UI     0x10  // Not directly applicable
#define RTC_B_AI     0x20  // Not directly applicable
#define RTC_B_PI     0x40  // Not directly applicable
#define RTC_B_SET    RTC_CTRL_F_STOP  // Map SET to STOP

// For compatibility with existing code using rtc_a and rtc_b pointers
#define RTC_A_ADD RTC_CTRL_D_ADD
#define RTC_B_ADD RTC_CTRL_F_ADD

// Global RTC register pointer
volatile struct rtc_regs_t *rtc_regs = (struct rtc_regs_t *)RTC_ADD;

/**
 * @brief Read BCD value from RTC62421 ones/tens register pair
 * @param ones_ptr Pointer to the ones register
 * @return Combined BCD value
 */
static uint8_t rtc_read_bcd(volatile uint8_t *ones_ptr) {
    uint8_t tens = *(ones_ptr + 1) & 0x0F;  // Tens register is next
    uint8_t ones = *ones_ptr & 0x0F;
    return (tens << 4) | ones;
}

/**
 * @brief Write BCD value to RTC62421 ones/tens register pair
 * @param ones_ptr Pointer to the ones register
 * @param value BCD value to write
 */
static void rtc_write_bcd(volatile uint8_t *ones_ptr, uint8_t value) {
    *ones_ptr = value & 0x0F;           // Write ones
    *(ones_ptr + 1) = (value >> 4) & 0x0F;  // Write tens
}

// ============================================================================
// High-level get/set functions for time values
// ============================================================================

/**
 * @brief Get seconds from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_seconds(void) {
    return rtc_read_bcd(&rtc_regs->sec_ones);
}

/**
 * @brief Set seconds in RTC (BCD format 0x00-0x59)
 * @param value BCD seconds value
 */
static void rtc_set_seconds(uint8_t value) {
    rtc_write_bcd(&rtc_regs->sec_ones, value);
}

/**
 * @brief Get minutes from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_minutes(void) {
    return rtc_read_bcd(&rtc_regs->min_ones);
}

/**
 * @brief Set minutes in RTC (BCD format 0x00-0x59)
 * @param value BCD minutes value
 */
static void rtc_set_minutes(uint8_t value) {
    rtc_write_bcd(&rtc_regs->min_ones, value);
}

/**
 * @brief Get hours from RTC (BCD format 0x00-0x23)
 */
static uint8_t rtc_get_hours(void) {
    return rtc_read_bcd(&rtc_regs->hour_ones);
}

/**
 * @brief Set hours in RTC (BCD format 0x00-0x23)
 * @param value BCD hours value
 */
static void rtc_set_hours(uint8_t value) {
    rtc_write_bcd(&rtc_regs->hour_ones, value);
}

/**
 * @brief Get day of month from RTC (BCD format 0x01-0x31)
 */
static uint8_t rtc_get_day(void) {
    return rtc_read_bcd(&rtc_regs->day_ones);
}

/**
 * @brief Set day of month in RTC (BCD format 0x01-0x31)
 * @param value BCD day value
 */
static void rtc_set_day(uint8_t value) {
    rtc_write_bcd(&rtc_regs->day_ones, value);
}

/**
 * @brief Get month from RTC (BCD format 0x01-0x12)
 */
static uint8_t rtc_get_month(void) {
    return rtc_read_bcd(&rtc_regs->month_ones);
}

/**
 * @brief Set month in RTC (BCD format 0x01-0x12)
 * @param value BCD month value
 */
static void rtc_set_month(uint8_t value) {
    rtc_write_bcd(&rtc_regs->month_ones, value);
}

/**
 * @brief Get year from RTC (BCD format 0x00-0x99)
 */
static uint8_t rtc_get_year(void) {
    return rtc_read_bcd(&rtc_regs->year_ones);
}

/**
 * @brief Set year in RTC (BCD format 0x00-0x99)
 * @param value BCD year value
 */
static void rtc_set_year(uint8_t value) {
    rtc_write_bcd(&rtc_regs->year_ones, value);
}

/**
 * @brief Get day of week from RTC (0-6)
 */
static uint8_t rtc_get_day_of_week(void) {
    return rtc_regs->day_of_week & 0x0F;
}

/**
 * @brief Set day of week in RTC (0-6)
 * @param value Day of week value
 */
static void rtc_set_day_of_week(uint8_t value) {
    rtc_regs->day_of_week = value & 0x0F;
}

/**
 * @brief Increment a BCD digit at given position in RTC register
 * @param ones_ptr Pointer to the ones register (tens is at ones_ptr+1)
 * @param pos Position (0=ones, 1=tens)
 * @param max Maximum value for this position
 */
static void rtc_increment_bcd_digit(volatile uint8_t *ones_ptr, uint8_t pos, uint8_t max) {
    uint8_t value = rtc_read_bcd(ones_ptr);
    uint8_t tens = (value >> 4) & 0xF;
    uint8_t ones = value & 0xF;
    
    if (pos == 0) {
        ones = (ones + 1) % 10;
        uint8_t full = tens * 10 + ones;
        if (full > max) {
            ones = 0;
        }
    } else {
        tens = (tens + 1) % ((max / 10) + 1);
    }
    rtc_write_bcd(ones_ptr, (tens << 4) | ones);
}

// ============================================================================
// Increment functions for individual time components
// ============================================================================

/**
 * @brief Increment seconds ones digit (0-9)
 */
static void rtc_increment_seconds_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->sec_ones, 0, 59);
}

/**
 * @brief Increment seconds tens digit (0-5)
 */
static void rtc_increment_seconds_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->sec_ones, 1, 59);
}

/**
 * @brief Increment minutes ones digit (0-9)
 */
static void rtc_increment_minutes_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->min_ones, 0, 59);
}

/**
 * @brief Increment minutes tens digit (0-5)
 */
static void rtc_increment_minutes_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->min_ones, 1, 59);
}

/**
 * @brief Increment hours ones digit (0-9)
 */
static void rtc_increment_hours_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->hour_ones, 0, 23);
}

/**
 * @brief Increment hours tens digit (0-2)
 */
static void rtc_increment_hours_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->hour_ones, 1, 23);
}

/**
 * @brief Increment day ones digit (0-9)
 */
static void rtc_increment_day_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->day_ones, 0, 31);
}

/**
 * @brief Increment day tens digit (0-3)
 */
static void rtc_increment_day_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->day_ones, 1, 39);
}

/**
 * @brief Increment month ones digit (0-9)
 */
static void rtc_increment_month_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->month_ones, 0, 12);
}

/**
 * @brief Increment month tens digit (0-1)
 */
static void rtc_increment_month_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->month_ones, 1, 19);
}

/**
 * @brief Increment year ones digit (0-9)
 */
static void rtc_increment_year_ones(void) {
    rtc_increment_bcd_digit(&rtc_regs->year_ones, 0, 99);
}

/**
 * @brief Increment year tens digit (0-9)
 */
static void rtc_increment_year_tens(void) {
    rtc_increment_bcd_digit(&rtc_regs->year_ones, 1, 99);
}

/**
 * @brief Initialize RTC62421 for normal operation
 * Sets 24-hour format and starts counting
 */
static void rtc_init(void) {
    volatile uint8_t *ctrl_f = (uint8_t *)RTC_CTRL_F_ADD;
    *ctrl_f = RTC_CTRL_F_24H;  // 24-hour format, not stopped, not in test mode
}

#endif
