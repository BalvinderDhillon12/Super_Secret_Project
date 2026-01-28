/*******************************************************************************
 * File: config.h
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: System configuration and hardware mapping
 *              Single source of truth for all timing constants and pin definitions
 ******************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * TEST MODE CONFIGURATION
 * Uncomment to enable test mode where 1 day = 24 seconds (1 hour = 1 second)
 * Comment out for production mode where 1 day = 24 hours
 ******************************************************************************/
#define TEST_MODE

/*******************************************************************************
 * TIMER CONFIGURATION
 * Timer0 reload values for 1-second (or 1/3600 second in test mode) ticks
 * 
 * For PIC18F67K40 @ 16MHz with Timer0 in 16-bit mode:
 * - Prescaler 1:256
 * - Timer clock = 16MHz / 4 / 256 = 15.625 kHz
 * 
 * Production: 1.0s tick requires 15625 counts
 * Timer reload = 65536 - 15625 = 49911 (0xC2F7)
 * 
 * Test Mode: ~277.8us tick requires ~4.34 counts (use 4)
 * Timer reload = 65536 - 4 = 65532 (0xFFFC)
 ******************************************************************************/
#ifdef TEST_MODE
    #define TMR0_RELOAD_HIGH    0xFF
    #define TMR0_RELOAD_LOW     0xFC
    #define TICKS_PER_SECOND    3600    // Accelerated: 3600 ticks = 1 virtual second
#else
    #define TMR0_RELOAD_HIGH    0xC2
    #define TMR0_RELOAD_LOW     0xF7
    #define TICKS_PER_SECOND    1       // Normal: 1 tick = 1 real second
#endif

/*******************************************************************************
 * LDR (LIGHT DEPENDENT RESISTOR) CONFIGURATION
 * ADC values range from 0 (dark) to 1023 (bright)
 * Hysteresis prevents oscillation around the transition point
 ******************************************************************************/
#define LDR_THRESHOLD_DUSK      400     // Below this = transition to dark
#define LDR_THRESHOLD_DAWN      600     // Above this = transition to light
#define LDR_ADC_CHANNEL         0       // ANx channel for LDR input

/*******************************************************************************
 * HARDWARE PIN MAPPINGS
 * Adjust these based on your specific hardware connections
 ******************************************************************************/

// Main Light Control (Relay/MOSFET)
#define LIGHT_TRIS              TRISDbits.TRISD0
#define LIGHT_LAT               LATDbits.LATD0
#define LIGHT_PORT              PORTDbits.RD0

// Heartbeat LED (Debug/Status Indicator)
#define HEARTBEAT_TRIS          TRISDbits.TRISD1
#define HEARTBEAT_LAT           LATDbits.LATD1

// Binary Clock Display (10 LEDs for hours 0-23, needs 5 bits minimum)
// Using PORTB for the LED array (RB0-RB4 for 5-bit binary representation)
#define CLOCK_DISPLAY_TRIS      TRISB
#define CLOCK_DISPLAY_LAT       LATB
#define CLOCK_DISPLAY_MASK      0x1F    // Lower 5 bits (RB0-RB4)

// LDR ADC Input
#define LDR_TRIS                TRISAbits.TRISA0    // Analog input
#define LDR_ANSEL               ANSELAbits.ANSELA0

/*******************************************************************************
 * TIMING CONSTANTS
 ******************************************************************************/
#define ENERGY_SAVE_START_HOUR  1       // 1am - turn light off
#define ENERGY_SAVE_END_HOUR    5       // 5am - turn light back on

// Solar midnight targets for DST detection
#define SOLAR_MIDNIGHT_WINTER   0       // 00:00 (midnight) in winter
#define SOLAR_MIDNIGHT_SUMMER   1       // 01:00 (1am) in summer (DST)

// Day length thresholds for season detection (in hours)
#define DAY_LENGTH_SUMMER_MIN   14      // > 14 hours = summer (DST active)
#define DAY_LENGTH_WINTER_MAX   10      // < 10 hours = winter (DST inactive)

/*******************************************************************************
 * SYSTEM CONSTANTS
 ******************************************************************************/
#define SECONDS_PER_MINUTE      60
#define MINUTES_PER_HOUR        60
#define HOURS_PER_DAY           24
#define MINUTES_PER_DAY         1440    // 24 * 60

/*******************************************************************************
 * OSCILLATOR CONFIGURATION
 ******************************************************************************/
#define _XTAL_FREQ              16000000UL  // 16 MHz

#endif // CONFIG_H
