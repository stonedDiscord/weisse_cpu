#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>
#include <stdbool.h>

#ifndef RTC_ADD
#error "Please set the RTC_ADD base address before including this file."
#endif

struct rtc_state_t
{
    uint8_t seconds;
    uint8_t sec_alarm;
    uint8_t minutes;
    uint8_t min_alarm;
    uint8_t hours;
    uint8_t hour_alarm;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint8_t year;
};

#define RTC_A_UIP 0x80

#define RTC_B_DS    0x01
#define RTC_B_24    0x02
#define RTC_B_DM    0x04
#define RTC_B_SQ    0x08
#define RTC_B_UI    0x10
#define RTC_B_AI    0x20
#define RTC_B_PI    0x40
#define RTC_B_SET   0x80

#define RTC_A_ADD RTC_ADD + 0x0A
#define RTC_B_ADD RTC_ADD + 0x0B

#endif
