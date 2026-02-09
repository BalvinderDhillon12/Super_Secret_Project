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
#include "Calendar.h"
#include <stdbool.h>

// PIC Configuration
#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000
#define BLINK_MS 300

static uint16_t g_threshold = 512;      /* midpoint between dark and light */
static bool     g_dark_above = true;    /* true if dark ADC value > light ADC value */

static uint8_t g_hours = 0;
static uint8_t g_minutes = 0;
static uint8_t g_seconds = 0;

static bool g_is_dark = false;
static bool g_dst_active = false;
static bool g_dst_fall_back_done = false;

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
                Calendar_AdvanceDay();
                g_dst_fall_back_done = false;
            }
        }
    }
}

void main(void) {
    uint16_t dark_value, light_value;

    LEDs_Init();
    ADC_Init();
    Buttons_Init();
    LCD_Init();

    /* Two-step RF2 calibration: dark then light */
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
    dark_value = ADC_ReadLDR();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    LEDs_SetMainLight(1);
    while (Button_RF2_Read() == 0) {
    }
    __delay_ms(50);
    light_value = ADC_ReadLDR();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    g_threshold = (dark_value + light_value) / 2u;
    g_dark_above = (dark_value > light_value);

    Timer_Init();
    Calendar_Init(START_YEAR, START_MONTH, START_DAY);
    g_dst_active = Calendar_IsDST();

    {
        uint16_t light = ADC_ReadLDR();
        if (g_dark_above) {
            g_is_dark = (light <= g_threshold);
        } else {
            g_is_dark = (light >= g_threshold);
        }
        if (g_is_dark) {
            g_hours = 0;
            g_minutes = 0;
        } else {
            g_hours = 12;
            g_minutes = 0;
        }
        g_seconds = 0;
    }

    uint32_t last_sensor = Timer_GetTicks();
    uint32_t last_heartbeat = Timer_GetTicks();
    uint32_t last_tick = Timer_GetTicks();

    while (1) {
        uint32_t now = Timer_GetTicks();
        bool time_advanced = false;

#ifdef TEST_MODE
        if (now - last_tick >= TICKS_PER_HOUR) {
            last_tick += TICKS_PER_HOUR;
            g_hours++;
            if (g_hours >= HOURS_PER_DAY) {
                g_hours = 0;
                Calendar_AdvanceDay();
                g_dst_fall_back_done = false;
            }
            g_minutes = 0;
            g_seconds = 0;
            time_advanced = true;
        }
#else
        if (now - last_tick >= TICKS_PER_SECOND) {
            last_tick += TICKS_PER_SECOND;
            AdvanceTimeOneSecond();
            time_advanced = (g_minutes == 0);
        }
#endif

        if (time_advanced) {
            if (g_hours == 1 && !g_dst_active &&
                Calendar_GetMonth() == 3 && Calendar_GetDay() == Calendar_LastSundayOfMarch()) {
                g_hours = 2;
                g_minutes = 0;
                g_seconds = 0;
                g_dst_active = true;
            }
            if (g_hours == 2 && g_dst_active && !g_dst_fall_back_done &&
                Calendar_GetMonth() == 10 && Calendar_GetDay() == Calendar_LastSundayOfOctober()) {
                g_hours = 1;
                g_minutes = 0;
                g_seconds = 0;
                g_dst_active = false;
                g_dst_fall_back_done = true;
            }
        }

        LEDs_SetClockDisplay(g_hours);

        static uint16_t light = 512;

        if ((now - last_sensor) >= SENSOR_INTERVAL) {
            last_sensor = now;
            light = ADC_ReadLDR();
            if (g_dark_above) {
                g_is_dark = (light <= g_threshold);
            } else {
                g_is_dark = (light >= g_threshold);
            }
        }

        static uint8_t last_displayed_second = 0xFF;
        static uint8_t last_displayed_hour = 0xFF;

#ifdef TEST_MODE
        if (g_hours != last_displayed_hour) {
            LCD_UpdateDisplay(g_hours, g_minutes,
                             Calendar_GetDay(), Calendar_GetMonth(), Calendar_GetYear(),
                             g_dst_active);
            last_displayed_hour = g_hours;
        }
#else
        if (g_seconds != last_displayed_second) {
            LCD_UpdateDisplay(g_hours, g_minutes,
                             Calendar_GetDay(), Calendar_GetMonth(), Calendar_GetYear(),
                             g_dst_active);
            last_displayed_second = g_seconds;
        }
#endif

        bool in_save_window = (g_hours >= ENERGY_SAVE_START_HOUR &&
                               g_hours < ENERGY_SAVE_END_HOUR);
        bool light_on = g_is_dark && !in_save_window;
        LEDs_SetMainLight(light_on);

        if ((now - last_heartbeat) >= (TICKS_PER_SECOND * 2)) {
            last_heartbeat = now;
            LEDs_ToggleHeartbeat();
        }

        __delay_ms(10);
    }
}
