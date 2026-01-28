#include "dst.h"

static bool is_last_sunday(uint8_t day, uint8_t month, uint16_t year) {
    // A day is the last Sunday if it is Sunday (0) and adding 7 days exceeds the month's days
    static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t max_days = days_in_month[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        max_days = 29;
    }
    return (day + 7 > max_days);
}

void DST_CheckAndAdjust(DateTime_t *time) {
    // UK DST: Last Sunday in March at 01:00 UTC -> 02:00 BST
    if (time->month == 3 && time->weekday == 0 && is_last_sunday(time->day, time->month, time->year)) {
        if (time->hours == 1 && !time->is_dst) {
            time->hours = 2;
            time->is_dst = true;
        }
    }
    
    // UK DST: Last Sunday in October at 02:00 BST -> 01:00 UTC
    if (time->month == 10 && time->weekday == 0 && is_last_sunday(time->day, time->month, time->year)) {
        if (time->hours == 2 && time->is_dst) {
            time->hours = 1;
            time->is_dst = false;
        }
    }
}

