/*******************************************************************************
 * File:   Calendar.c
 * Purpose: Internal calendar with leap year and UK DST (BST) support.
 *          UK DST: BST starts last Sunday March, ends last Sunday October.
 ******************************************************************************/

#include "Calendar.h"

static uint16_t s_year = 2026;
static uint8_t s_month = 1;
static uint8_t s_day = 1;

/* Days in each month (non-leap). Index 0 = January. */
static const uint8_t DAYS_IN_MONTH[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint8_t Calendar_IsLeapYear(uint16_t y) {
    if (y % 4 != 0) return 0;
    if (y % 100 != 0) return 1;
    if (y % 400 == 0) return 1;
    return 0;
}

/* Tomohiko Sakamoto: day of week. Returns 0=Sun, 1=Mon, ..., 6=Sat */
static uint8_t DayOfWeek(uint16_t y, uint8_t m, uint8_t d) {
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) {
        y--;
    }
    return (uint8_t)((y + y/4 - y/100 + y/400 + t[m - 1] + d) % 7);
}

/* Get last day of month (28, 29, 30, or 31). */
static uint8_t LastDayOfMonth(uint16_t y, uint8_t m) {
    if (m != 2) {
        return DAYS_IN_MONTH[m - 1];
    }
    return (uint8_t)(28 + Calendar_IsLeapYear(y));
}

/* Get day number of last Sunday in given month. */
static uint8_t LastSundayOfMonth(uint16_t y, uint8_t m) {
    uint8_t last = LastDayOfMonth(y, m);
    return last - DayOfWeek(y, m, last);
}

void Calendar_Init(uint16_t year, uint8_t month, uint8_t day) {
    s_year = year;
    s_month = (month >= 1 && month <= 12) ? month : 1;
    s_day = day;
}

void Calendar_AdvanceDay(void) {
    uint8_t last = LastDayOfMonth(s_year, s_month);

    if (s_day >= last) {
        s_day = 1;
        if (s_month >= 12) {
            s_month = 1;
            s_year++;
        } else {
            s_month++;
        }
    } else {
        s_day++;
    }
}

uint8_t Calendar_DayOfWeek(void) {
    return DayOfWeek(s_year, s_month, s_day);
}

uint8_t Calendar_LastSundayOfMarch(void) {
    return LastSundayOfMonth(s_year, 3);
}

uint8_t Calendar_LastSundayOfOctober(void) {
    return LastSundayOfMonth(s_year, 10);
}

uint8_t Calendar_IsDST(void) {
    if (s_month < 3 || s_month > 10) return 0;
    if (s_month > 3 && s_month < 10) return 1;
    if (s_month == 3) return (s_day >= LastSundayOfMonth(s_year, 3));
    return (s_day <= LastSundayOfMonth(s_year, 10));  /* month == 10 */
}

uint16_t Calendar_GetYear(void) {
    return s_year;
}

uint8_t Calendar_GetMonth(void) {
    return s_month;
}

uint8_t Calendar_GetDay(void) {
    return s_day;
}
