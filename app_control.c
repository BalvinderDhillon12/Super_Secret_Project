/*******************************************************************************
 * File: app_control.c
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Application Control Implementation
 ******************************************************************************/

#include "app_control.h"
#include "config.h"
#include "bsp.h"

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
static bool IsInEnergySaveWindow(uint8_t hour);

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

void APP_Task(Time_t now, bool is_dark) {
    bool light_should_be_on = false;
    
    // Light Control Rule:
    // Turn light ON if:
    //   1. It is dark (nighttime), AND
    //   2. NOT in energy saving window (1am to 5am)
    
    if (is_dark && !IsInEnergySaveWindow(now.hours)) {
        light_should_be_on = true;
    }
    
    // Apply the decision to hardware
    BSP_SetMainLight(light_should_be_on);
}

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

static bool IsInEnergySaveWindow(uint8_t hour) {
    // Energy saving window: 1am to 5am (inclusive of 1am, exclusive of 5am)
    // Hours 1, 2, 3, 4 should have light OFF
    
    if (hour >= ENERGY_SAVE_START_HOUR && hour < ENERGY_SAVE_END_HOUR) {
        return true;
    }
    
    return false;
}
