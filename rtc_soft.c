/*******************************************************************************
 * File: rtc_soft.c
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Software Real-Time Clock Implementation
 ******************************************************************************/

#include <xc.h>
#include "rtc_soft.h"
#include "config.h"

/*******************************************************************************
 * PRIVATE VARIABLES
 * 
 * NOTE: These are shared between ISR and main loop, so multi-byte variables
 *       require critical sections for atomic access
 ******************************************************************************/
static volatile uint8_t rtc_seconds = 0;
static volatile uint8_t rtc_minutes = 0;
static volatile uint8_t rtc_hours = 0;

#ifdef TEST_MODE
static volatile uint16_t rtc_tick_counter = 0;  // Accumulator for accelerated ticks
#endif

/*******************************************************************************
 * CRITICAL SECTION MACROS
 * 
 * For PIC18, GIE (Global Interrupt Enable) is in INTCON0
 ******************************************************************************/
#define ENTER_CRITICAL()    (INTCON0bits.GIE = 0)
#define EXIT_CRITICAL()     (INTCON0bits.GIE = 1)

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

void RTC_Init(uint16_t initial_ldr_value) {
    // Estimate initial time based on light level
    if (initial_ldr_value < LDR_THRESHOLD_DUSK) {
        // Dark - assume midnight
        RTC_SetTime(0, 0, 0);
    } else {
        // Bright - assume noon
        RTC_SetTime(12, 0, 0);
    }
    
#ifdef TEST_MODE
    rtc_tick_counter = 0;
#endif
}

void RTC_TickISR(void) {
    // This function is called from the Timer0 ISR
    // In normal mode: called once per second
    // In test mode: called 3600 times per second
    
#ifdef TEST_MODE
    // In test mode, accumulate ticks until we reach a "virtual second"
    rtc_tick_counter++;
    
    if (rtc_tick_counter >= TICKS_PER_SECOND) {
        rtc_tick_counter = 0;
        // Fall through to increment time
    } else {
        // Not yet a full virtual second
        return;
    }
#endif
    
    // Increment seconds
    rtc_seconds++;
    
    if (rtc_seconds >= SECONDS_PER_MINUTE) {
        rtc_seconds = 0;
        rtc_minutes++;
        
        if (rtc_minutes >= MINUTES_PER_HOUR) {
            rtc_minutes = 0;
            rtc_hours++;
            
            if (rtc_hours >= HOURS_PER_DAY) {
                rtc_hours = 0;  // Midnight rollover
            }
        }
    }
}

Time_t RTC_GetTime(void) {
    Time_t current_time;
    
    // Enter critical section to ensure atomic read
    ENTER_CRITICAL();
    
    current_time.hours = rtc_hours;
    current_time.minutes = rtc_minutes;
    current_time.seconds = rtc_seconds;
    
    EXIT_CRITICAL();
    
    return current_time;
}

uint16_t RTC_GetTotalMinutes(void) {
    uint16_t total_minutes;
    uint8_t hours_copy, minutes_copy;
    
    // Enter critical section
    ENTER_CRITICAL();
    
    hours_copy = rtc_hours;
    minutes_copy = rtc_minutes;
    
    EXIT_CRITICAL();
    
    // Calculate total minutes (hours * 60 + minutes)
    // Use shift and add to avoid multiplication on 8-bit PIC
    // hours * 60 = hours * 64 - hours * 4
    //            = (hours << 6) - (hours << 2)
    total_minutes = ((uint16_t)hours_copy << 6) - ((uint16_t)hours_copy << 2);
    total_minutes += minutes_copy;
    
    return total_minutes;
}

void RTC_ApplySync(int16_t adjustment_min) {
    int16_t total_minutes;
    uint8_t new_hours, new_minutes;
    
    // Get current total minutes
    ENTER_CRITICAL();
    total_minutes = ((int16_t)rtc_hours << 6) - ((int16_t)rtc_hours << 2) + rtc_minutes;
    EXIT_CRITICAL();
    
    // Apply adjustment
    total_minutes += adjustment_min;
    
    // Handle wraparound (day has 1440 minutes)
    while (total_minutes < 0) {
        total_minutes += MINUTES_PER_DAY;
    }
    
    while (total_minutes >= MINUTES_PER_DAY) {
        total_minutes -= MINUTES_PER_DAY;
    }
    
    // Convert back to hours and minutes
    // Divide by 60: use subtraction loop (avoid division operator)
    new_hours = 0;
    while (total_minutes >= MINUTES_PER_HOUR) {
        total_minutes -= MINUTES_PER_HOUR;
        new_hours++;
    }
    new_minutes = (uint8_t)total_minutes;
    
    // Update time atomically
    ENTER_CRITICAL();
    rtc_hours = new_hours;
    rtc_minutes = new_minutes;
    rtc_seconds = 0;  // Reset seconds on sync
    EXIT_CRITICAL();
}

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    // Validate inputs
    if (hours >= HOURS_PER_DAY) hours = 0;
    if (minutes >= MINUTES_PER_HOUR) minutes = 0;
    if (seconds >= SECONDS_PER_MINUTE) seconds = 0;
    
    // Set time atomically
    ENTER_CRITICAL();
    rtc_hours = hours;
    rtc_minutes = minutes;
    rtc_seconds = seconds;
    EXIT_CRITICAL();
}
