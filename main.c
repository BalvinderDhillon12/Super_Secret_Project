/*******************************************************************************
 * File: main.c
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * Compiler: XC8 (C99)
 * 
 * Description: Main entry point and super-loop
 *              Solar-synchronized outdoor light controller with automatic DST
 *              and energy-saving mode (1am-5am)
 * 
 * Author: ECM Project
 * Date: 2026
 ******************************************************************************/

#include "config.h"
#include "bsp.h"
#include "rtc_soft.h"
#include "solar_mgr.h"
#include "app_control.h"

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
static void SystemInit(void);

/*******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************/
void main(void) {
    // Initialize all system modules
    SystemInit();
    
    // Main super-loop
    while(1) {
        // 1. Get current data
        Time_t now = RTC_GetTime();
        uint16_t light_level = BSP_GetLDR();
        
        // 2. Process solar logic and check for drift correction
        // Returns non-zero if a sync adjustment is needed
        int16_t drift = SOLAR_Update(light_level, now);
        
        if (drift != 0) {
            // Apply the drift correction to the RTC
            RTC_ApplySync(drift);
        }
        
        // 3. Apply application control rules
        // - Light control (with energy saving 1am-5am)
        APP_Task(now, SOLAR_IsDark());
        
        // 4. Update display (binary clock showing current hour)
        BSP_SetClockDisplay(now.hours);
        
        // 5. Optional: Toggle heartbeat LED for visual health check
        // (Could be done at a slower rate, e.g., once per second)
        // For simplicity, we'll leave it to demonstrate the loop is running
        static uint16_t heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 30000) {  // Adjust based on loop frequency
            BSP_ToggleHeartbeat();
            heartbeat_counter = 0;
        }
    }
}

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Initialize all system modules in correct order
 */
static void SystemInit(void) {
    // 1. Initialize hardware (BSP)
    BSP_Init();
    
    // 2. Get initial LDR reading for RTC calibration
    uint16_t initial_light = BSP_GetLDR();
    
    // 3. Initialize RTC with initial time estimate
    // If dark: assume midnight (00:00)
    // If bright: assume noon (12:00)
    RTC_Init(initial_light);
    
    // 4. Initialize solar manager state machine
    SOLAR_Init();
    
    // 5. Global interrupts are already enabled by BSP_Init()
    // The system is now ready to run
}
