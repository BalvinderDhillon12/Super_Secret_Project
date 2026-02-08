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
#include "Buttons.h"
#include <stdbool.h>

// PIC Configuration
#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000
#define NUM_SAMPLES 32
#define BLINK_MS 300

 // GLOBAL 

static uint16_t g_ldr_dark_value = 0;
static uint16_t g_ldr_delta = 10;

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


static uint16_t ReadLDR_Averaged(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        sum += ADC_ReadLDR();
        __delay_ms(2);
    }
    return (uint16_t)(sum / NUM_SAMPLES);
}


// MAIN PROGRAM

void main(void) {
    uint16_t light_value;

    // Initialize hardware (timer started after calibration)
    LEDs_Init();
    ADC_Init();
    Buttons_Init();
    //LCD_Init();

    /* Two-step RF2 calibration: dark then light */
    /* Phase 1: Blink LED 9 - user covers LDR, then presses RF2 to save dark value */
    while (1) {
        LEDs_SetMainLight(1);
        __delay_ms(BLINK_MS);
        LEDs_SetMainLight(0);
        __delay_ms(BLINK_MS);
        if (Button_RF2_Read() == 1) {
            break;
        }
    }
    __delay_ms(50);
    g_ldr_dark_value = ReadLDR_Averaged();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    /* Phase 2: LED 9 solid ON - user exposes LDR, then presses RF2 to save light value */
    LEDs_SetMainLight(1);
    while (Button_RF2_Read() == 0) {
    }
    __delay_ms(50);
    light_value = ReadLDR_Averaged();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    if (g_ldr_dark_value > light_value)
        g_ldr_delta = (g_ldr_dark_value - light_value) / 2u;
    else
        g_ldr_delta = (light_value - g_ldr_dark_value) / 2u;
    if (g_ldr_delta < 10u) g_ldr_delta = 10u;

    Timer_Init();

    /* Set initial time from one sample using calibrated comparison */
    {
        uint16_t light = ADC_ReadLDR();
        bool currently_dark = !(light > g_ldr_dark_value + g_ldr_delta || light + g_ldr_delta < g_ldr_dark_value);
        if (currently_dark) {
            g_hours = 0;
            g_minutes = 0;
        } else {
            g_hours = 12;
            g_minutes = 0;
        }
    }
    g_seconds = 0;
    g_target_solar_midnight = 0;

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

                light = ADC_ReadLDR();
                bool currently_dark = !(light > g_ldr_dark_value + g_ldr_delta || light + g_ldr_delta < g_ldr_dark_value);

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







        
