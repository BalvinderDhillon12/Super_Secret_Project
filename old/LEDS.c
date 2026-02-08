/*******************************************************************************
 * File: LEDS.c
 * Description: LEDs driver interface.
 *              Controls 9 LEDs from the 10 LED bus: 
 *              - Binary clock (LEDs 1-5 for 5-bit hour),
 *              - LEDs 6-7 off, 
 *              - Heartbeat (LED 8, RB0), 
 *              - Main light (LED 9, RB1).
 ******************************************************************************/

 #include <xc.h>
 #include "LEDS.h"
 #include "Config.h"
 
 /**
  * Initialize the 10-LED bus.
  * Set the first 9 LED pins (10 is unused) as outputs (initially off).
  */
 void LEDs_Init(void) {
     // LED 1: RG0 
     TRISGbits.TRISG0 = 0;
     LATGbits.LATG0 = 0;
     // LED 2: RG1 
     TRISGbits.TRISG1 = 0;
     LATGbits.LATG1 = 0;
     // LED 3: RA2 
     TRISAbits.TRISA2 = 0;
     LATAbits.LATA2 = 0;
     // LED 4: RF6 
     TRISFbits.TRISF6 = 0;
     LATFbits.LATF6 = 0;
     // LED 5: RA4 
     ANSELAbits.ANSELA4 = 0;
     TRISAbits.TRISA4 = 0;
     LATAbits.LATA4 = 0;
     // LED 6: RA5 
     TRISAbits.TRISA5 = 0;
     LATAbits.LATA5 = 0;
     // LED 7: RF0 
     TRISFbits.TRISF0 = 0;
     LATFbits.LATF0 = 0;
     // LED 8: RB0 
     TRISBbits.TRISB0 = 0;
     LATBbits.LATB0 = 0;
     // LED 9: RB1 
     TRISBbits.TRISB1 = 0;
     LATBbits.LATB1 = 0;
 }
 
 /**
  * Set the main streetlight on or off (LED 9, RB1).
  */
 void LEDs_SetMainLight(bool state) {
     LATBbits.LATB1 = state ? 1 : 0;
 }
 
/**
  * Update the binary clock display to show the current hour (LEDs 1-5).
  * Display  current hour (0-23) as a 5-bit binary pattern on LEDs 1 through 5.
  */
 void LEDs_SetClockDisplay(uint8_t hour) {
     if (hour >= HOURS_PER_DAY) {
         hour = 0;
     }
 
     LATGbits.LATG0 = (hour & 1) ? 1 : 0;   // LED 1: bit 0
     LATGbits.LATG1 = (hour & 2) ? 1 : 0;   // LED 2: bit 1 
     LATAbits.LATA2 = (hour & 4) ? 1 : 0;   // LED 3: bit 2 
     LATFbits.LATF6 = (hour & 8) ? 1 : 0;   // LED 4: bit 3 
     LATAbits.LATA4 = (hour & 16) ? 1 : 0;  // LED 5: bit 4 
 }
 
 /**
  * Toggle the heartbeat LED (LED 8, RB0) for visual status indication.
  */
 void LEDs_ToggleHeartbeat(void) {
     LATBbits.LATB0 ^= 1;
 }
 