#ifndef HEADER_RTC
#define HEADER_RTC

#include <stdint.h>
#include <stdbool.h>

struct rtc_state_t {
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

struct rtc_a_t
{
    uint8_t rate_select: 4;
    uint8_t divider: 3;
    bool update_in_progress;
};

struct rtc_b_t
{
    bool dst;
    bool hour_format;
    bool data_mode;
    bool square_wave;
    bool update_ended_interrupt;
    bool alarm_interrupt;
    bool periodic_interrupt;
    bool set;
};

#define RTC_A_ADD RTC_ADD + 0x0A
#define RTC_B_ADD RTC_ADD + 0x0B



#endif
