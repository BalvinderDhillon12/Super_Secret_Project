/*******************************************************************************
 * File: rtc_soft.h
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Software Real-Time Clock
 *              Maintains wall clock time independent of hardware timer source
 ******************************************************************************/

#ifndef RTC_SOFT_H
#define RTC_SOFT_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/

/**
 * @brief Time structure for hours, minutes, seconds
 */
typedef struct {
    uint8_t hours;      // 0-23
    uint8_t minutes;    // 0-59
    uint8_t seconds;    // 0-59
} Time_t;

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize the RTC based on initial light level
 * 
 * Sets initial time estimate:
 * - If dark (LDR low): assume midnight (00:00:00)
 * - If bright (LDR high): assume noon (12:00:00)
 * 
 * @param initial_ldr_value Current LDR reading for initial calibration
 */
void RTC_Init(uint16_t initial_ldr_value);

/**
 * @brief Tick handler called from ISR
 * 
 * Increments the clock: seconds -> minutes -> hours -> day rollover
 * Must be called at 1Hz rate (or accelerated rate in test mode)
 */
void RTC_TickISR(void);

/**
 * @brief Get current time
 * 
 * Uses critical section to ensure atomic read of multi-byte values
 * 
 * @return Time_t Current time structure
 */
Time_t RTC_GetTime(void);

/**
 * @brief Get total minutes since midnight
 * 
 * Useful for arithmetic comparisons and calculations
 * 
 * @return uint16_t Minutes since midnight (0-1439)
 */
uint16_t RTC_GetTotalMinutes(void);

/**
 * @brief Apply time synchronization adjustment
 * 
 * Adds or subtracts minutes to correct for drift
 * Handles wraparound at midnight
 * 
 * @param adjustment_min Signed adjustment in minutes (positive = add, negative = subtract)
 */
void RTC_ApplySync(int16_t adjustment_min);

/**
 * @brief Set time directly (used for initial calibration)
 * 
 * @param hours Hour to set (0-23)
 * @param minutes Minute to set (0-59)
 * @param seconds Second to set (0-59)
 */
void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

#endif // RTC_SOFT_H
