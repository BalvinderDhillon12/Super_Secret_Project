/*******************************************************************************
 * File:   Calendar.h
 * Purpose: Internal calendar with leap year and UK DST (BST) support.
 ******************************************************************************/

#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdint.h>

void Calendar_Init(uint16_t year, uint8_t month, uint8_t day);
void Calendar_AdvanceDay(void);
uint8_t Calendar_IsLeapYear(uint16_t y);
uint8_t Calendar_DayOfWeek(void);
uint8_t Calendar_IsDST(void);
uint8_t Calendar_LastSundayOfMarch(void);
uint8_t Calendar_LastSundayOfOctober(void);
uint16_t Calendar_GetYear(void);
uint8_t Calendar_GetMonth(void);
uint8_t Calendar_GetDay(void);

#endif /* CALENDAR_H */
