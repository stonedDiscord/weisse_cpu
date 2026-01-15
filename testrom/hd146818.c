#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>

struct rtc_state {
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

#endif
