/*******************************************************************************
 * File: solar_mgr.h
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Solar Logic Manager
 *              Core intelligence for tracking sun cycles, detecting DST,
 *              and calculating drift corrections
 ******************************************************************************/

#ifndef SOLAR_MGR_H
#define SOLAR_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include "rtc_soft.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize the solar manager state machine
 */
void SOLAR_Init(void);

/**
 * @brief Update solar state and calculate drift corrections
 * 
 * This function should be called periodically (every main loop iteration)
 * with the current LDR reading and time.
 * 
 * State machine:
 * - Applies hysteresis filtering to LDR input
 * - Detects Dusk and Dawn transitions
 * - Records timestamps for cycle analysis
 * - Calculates solar midnight and drift error
 * - Determines season (DST) based on day length
 * 
 * @param ldr_value Current LDR reading (0-1023)
 * @param now Current time
 * @return int16_t Drift correction in minutes (0 = no correction needed)
 */
int16_t SOLAR_Update(uint16_t ldr_value, Time_t now);

/**
 * @brief Get current day/night state
 * 
 * @return bool true = dark (night), false = bright (day)
 */
bool SOLAR_IsDark(void);

#endif // SOLAR_MGR_H
