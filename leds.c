/*******************************************************************************
 * File: leds.c
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: LEDs driver implementation.
 *              Owns all code for the 10-LED output bus: main light, binary
 *              clock display, and heartbeat LED. Pin mappings come from
 *              config.h; each hardware-dependent line is marked for customization.
 ******************************************************************************/

#include <xc.h>
#include "leds.h"
#include "config.h"

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Initialize the 10-LED bus and LDR input pin.
 *
 * Main light and heartbeat are outputs (off at reset). Binary clock uses
 * the lower 5 bits of the configured port. LDR pin is set as input and
 * analog so the ADC can read it.
 */
void LEDs_Init(void) {
    /* TODO: CUSTOMIZE - Check schematic for Main Light pin assignment. */
    LIGHT_TRIS = 0;   /* Output */
    LIGHT_LAT = 0;    /* Initially OFF */

    /* TODO: CUSTOMIZE - Check schematic for Heartbeat LED pin assignment. */
    HEARTBEAT_TRIS = 0;  /* Output */
    HEARTBEAT_LAT = 0;   /* Initially OFF */

    /* TODO: CUSTOMIZE - Check schematic for Binary Clock (10-LED bus) port/pin assignment. */
    CLOCK_DISPLAY_TRIS &= ~CLOCK_DISPLAY_MASK;  /* Lower 5 bits as outputs */
    CLOCK_DISPLAY_LAT &= ~CLOCK_DISPLAY_MASK;   /* Initially all OFF */

    /* TODO: CUSTOMIZE - Check schematic for LDR (analog) pin assignment. */
    LDR_TRIS = 1;   /* Input */
    LDR_ANSEL = 1;  /* Analog mode for ADC */
}

/**
 * @brief Set the main streetlight output high or low.
 *
 * We drive the pin high for ON and low for OFF; the hardware (relay/MOSFET)
 * inverts if necessary.
 */
void LEDs_SetMainLight(bool state) {
    /* TODO: CUSTOMIZE - Check schematic for Main Light pin assignment. */
    LIGHT_LAT = state ? 1 : 0;
}

/**
 * @brief Set the binary clock display to show the given hour.
 *
 * Hour must be 0-23. We mask to 5 bits (0x1F) so the display shows a valid
 * pattern; we only update the bits that belong to the clock mask so other
 * pins on the same port are unchanged.
 */
void LEDs_SetClockDisplay(uint8_t hour) {
    if (hour >= HOURS_PER_DAY) {
        hour = 0;
    }

    /* TODO: CUSTOMIZE - Check schematic for Binary Clock (10-LED bus) port assignment. */
    CLOCK_DISPLAY_LAT = (CLOCK_DISPLAY_LAT & ~CLOCK_DISPLAY_MASK) |
                        (hour & CLOCK_DISPLAY_MASK);
}

/**
 * @brief Toggle the heartbeat LED.
 *
 * Used by the main loop at a slow rate to indicate the system is running.
 */
void LEDs_ToggleHeartbeat(void) {
    /* TODO: CUSTOMIZE - Check schematic for Heartbeat LED pin assignment. */
    HEARTBEAT_LAT ^= 1;
}
