/*******************************************************************************
 * File: LEDS.h
 * Description: LEDs driver interface.
 *              Controls 9 LEDs from the 10 LED bus: 
 *              - Binary clock (LEDs 1-5 for 5-bit hour),
 *              - LEDs 6-7 off, 
 *              - Heartbeat (LED 8, RB0), 
 *              - Main light (LED 9, RB1).
 ******************************************************************************/

 #ifndef LEDS_H
 #define LEDS_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 /**
  * Initialize the 10-LED bus.
  * Set the first 9 LED pins (10 is unused) as outputs (initially off).
  */
 void LEDs_Init(void);
 
 /**
  * Set the main streetlight on or off (LED 9, RB1).
  */
 void LEDs_SetMainLight(bool state);
 
 /**
  * Update the binary clock display to show the current hour (LEDs 1-5).
  * Display current hour (0-23) as a 5-bit binary pattern on LEDs 1 through 5.
  */
 void LEDs_SetClockDisplay(uint8_t hour);
 
 /**
  * Toggle the heartbeat LED (LED 8, RB0) for visual status indication.
  */
 void LEDs_ToggleHeartbeat(void);
 
 #endif /* LEDS_H */