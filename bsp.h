/*******************************************************************************
 * File: bsp.h
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Board Support Package - Hardware Abstraction Layer
 *              This is the ONLY module allowed to include <xc.h> and access
 *              hardware registers directly.
 ******************************************************************************/

#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize all hardware peripherals
 * 
 * Configures:
 * - Oscillator (16MHz internal)
 * - GPIO pins (outputs for LEDs, relay; input for LDR)
 * - ADCC (ADC with Computation) for LDR reading
 * - Timer0 for 1-second (or accelerated) ticks
 */
void BSP_Init(void);

/**
 * @brief Read the Light Dependent Resistor (LDR) value
 * 
 * Uses ADCC hardware accumulation to average 32 samples for noise reduction
 * 
 * @return uint16_t ADC value (0 = dark, 1023 = bright)
 */
uint16_t BSP_GetLDR(void);

/**
 * @brief Control the main outdoor light
 * 
 * @param state true = light ON, false = light OFF
 */
void BSP_SetMainLight(bool state);

/**
 * @brief Update the binary clock display
 * 
 * Maps the hour (0-23) to a 5-bit binary representation on LEDs
 * 
 * @param hour Current hour (0-23)
 */
void BSP_SetClockDisplay(uint8_t hour);

/**
 * @brief Toggle the heartbeat LED for visual health check
 */
void BSP_ToggleHeartbeat(void);

#endif // BSP_H
