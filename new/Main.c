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
#define NUM_SAMPLES 32
#define BLINK_MS 300

static uint16_t g_ldr_dark_value = 0;
static uint16_t g_ldr_delta = 10;

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

static uint16_t ReadLDR_Averaged(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        sum += ADC_ReadLDR();
        __delay_ms(2);
    }
    return (uint16_t)(sum / NUM_SAMPLES);
}

void main(void) {
    uint16_t light_value;

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
    g_ldr_dark_value = ReadLDR_Averaged();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

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
    Calendar_Init(START_YEAR, START_MONTH, START_DAY);
    g_dst_active = Calendar_IsDST();

    {
        uint16_t light = ADC_ReadLDR();
        g_is_dark = !(light > g_ldr_dark_value + g_ldr_delta || light + g_ldr_delta < g_ldr_dark_value);
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

            /* DST spring-forward: last Sun Mar, 1AM -> 2AM */
            if (g_hours == 1 && !g_dst_active &&
                Calendar_GetMonth() == 3 && Calendar_GetDay() == Calendar_LastSundayOfMarch()) {
                g_hours = 2;
                g_dst_active = true;
            }
            /* DST fall-back: last Sun Oct, 2AM -> 1AM */
            if (g_hours == 2 && g_dst_active && !g_dst_fall_back_done &&
                Calendar_GetMonth() == 10 && Calendar_GetDay() == Calendar_LastSundayOfOctober()) {
                g_hours = 1;
                g_dst_active = false;
                g_dst_fall_back_done = true;
            }
        }
#else
        if (now - last_tick >= TICKS_PER_SECOND) {
            last_tick += TICKS_PER_SECOND;
            AdvanceTimeOneSecond();

            /* DST spring-forward: last Sun Mar, 1AM -> 2AM */
            if (g_hours == 1 && g_minutes == 0 && !g_dst_active &&
                Calendar_GetMonth() == 3 && Calendar_GetDay() == Calendar_LastSundayOfMarch()) {
                g_hours = 2;
                g_minutes = 0;
                g_seconds = 0;
                g_dst_active = true;
            }
            /* DST fall-back: last Sun Oct, 2AM -> 1AM */
            if (g_hours == 2 && g_minutes == 0 && g_dst_active && !g_dst_fall_back_done &&
                Calendar_GetMonth() == 10 && Calendar_GetDay() == Calendar_LastSundayOfOctober()) {
                g_hours = 1;
                g_minutes = 0;
                g_seconds = 0;
                g_dst_active = false;
                g_dst_fall_back_done = true;
            }
        }
#endif

        LEDs_SetClockDisplay(g_hours);

        static uint16_t light = 512;
        uint32_t sensor_interval = TICKS_PER_HOUR;
#ifndef TEST_MODE
        sensor_interval = 60;
#endif

        if ((now - last_sensor) >= sensor_interval) {
            last_sensor = now;
            light = ReadLDR_Averaged();
            g_is_dark = !(light > g_ldr_dark_value + g_ldr_delta || light + g_ldr_delta < g_ldr_dark_value);
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
