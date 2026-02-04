/*******************************************************************************
 * File: leds.h
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: LEDs driver interface.
 *              Controls the single 10-LED output bus: main streetlight,
 *              binary clock display (5 bits for hour), and heartbeat status LED.
 *              Also configures the LDR pin as analog input for the ADC.
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
 * Sets main light, heartbeat, and binary clock pins as outputs (initially off),
 * and LDR pin as analog input for ADC.
 */
void LEDs_Init(void);

/**
 * @brief Set the main streetlight on or off.
 *
 * @param state true = light ON, false = light OFF
 */
void LEDs_SetMainLight(bool state);

/**
 * @brief Update the binary clock display to show the current hour.
 *
 * Displays hour (0-23) as a 5-bit binary pattern on the LED array.
 *
 * @param hour Current hour (0-23)
 */
void LEDs_SetClockDisplay(uint8_t hour);

/**
 * @brief Toggle the heartbeat LED for visual status indication.
 */
void LEDs_ToggleHeartbeat(void);

#endif /* LEDS_H */
