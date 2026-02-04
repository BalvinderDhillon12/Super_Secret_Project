/*******************************************************************************
 * File: config.h
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: System configuration and hardware mapping.
 *              Single source of truth for all global constants, pin definitions,
 *              and timing parameters. Driver modules include this for thresholds
 *              and hardware mappings.
 ******************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * TEST MODE CONFIGURATION
 * Uncomment to enable test mode where 1 day = 24 seconds (1 hour = 1 second).
 * Comment out for production mode where 1 day = 24 hours.
 ******************************************************************************/
#define TEST_MODE

/*******************************************************************************
 * TIMER CONFIGURATION
 * Timer0 reload values for 1-second (or 1/3600 second in test mode) ticks.
 *
 * For PIC18F67K40 @ 64 MHz with Timer0 in 16-bit mode:
 * - Prescaler 1:256
 * - Timer clock = 64 MHz / 4 / 256 = 62.5 kHz
 *
 * Production: 1.0 s tick requires 62500 counts
 *   Timer reload = 65536 - 62500 = 3036; verified lab value 3035 (0x0BDB)
 *   TMR0_RELOAD_HIGH = 0x0B, TMR0_RELOAD_LOW = 0xDB
 *
 * Test Mode: short period for accelerated time (reload 0xFFFC)
 ******************************************************************************/
#ifdef TEST_MODE
    #define TMR0_RELOAD_HIGH    0xFF
    #define TMR0_RELOAD_LOW     0xFC
    #define TICKS_PER_SECOND    3600    /* Accelerated: 3600 ticks = 1 virtual second */
#else
    #define TMR0_RELOAD_HIGH    0x0B
    #define TMR0_RELOAD_LOW     0xDB
    #define TICKS_PER_SECOND    1       /* Normal: 1 tick = 1 real second */
#endif

/*******************************************************************************
 * LDR (LIGHT DEPENDENT RESISTOR) CONFIGURATION
 * ADC values range from 0 (dark) to 1023 (bright).
 * Hysteresis prevents oscillation around the transition point.
 ******************************************************************************/
#define LDR_THRESHOLD_DUSK      400     /* Below this = transition to dark */
#define LDR_THRESHOLD_DAWN      600     /* Above this = transition to light */

/*******************************************************************************
 * ENERGY-SAVING CONSTANTS
 * 1am-5am: light is turned off even when dark to save energy.
 ******************************************************************************/
#define ENERGY_SAVE_START_HOUR  1       /* 1am - turn light off */
#define ENERGY_SAVE_END_HOUR    5       /* 5am - turn light back on */

/*******************************************************************************
 * SOLAR / DST CONSTANTS
 * Solar midnight targets for drift correction and DST detection.
 ******************************************************************************/
#define SOLAR_MIDNIGHT_WINTER   0       /* 00:00 (midnight) in winter */
#define SOLAR_MIDNIGHT_SUMMER   1       /* 01:00 (1am) in summer (DST) */

/* Day length thresholds for season detection (in hours) */
#define DAY_LENGTH_SUMMER_MIN   14      /* > 14 hours = summer (DST active) */
#define DAY_LENGTH_WINTER_MAX   10      /* < 10 hours = winter (DST inactive) */

/*******************************************************************************
 * SYSTEM TIME CONSTANTS
 ******************************************************************************/
#define SECONDS_PER_MINUTE      60
#define MINUTES_PER_HOUR        60
#define HOURS_PER_DAY           24
#define MINUTES_PER_DAY         1440    /* 24 * 60 */

#endif /* CONFIG_H */
