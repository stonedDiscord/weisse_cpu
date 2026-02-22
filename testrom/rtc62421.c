#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>
#include <stdbool.h>

#ifndef RTC_IO
#error "Please set the RTC_IO base address before including this file."
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

// Register addresses (I/O port offsets) - calculated at compile time
#define RTC_REG_SEC_ONES     (RTC_IO + 0x00)
#define RTC_REG_SEC_TENS     (RTC_IO + 0x01)
#define RTC_REG_MIN_ONES     (RTC_IO + 0x02)
#define RTC_REG_MIN_TENS     (RTC_IO + 0x03)
#define RTC_REG_HOUR_ONES    (RTC_IO + 0x04)
#define RTC_REG_HOUR_TENS    (RTC_IO + 0x05)
#define RTC_REG_DAY_ONES     (RTC_IO + 0x06)
#define RTC_REG_DAY_TENS     (RTC_IO + 0x07)
#define RTC_REG_MONTH_ONES   (RTC_IO + 0x08)
#define RTC_REG_MONTH_TENS   (RTC_IO + 0x09)
#define RTC_REG_YEAR_ONES    (RTC_IO + 0x0A)
#define RTC_REG_YEAR_TENS    (RTC_IO + 0x0B)
#define RTC_REG_DAY_OF_WEEK  (RTC_IO + 0x0C)
#define RTC_REG_CTRL_D       (RTC_IO + 0x0D)
#define RTC_REG_CTRL_E       (RTC_IO + 0x0E)
#define RTC_REG_CTRL_F       (RTC_IO + 0x0F)

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
#define RTC_A_ADD RTC_REG_CTRL_D
#define RTC_B_ADD RTC_REG_CTRL_F

// ============================================================================
// Low-level I/O functions using inline assembly
// The port address is encoded directly in the IN/OUT instruction
// ============================================================================

// Control register D access
static uint8_t rtc_read_ctrl_d(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_CTRL_D
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_ctrl_d(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_CTRL_D
    __endasm;
}

// Control register F access
static uint8_t rtc_read_ctrl_f(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_CTRL_F
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_ctrl_f(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_CTRL_F
    __endasm;
}

// Seconds register functions
static uint8_t rtc_read_sec_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_SEC_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_sec_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_SEC_ONES
    __endasm;
}

static uint8_t rtc_read_sec_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_SEC_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_sec_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_SEC_TENS
    __endasm;
}

// Minutes register functions
static uint8_t rtc_read_min_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_MIN_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_min_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_MIN_ONES
    __endasm;
}

static uint8_t rtc_read_min_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_MIN_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_min_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_MIN_TENS
    __endasm;
}

// Hours register functions
static uint8_t rtc_read_hour_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_HOUR_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_hour_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_HOUR_ONES
    __endasm;
}

static uint8_t rtc_read_hour_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_HOUR_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_hour_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_HOUR_TENS
    __endasm;
}

// Day register functions
static uint8_t rtc_read_day_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_DAY_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_day_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_DAY_ONES
    __endasm;
}

static uint8_t rtc_read_day_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_DAY_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_day_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_DAY_TENS
    __endasm;
}

// Month register functions
static uint8_t rtc_read_month_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_MONTH_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_month_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_MONTH_ONES
    __endasm;
}

static uint8_t rtc_read_month_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_MONTH_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_month_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_MONTH_TENS
    __endasm;
}

// Year register functions
static uint8_t rtc_read_year_ones(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_YEAR_ONES
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_year_ones(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_YEAR_ONES
    __endasm;
}

static uint8_t rtc_read_year_tens(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_YEAR_TENS
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_year_tens(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_YEAR_TENS
    __endasm;
}

// Day of week register functions
static uint8_t rtc_read_day_of_week_reg(void) {
    uint8_t out;
    __asm
        POP HL
        IN RTC_REG_DAY_OF_WEEK
        MOV L, A
        PUSH HL
    __endasm;
    return out;
}

static void rtc_write_day_of_week_reg(uint8_t value) {
    value;
    __asm
        OUT RTC_REG_DAY_OF_WEEK
    __endasm;
}

// ============================================================================
// High-level BCD read/write functions
// ============================================================================

/**
 * @brief Read BCD value from ones/tens pair
 */
static uint8_t rtc_read_bcd_pair(uint8_t ones, uint8_t tens) {
    return ((tens & 0x0F) << 4) | (ones & 0x0F);
}

// ============================================================================
// High-level get/set functions for time values
// ============================================================================

/**
 * @brief Get seconds from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_seconds(void) {
    return rtc_read_bcd_pair(rtc_read_sec_ones(), rtc_read_sec_tens());
}

/**
 * @brief Set seconds in RTC (BCD format 0x00-0x59)
 * @param value BCD seconds value
 */
static void rtc_set_seconds(uint8_t value) {
    rtc_write_sec_ones(value & 0x0F);
    rtc_write_sec_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get minutes from RTC (BCD format 0x00-0x59)
 */
static uint8_t rtc_get_minutes(void) {
    return rtc_read_bcd_pair(rtc_read_min_ones(), rtc_read_min_tens());
}

/**
 * @brief Set minutes in RTC (BCD format 0x00-0x59)
 * @param value BCD minutes value
 */
static void rtc_set_minutes(uint8_t value) {
    rtc_write_min_ones(value & 0x0F);
    rtc_write_min_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get hours from RTC (BCD format 0x00-0x23)
 */
static uint8_t rtc_get_hours(void) {
    return rtc_read_bcd_pair(rtc_read_hour_ones(), rtc_read_hour_tens());
}

/**
 * @brief Set hours in RTC (BCD format 0x00-0x23)
 * @param value BCD hours value
 */
static void rtc_set_hours(uint8_t value) {
    rtc_write_hour_ones(value & 0x0F);
    rtc_write_hour_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get day of month from RTC (BCD format 0x01-0x31)
 */
static uint8_t rtc_get_day(void) {
    return rtc_read_bcd_pair(rtc_read_day_ones(), rtc_read_day_tens());
}

/**
 * @brief Set day of month in RTC (BCD format 0x01-0x31)
 * @param value BCD day value
 */
static void rtc_set_day(uint8_t value) {
    rtc_write_day_ones(value & 0x0F);
    rtc_write_day_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get month from RTC (BCD format 0x01-0x12)
 */
static uint8_t rtc_get_month(void) {
    return rtc_read_bcd_pair(rtc_read_month_ones(), rtc_read_month_tens());
}

/**
 * @brief Set month in RTC (BCD format 0x01-0x12)
 * @param value BCD month value
 */
static void rtc_set_month(uint8_t value) {
    rtc_write_month_ones(value & 0x0F);
    rtc_write_month_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get year from RTC (BCD format 0x00-0x99)
 */
static uint8_t rtc_get_year(void) {
    return rtc_read_bcd_pair(rtc_read_year_ones(), rtc_read_year_tens());
}

/**
 * @brief Set year in RTC (BCD format 0x00-0x99)
 * @param value BCD year value
 */
static void rtc_set_year(uint8_t value) {
    rtc_write_year_ones(value & 0x0F);
    rtc_write_year_tens((value >> 4) & 0x0F);
}

/**
 * @brief Get day of week from RTC (0-6)
 */
static uint8_t rtc_get_day_of_week(void) {
    return rtc_read_day_of_week_reg() & 0x0F;
}

/**
 * @brief Set day of week in RTC (0-6)
 * @param value Day of week value
 */
static void rtc_set_day_of_week(uint8_t value) {
    rtc_write_day_of_week_reg(value & 0x0F);
}

// ============================================================================
// Increment functions for individual time components
// ============================================================================

/**
 * @brief Increment a BCD digit at given position
 * @param current_bcd Current BCD value
 * @param pos Position (0=ones, 1=tens)
 * @param max Maximum value for this field
 * @return New BCD value
 */
static uint8_t rtc_increment_bcd_digit_value(uint8_t current_bcd, uint8_t pos, uint8_t max) {
    uint8_t tens = (current_bcd >> 4) & 0xF;
    uint8_t ones = current_bcd & 0xF;
    
    if (pos == 0) {
        ones = (ones + 1) % 10;
        uint8_t full = tens * 10 + ones;
        if (full > max) {
            ones = 0;
        }
    } else {
        tens = (tens + 1) % ((max / 10) + 1);
    }
    return (tens << 4) | ones;
}

/**
 * @brief Increment seconds ones digit (0-9)
 */
static void rtc_increment_seconds_ones(void) {
    uint8_t val = rtc_get_seconds();
    val = rtc_increment_bcd_digit_value(val, 0, 59);
    rtc_set_seconds(val);
}

/**
 * @brief Increment seconds tens digit (0-5)
 */
static void rtc_increment_seconds_tens(void) {
    uint8_t val = rtc_get_seconds();
    val = rtc_increment_bcd_digit_value(val, 1, 59);
    rtc_set_seconds(val);
}

/**
 * @brief Increment minutes ones digit (0-9)
 */
static void rtc_increment_minutes_ones(void) {
    uint8_t val = rtc_get_minutes();
    val = rtc_increment_bcd_digit_value(val, 0, 59);
    rtc_set_minutes(val);
}

/**
 * @brief Increment minutes tens digit (0-5)
 */
static void rtc_increment_minutes_tens(void) {
    uint8_t val = rtc_get_minutes();
    val = rtc_increment_bcd_digit_value(val, 1, 59);
    rtc_set_minutes(val);
}

/**
 * @brief Increment hours ones digit (0-9)
 */
static void rtc_increment_hours_ones(void) {
    uint8_t val = rtc_get_hours();
    val = rtc_increment_bcd_digit_value(val, 0, 23);
    rtc_set_hours(val);
}

/**
 * @brief Increment hours tens digit (0-2)
 */
static void rtc_increment_hours_tens(void) {
    uint8_t val = rtc_get_hours();
    val = rtc_increment_bcd_digit_value(val, 1, 23);
    rtc_set_hours(val);
}

/**
 * @brief Increment day ones digit (0-9)
 */
static void rtc_increment_day_ones(void) {
    uint8_t val = rtc_get_day();
    val = rtc_increment_bcd_digit_value(val, 0, 31);
    rtc_set_day(val);
}

/**
 * @brief Increment day tens digit (0-3)
 */
static void rtc_increment_day_tens(void) {
    uint8_t val = rtc_get_day();
    val = rtc_increment_bcd_digit_value(val, 1, 39);
    rtc_set_day(val);
}

/**
 * @brief Increment month ones digit (0-9)
 */
static void rtc_increment_month_ones(void) {
    uint8_t val = rtc_get_month();
    val = rtc_increment_bcd_digit_value(val, 0, 12);
    rtc_set_month(val);
}

/**
 * @brief Increment month tens digit (0-1)
 */
static void rtc_increment_month_tens(void) {
    uint8_t val = rtc_get_month();
    val = rtc_increment_bcd_digit_value(val, 1, 19);
    rtc_set_month(val);
}

/**
 * @brief Increment year ones digit (0-9)
 */
static void rtc_increment_year_ones(void) {
    uint8_t val = rtc_get_year();
    val = rtc_increment_bcd_digit_value(val, 0, 99);
    rtc_set_year(val);
}

/**
 * @brief Increment year tens digit (0-9)
 */
static void rtc_increment_year_tens(void) {
    uint8_t val = rtc_get_year();
    val = rtc_increment_bcd_digit_value(val, 1, 99);
    rtc_set_year(val);
}

/**
 * @brief Initialize RTC62421 for normal operation
 * Sets 24-hour format and starts counting
 */
static void rtc_init(void) {
    rtc_write_ctrl_f(RTC_CTRL_F_24H);  // 24-hour format, not stopped, not in test mode
}

#endif
