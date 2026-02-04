/*******************************************************************************
 * File: leds.h
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: LEDs driver interface.
 *              Controls 10 LEDs on scattered ports (G, A, F, B, C): main light
 *              (LED 10, RC6), binary clock display (LEDs 1-5 for 5-bit hour),
 *              and heartbeat (LED 9, RB1). Also configures the LDR pin as analog
 *              input for the ADC.
 ******************************************************************************/

#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize the 10-LED output bus and LDR input pin.
 *
 * Sets all 10 LED pins as outputs (initially off) and LDR pin as analog input.
 */
void LEDs_Init(void);

/**
 * @brief Set the main streetlight on or off (LED 10, RC6).
 *
 * @param state true = light ON, false = light OFF
 */
void LEDs_SetMainLight(bool state);

/**
 * @brief Update the binary clock display to show the current hour (LEDs 1-5).
 *
 * Displays hour (0-23) as a 5-bit binary pattern on LEDs 1 through 5.
 *
 * @param hour Current hour (0-23)
 */
void LEDs_SetClockDisplay(uint8_t hour);

/**
 * @brief Toggle the heartbeat LED (LED 9, RB1) for visual status indication.
 */
void LEDs_ToggleHeartbeat(void);

#endif /* LEDS_H */
