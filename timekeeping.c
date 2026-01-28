#include "timekeeping.h"

DateTime_t current_time;

static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Helper to calculate weekday (0=Sunday)
static uint8_t calculate_weekday(uint8_t d, uint8_t m, uint16_t y) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

void Timekeeping_Init(uint8_t h, uint8_t m, uint8_t d, uint8_t mon, uint16_t y) {
    current_time.hours = h;
    current_time.minutes = m;
    current_time.seconds = 0;
    current_time.day = d;
    current_time.month = mon;
    current_time.year = y;
    current_time.weekday = calculate_weekday(d, mon, y);
}

bool Timekeeping_IsLeapYear(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void Timekeeping_Tick(void) {
    current_time.seconds++;
    if (current_time.seconds >= 60) {
        current_time.seconds = 0;
        current_time.minutes++;
        if (current_time.minutes >= 60) {
            current_time.minutes = 0;
            current_time.hours++;
            if (current_time.hours >= 24) {
                current_time.hours = 0;
                current_time.day++;
                
                uint8_t max_days = days_in_month[current_time.month - 1];
                if (current_time.month == 2 && Timekeeping_IsLeapYear(current_time.year)) {
                    max_days = 29;
                }
                
                if (current_time.day > max_days) {
                    current_time.day = 1;
                    current_time.month++;
                    if (current_time.month > 12) {
                        current_time.month = 1;
                        current_time.year++;
                    }
                }
                current_time.weekday = calculate_weekday(current_time.day, current_time.month, current_time.year);
            }
        }
    }
}

void Timekeeping_AdjustHours(int8_t offset) {
    int16_t new_hours = (int16_t)current_time.hours + offset;
    if (new_hours >= 24) {
        current_time.hours = new_hours - 24;
        // Simplified: assuming we don't cross day boundaries for small adjustments
        // but for DST it might. For now, just handle the hour wrap.
    } else if (new_hours < 0) {
        current_time.hours = new_hours + 24;
    } else {
        current_time.hours = (uint8_t)new_hours;
    }
}

