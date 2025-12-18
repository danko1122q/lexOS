#include "rtc.h"
#include "../../include/kernel.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

#define RTC_SECONDS   0x00
#define RTC_MINUTES   0x02
#define RTC_HOURS     0x04
#define RTC_DAY       0x07
#define RTC_MONTH     0x08
#define RTC_YEAR      0x09
#define RTC_STATUS_A  0x0A
#define RTC_STATUS_B  0x0B

static uint8_t bcd_to_binary(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t read_cmos(uint8_t reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

static int is_updating(void) {
    outb(CMOS_ADDR, RTC_STATUS_A);
    return (inb(CMOS_DATA) & 0x80);
}

void rtc_init(void) {
}

void rtc_get_time(rtc_time_t *time) {
    while (is_updating());
    
    time->second = read_cmos(RTC_SECONDS);
    time->minute = read_cmos(RTC_MINUTES);
    time->hour   = read_cmos(RTC_HOURS);
    time->day    = read_cmos(RTC_DAY);
    time->month  = read_cmos(RTC_MONTH);
    time->year   = read_cmos(RTC_YEAR);
    
    uint8_t status_b = read_cmos(RTC_STATUS_B);
    
    if (!(status_b & 0x04)) {
        time->second = bcd_to_binary(time->second);
        time->minute = bcd_to_binary(time->minute);
        time->hour   = bcd_to_binary(time->hour & 0x7F) | (time->hour & 0x80);
        time->day    = bcd_to_binary(time->day);
        time->month  = bcd_to_binary(time->month);
        time->year   = bcd_to_binary(time->year);
    }
    
    time->year += 2000;
}
