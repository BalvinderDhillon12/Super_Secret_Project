/*******************************************************************************
 * File: leds.c
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: LEDs driver implementation.
 *              Owns all code for the 9-LED output bus on scattered ports:
 *              binary clock (LEDs 1-5: RG0, RG1, RA2, RF6, RA4), LEDs 6-7 off (RA5, RF0),
 *              heartbeat (LED 8, RB0), main light (LED 9, RB1). LDR pin (RA3) in ADC_Init().
 ******************************************************************************/

#include <xc.h>
#include "leds.h"
#include "config.h"

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Initialize the 9-LED bus.
 *
 * All 9 LED pins are set as outputs (off at reset).
 * LDR pin (RA3) is configured in ADC_Init().
 */
void LEDs_Init(void) {
    /* LED 1: RG0 */
    TRISGbits.TRISG0 = 0;
    LATGbits.LATG0 = 0;
    /* LED 2: RG1 */
    TRISGbits.TRISG1 = 0;
    LATGbits.LATG1 = 0;
    /* LED 3: RA2 */
    TRISAbits.TRISA2 = 0;
    LATAbits.LATA2 = 0;
    /* LED 4: RF6 */
    TRISFbits.TRISF6 = 0;
    LATFbits.LATF6 = 0;
    /* LED 5: RA4 */
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    /* LED 6: RA5 */
    TRISAbits.TRISA5 = 0;
    LATAbits.LATA5 = 0;
    /* LED 7: RF0 */
    TRISFbits.TRISF0 = 0;
    LATFbits.LATF0 = 0;
    /* LED 8: RB0 */
    TRISBbits.TRISB0 = 0;
    LATBbits.LATB0 = 0;
    /* LED 9: RB1 (main light / day-night) */
    TRISBbits.TRISB1 = 0;
    LATBbits.LATB1 = 0;
}

/**
 * @brief Set the main streetlight output high or low (LED 9, RB1).
 *
 * We drive the pin high for ON and low for OFF; the hardware (relay/MOSFET)
 * inverts if necessary.
 */
void LEDs_SetMainLight(bool state) {
    LATBbits.LATB1 = state ? 1 : 0;
}

/**
 * @brief Set the binary clock display to show the given hour (LEDs 1-5).
 *
 * Hour must be 0-23. Bits 0-4 map to LEDs 1-5 (RG0, RG1, RA2, RF6, RA4).
 */
void LEDs_SetClockDisplay(uint8_t hour) {
    if (hour >= HOURS_PER_DAY) {
        hour = 0;
    }

    LATGbits.LATG0 = (hour & 1) ? 1 : 0;   /* LED 1: bit 0 */
    LATGbits.LATG1 = (hour & 2) ? 1 : 0;   /* LED 2: bit 1 */
    LATAbits.LATA2 = (hour & 4) ? 1 : 0;   /* LED 3: bit 2 */
    LATFbits.LATF6 = (hour & 8) ? 1 : 0;   /* LED 4: bit 3 */
    LATAbits.LATA4 = (hour & 16) ? 1 : 0;  /* LED 5: bit 4 */
}

/**
 * @brief Toggle the heartbeat LED (LED 8, RB0).
 *
 * Used by the main loop at a slow rate to indicate the system is running.
 */
void LEDs_ToggleHeartbeat(void) {
    LATBbits.LATB0 ^= 1;
}
