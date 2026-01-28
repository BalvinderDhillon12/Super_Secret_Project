#ifndef TIMEKEEPING_H
#define TIMEKEEPING_H

#include "config.h"

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday; // 0=Sunday, 1=Monday, ...
    bool is_dst;
} DateTime_t;

extern DateTime_t current_time;

void Timekeeping_Init(uint8_t h, uint8_t m, uint8_t d, uint8_t mon, uint16_t y);
void Timekeeping_Tick(void);
void Timekeeping_AdjustHours(int8_t offset);
bool Timekeeping_IsLeapYear(uint16_t year);

#endif // TIMEKEEPING_H

