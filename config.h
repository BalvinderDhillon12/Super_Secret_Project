#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// System Clock Frequency
#define _XTAL_FREQ 64000000

// Test Mode Toggle (Comment out for normal mode)
#define TEST_MODE

#ifdef TEST_MODE
    #define TICK_PERIOD_MS 10 // 1 virtual second = 10ms (1 day = 24 * 3600 * 10ms = 864s)
#else
    #define TICK_PERIOD_MS 1000 // 1 real second = 1000ms
#endif

// Pin Mappings
// LEDs for Binary Hour (5 bits: 0-23)
#define LED_H0 LATDbits.LATD0
#define LED_H1 LATDbits.LATD1
#define LED_H2 LATDbits.LATD2
#define LED_H3 LATDbits.LATD3
#define LED_H4 LATDbits.LATD4

#define TRIS_H0 TRISDbits.TRISD0
#define TRIS_H1 TRISDbits.TRISD1
#define TRIS_H2 TRISDbits.TRISD2
#define TRIS_H3 TRISDbits.TRISD3
#define TRIS_H4 TRISDbits.TRISD4

// Main Outside Light LED
#define LED_OUTSIDE LATDbits.LATD5
#define TRIS_OUTSIDE TRISDbits.TRISD5

// Extra functional LEDs (for status or future use)
#define LED_STATUS1 LATDbits.LATD6
#define LED_STATUS2 LATDbits.LATD7
#define LED_STATUS3 LATAbits.LATA6 // Assuming RA6 is available

#define TRIS_STATUS1 TRISDbits.TRISD6
#define TRIS_STATUS2 TRISDbits.TRISD7
#define TRIS_STATUS3 TRISAbits.TRISA6

// LDR Input (ADC Channel)
#define LDR_CHANNEL 0x00 // AN0
#define TRIS_LDR TRISAbits.TRISA0
#define ANSEL_LDR ANSELAbits.ANSELA0

// Thresholds
#define LDR_THRESHOLD_DUSK 400  // Example ADC value for dusk
#define LDR_THRESHOLD_DAWN 600  // Example ADC value for dawn (hysteresis)

#endif // CONFIG_H

