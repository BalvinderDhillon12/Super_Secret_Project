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
 * For PIC18F67K40 @ 16MHz with Timer0 in 16-bit mode:
 * - Prescaler 1:256
 * - Timer clock = 16MHz / 4 / 256 = 15.625 kHz
 *
 * Production: 1.0s tick requires 15625 counts
 *   Timer reload = 65536 - 15625 = 49911 (0xC2F7)
 *
 * Test Mode: ~277.8us tick requires ~4.34 counts (use 4)
 *   Timer reload = 65536 - 4 = 65532 (0xFFFC)
 ******************************************************************************/
#ifdef TEST_MODE
    /* TODO: CUSTOMIZE - Check schematic for Timer0 reload assignment (test mode) */
    #define TMR0_RELOAD_HIGH    0xFF
    #define TMR0_RELOAD_LOW     0xFC
    #define TICKS_PER_SECOND    3600    /* Accelerated: 3600 ticks = 1 virtual second */
#else
    /* TODO: CUSTOMIZE - Check schematic for Timer0 reload assignment (production) */
    #define TMR0_RELOAD_HIGH    0xC2
    #define TMR0_RELOAD_LOW     0xF7
    #define TICKS_PER_SECOND    1       /* Normal: 1 tick = 1 real second */
#endif

/*******************************************************************************
 * LDR (LIGHT DEPENDENT RESISTOR) CONFIGURATION
 * ADC values range from 0 (dark) to 1023 (bright).
 * Hysteresis prevents oscillation around the transition point.
 ******************************************************************************/
#define LDR_THRESHOLD_DUSK      400     /* Below this = transition to dark */
#define LDR_THRESHOLD_DAWN      600     /* Above this = transition to light */
#define LDR_ADC_CHANNEL         0       /* ANx channel for LDR input */

/*******************************************************************************
 * HARDWARE PIN MAPPINGS
 * Adjust these based on your specific hardware connections.
 * Each driver (leds.c, adc.c) uses these; change here to match your schematic.
 ******************************************************************************/

/* Main Light Control (Relay/MOSFET) - used by leds.c */
/* TODO: CUSTOMIZE - Check schematic for Main Light pin assignment */
#define LIGHT_TRIS              TRISDbits.TRISD0
#define LIGHT_LAT               LATDbits.LATD0
#define LIGHT_PORT              PORTDbits.RD0

/* Heartbeat LED (Debug/Status Indicator) - used by leds.c */
/* TODO: CUSTOMIZE - Check schematic for Heartbeat LED pin assignment */
#define HEARTBEAT_TRIS          TRISDbits.TRISD1
#define HEARTBEAT_LAT           LATDbits.LATD1

/* Binary Clock Display (5 bits of 10-LED bus for hours 0-23) - used by leds.c */
/* TODO: CUSTOMIZE - Check schematic for Binary Clock port/pin assignment */
#define CLOCK_DISPLAY_TRIS      TRISB
#define CLOCK_DISPLAY_LAT       LATB
#define CLOCK_DISPLAY_MASK      0x1F    /* Lower 5 bits (RB0-RB4) */

/* LDR ADC Input - used by adc.c and for analog config in leds.c */
/* TODO: CUSTOMIZE - Check schematic for LDR (ANx) pin assignment */
#define LDR_TRIS                TRISAbits.TRISA0
#define LDR_ANSEL               ANSELAbits.ANSELA0

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

/*******************************************************************************
 * OSCILLATOR CONFIGURATION
 ******************************************************************************/
#define _XTAL_FREQ              16000000UL  /* 16 MHz */

#endif /* CONFIG_H */
