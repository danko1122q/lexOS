#ifndef RTC_H
#define RTC_H

#include "../../include/types.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

void rtc_init(void);
void rtc_get_time(rtc_time_t *time);

#endif
