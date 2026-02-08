/*******************************************************************************
 * File:   Main.c
 * Purpose: Solar-synchronized streetlight controller
 ******************************************************************************/
#include <xc.h>
#include "Config.h"
#include "Timer.h"
#include "ADC.h"
#include "LEDS.h"
#include "LCD.h" 
#include <stdbool.h>

// PIC Configuration
#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000


 // GLOBAL 

static uint8_t g_hours = 0;
static uint8_t g_minutes = 0;
static uint8_t g_seconds = 0;

static bool g_is_dark = false;
static uint16_t g_dusk_time = 0;
static uint16_t g_dawn_time = 0;
static bool g_dusk_recorded = false;
static uint8_t g_target_solar_midnight = 0;


 // Convert current time to total minutes since midnight

static uint16_t GetTotalMinutes(void) {
    return (uint16_t)(g_hours * 60u + g_minutes);
}


 // Adjust clock by specified number of minutes (handles wrap-around)

static void AdjustClock(int16_t minutes_to_adjust) {
    int16_t total_minutes = (int16_t)(g_hours * 60 + g_minutes);
    total_minutes += minutes_to_adjust;
    
    // Handle wrap-around
    while (total_minutes < 0) {
        total_minutes += MINUTES_PER_DAY;
    }
    while (total_minutes >= MINUTES_PER_DAY) {
        total_minutes -= MINUTES_PER_DAY;
    }
    
    g_hours = (uint8_t)(total_minutes / 60);
    g_minutes = (uint8_t)(total_minutes % 60);
    g_seconds = 0;
}


 //  Advance time by one second 
 
static void AdvanceTimeOneSecond(void) {
    g_seconds++;
    if (g_seconds >= SECONDS_PER_MINUTE) {
        g_seconds = 0;
        g_minutes++;
        if (g_minutes >= MINUTES_PER_HOUR) {
            g_minutes = 0;
            g_hours++;
            if (g_hours >= HOURS_PER_DAY) {
                g_hours = 0;
            }
        }
    }
}


// MAIN PROGRAM

void main(void) {
    // Initialize hardware
    LEDs_Init();
    ADC_Init();
    Timer_Init();
    //LCD_Init(); 
    
    // Set initial time based on light level
    uint16_t initial_light = ADC_ReadLDR();
    if (initial_light < LDR_THRESHOLD_DUSK) {
        g_hours = 0;    // Dark = midnight
        g_minutes = 0;
    } else {
        g_hours = 12;   // Light = noon
        g_minutes = 0;
    }
    g_seconds = 0;
    g_target_solar_midnight = 0;
    
   // uint32_t last_second = Timer_GetTicks();
    uint32_t last_sensor = Timer_GetTicks();
    uint32_t last_heartbeat = Timer_GetTicks();
    uint32_t last_tick = Timer_GetTicks(); 
    
    while (1) {
        uint32_t now = Timer_GetTicks();

        // Timekeeping
        #ifdef TEST_MODE
            // TEST MODE: 1 tick = 1 hour (24 seconds = 1 day)
            if (now - last_tick >= TICKS_PER_HOUR) {
                last_tick += TICKS_PER_HOUR;

                // Advance by 1 hour
                g_hours++;
                if (g_hours >= HOURS_PER_DAY) {
                    g_hours = 0;
                }
                // Keep minutes and seconds at 0 in test mode
                g_minutes = 0;
                g_seconds = 0;
            }
        #else
         
        
        #endif

            LEDs_SetClockDisplay(g_hours);

            // Light sensor value (shared between sensor check and LCD update)
            static uint16_t light = 512;  

            uint32_t sensor_interval = TICKS_PER_HOUR;
            #ifndef TEST_MODE
                sensor_interval = 60;
            #endif

            if ((now - last_sensor) >= sensor_interval) {
                last_sensor = now;

                light = ADC_ReadLDR();  // Update light value (no uint16_t here!)
                bool currently_dark = (light < LDR_THRESHOLD_DUSK);

            // DUSK: Transition from light to dark
            if (!g_is_dark && currently_dark) {
                g_dusk_time = GetTotalMinutes();
                g_dusk_recorded = true;
            }
            
            // DAWN: Transition from dark to light
            if (g_is_dark && !currently_dark) {
                g_dawn_time = GetTotalMinutes();
                
                if (g_dusk_recorded) {
                    // Calculate night duration
                    uint16_t night_duration;
                    if (g_dawn_time >= g_dusk_time) {
                        night_duration = g_dawn_time - g_dusk_time;
                    } else {
                        night_duration = (MINUTES_PER_DAY - g_dusk_time) + g_dawn_time;
                    }
                    
                    // Calculate day length for season detection
                    uint16_t day_duration = MINUTES_PER_DAY - night_duration;
                    uint8_t day_hours = day_duration / 60;
                    
                    // Detect season
                    if (day_hours > DAY_LENGTH_SUMMER_MIN) {
                        g_target_solar_midnight = 60;  // Summer: 1:00am
                    } else if (day_hours < DAY_LENGTH_WINTER_MAX) {
                        g_target_solar_midnight = 0;   // Winter: 0:00am
                    }
                    
                    // Calculate solar midnight (midpoint of night)
                    uint16_t half_night = night_duration / 2;
                    uint16_t solar_midnight = g_dusk_time + half_night;
                    if (solar_midnight >= MINUTES_PER_DAY) {
                        solar_midnight -= MINUTES_PER_DAY;
                    }
                    
                    // Calculate drift
                    int16_t drift = (int16_t)solar_midnight - (int16_t)g_target_solar_midnight;
                    
                    // Handle wrap-around
                    if (drift > 720) {
                        drift -= MINUTES_PER_DAY;
                    } else if (drift < -720) {
                        drift += MINUTES_PER_DAY;
                    }
                    
                    // Adjust if drift >2 minutes
                    if (drift > 2 || drift < -2) {
                        AdjustClock(-drift);
                    }
                    
                    g_dusk_recorded = false;
                }
            }
            
            g_is_dark = currently_dark;
        }
        
        // LCD 
        static uint8_t last_displayed_second = 0xFF;
        static uint8_t last_displayed_hour = 0xFF;  

        #ifdef TEST_MODE
            // In test mode, update every hour change
            if (g_hours != last_displayed_hour) {
                bool is_summer = (g_target_solar_midnight == 60);
                LCD_UpdateDisplay(g_hours, g_minutes, is_summer, light);
                last_displayed_hour = g_hours;
            }
        #else
            // In normal mode, update every second
            if (g_seconds != last_displayed_second) {
                bool is_summer = (g_target_solar_midnight == 60);
                LCD_UpdateDisplay(g_hours, g_minutes, is_summer, light);
                last_displayed_second = g_seconds;
            }
        #endif
         // LED CONTROL
         
        // Energy saving: turn off between 1am-5am
        bool in_save_window = (g_hours >= ENERGY_SAVE_START_HOUR && 
                               g_hours < ENERGY_SAVE_END_HOUR);
        
        // Main light: on if dark AND not in energy-save window
        bool light_on = g_is_dark && !in_save_window;
        LEDs_SetMainLight(light_on);
        
        // Heartbeat: toggle every 2 seconds
        if ((now - last_heartbeat) >= (TICKS_PER_SECOND * 2)) {
            last_heartbeat = now;
            LEDs_ToggleHeartbeat();
        }
        
        // Small delay to prevent loop from running at max speed
        __delay_ms(10);
    }
}







        
