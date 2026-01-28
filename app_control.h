/*******************************************************************************
 * File: app_control.h
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Application Control Layer
 *              Orchestrates the light control rules and display updates
 ******************************************************************************/

#ifndef APP_CONTROL_H
#define APP_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "rtc_soft.h"

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Main application task - apply control logic
 * 
 * Implements the light control rule:
 * - IF dark (night) AND NOT between 1am-5am THEN light ON
 * - ELSE light OFF
 * 
 * Energy saving: Light is OFF between 1am and 5am even if dark
 * 
 * @param now Current time
 * @param is_dark Current day/night state from solar manager
 */
void APP_Task(Time_t now, bool is_dark);

#endif // APP_CONTROL_H
