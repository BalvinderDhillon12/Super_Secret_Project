/*******************************************************************************
 * File:   Main.c
 * Purpose: Solar-synchronized streetlight controller
 ******************************************************************************/
#include <xc.h>
#include "Config.h"
#include "Timer.h"
#include "ADC.h"
#include "LEDS.h"
#include <stdbool.h>

// PIC Configuration
#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000


/** Solar state: unknown until we see clear dark or light (hysteresis). */
typedef enum {
    SOLAR_STATE_UNKNOWN,
    SOLAR_STATE_DAY,
    SOLAR_STATE_NIGHT
} SolarState_t;

static uint8_t g_hours = 0;
static uint8_t g_minutes = 0;
static uint8_t g_seconds = 0;

static SolarState_t g_solar_state = SOLAR_STATE_UNKNOWN;
static bool g_is_dark = false;
static uint16_t g_dusk_time = 0;
static uint16_t g_dawn_time = 0;
static bool g_dusk_recorded = false;
static uint8_t g_target_solar_midnight = 0;   /* Target solar midnight in minutes (0 or 60) */


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

    /* Solar state: settle UNKNOWN from first reading (hysteresis). */
    if (initial_light < LDR_THRESHOLD_DUSK) {
        g_solar_state = SOLAR_STATE_NIGHT;
        g_is_dark = true;
    } else if (initial_light > LDR_THRESHOLD_DAWN) {
        g_solar_state = SOLAR_STATE_DAY;
        g_is_dark = false;
    } else {
        g_solar_state = SOLAR_STATE_UNKNOWN;
        g_is_dark = false;
    }

    uint32_t last_sensor = Timer_GetTicks();
    uint32_t last_heartbeat = Timer_GetTicks();
    uint32_t last_tick = Timer_GetTicks();

    while (1) {
        uint32_t now = Timer_GetTicks();

        /* Timekeeping: advance clock every TICKS_PER_SECOND (same in TEST_MODE and production). */
        if (now - last_tick >= (uint32_t)TICKS_PER_SECOND) {
            last_tick += (uint32_t)TICKS_PER_SECOND;
            AdvanceTimeOneSecond();
        }

        LEDs_SetClockDisplay(g_hours);
    


        /* Sensor: every virtual second in test, every 60 s in production. */
#ifdef TEST_MODE
        uint32_t sensor_interval = (uint32_t)TICKS_PER_SECOND;
#else
        uint32_t sensor_interval = 60;
#endif
        if ((now - last_sensor) >= sensor_interval) {
            last_sensor = now;

            uint16_t light = ADC_ReadLDR();

            switch (g_solar_state) {
                case SOLAR_STATE_UNKNOWN:
                    if (light < LDR_THRESHOLD_DUSK) {
                        g_solar_state = SOLAR_STATE_NIGHT;
                        g_is_dark = true;
                    } else if (light > LDR_THRESHOLD_DAWN) {
                        g_solar_state = SOLAR_STATE_DAY;
                        g_is_dark = false;
                    }
                    break;

                case SOLAR_STATE_DAY:
                    g_is_dark = false;
                    if (light < LDR_THRESHOLD_DUSK) {
                        g_solar_state = SOLAR_STATE_NIGHT;
                        g_is_dark = true;
                        g_dusk_time = GetTotalMinutes();
                        g_dusk_recorded = true;
                    }
                    break;

                case SOLAR_STATE_NIGHT:
                    g_is_dark = true;
                    if (light > LDR_THRESHOLD_DAWN) {
                        g_solar_state = SOLAR_STATE_DAY;
                        g_is_dark = false;
                        g_dawn_time = GetTotalMinutes();

                        if (g_dusk_recorded) {
                            uint16_t night_duration;
                            if (g_dawn_time >= g_dusk_time) {
                                night_duration = g_dawn_time - g_dusk_time;
                            } else {
                                night_duration = (MINUTES_PER_DAY - g_dusk_time) + g_dawn_time;
                            }

                            uint16_t day_duration = MINUTES_PER_DAY - night_duration;
                            uint8_t day_hours = day_duration / 60;

                            if (day_hours > DAY_LENGTH_SUMMER_MIN) {
                                g_target_solar_midnight = 60;
                            } else if (day_hours < DAY_LENGTH_WINTER_MAX) {
                                g_target_solar_midnight = 0;
                            }

                            uint16_t half_night = night_duration / 2;
                            uint16_t solar_midnight = g_dusk_time + half_night;
                            if (solar_midnight >= MINUTES_PER_DAY) {
                                solar_midnight -= MINUTES_PER_DAY;
                            }

                            int16_t drift = (int16_t)solar_midnight - (int16_t)g_target_solar_midnight;
                            if (drift > 720) {
                                drift -= MINUTES_PER_DAY;
                            } else if (drift < -720) {
                                drift += MINUTES_PER_DAY;
                            }
                            if (drift > 2 || drift < -2) {
                                AdjustClock(-drift);
                            }
                            g_dusk_recorded = false;
                        }
                    }
                    break;
            }
        }
        
        
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







        
