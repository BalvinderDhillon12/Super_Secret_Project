/*******************************************************************************
 * File: main.c
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 * Compiler: XC8 (C99)
 *
 * Description: Main entry point and super-loop.
 *              Contains the infinite loop, timekeeping state variables,
 *              solar sync logic (dusk/dawn detection and drift correction),
 *              and energy-saving rules (1am-5am). Hardware is accessed only
 *              through the ADC, Timer, and LEDs drivers.
 ******************************************************************************/

#include <xc.h>
#include "config.h"
#include "timer.h"
#include "adc.h"
#include "leds.h"

/*******************************************************************************
 * CONFIGURATION BITS (PIC18F67K40) - Lab verified hardware settings
 ******************************************************************************/
#pragma config FEXTOSC = HS     // External Oscillator mode (Crystal > 8MHz)
#pragma config RSTOSC = EXTOSC_4PLL // Power-up default: EXTOSC with 4x PLL
#pragma config WDTE = OFF       // Watchdog Timer Disabled

#define _XTAL_FREQ 64000000

/*******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/

/** Time of day: hours (0-23), minutes (0-59), seconds (0-59). */
typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} Time_t;

/** Solar state machine: unknown until we see clear dark or light. */
typedef enum {
    SOLAR_STATE_UNKNOWN,
    SOLAR_STATE_DAY,
    SOLAR_STATE_NIGHT
} SolarState_t;

/*******************************************************************************
 * TIMEKEEPING STATE (application-owned; not in ISR)
 ******************************************************************************/
static uint8_t g_hours = 0;
static uint8_t g_minutes = 0;
static uint8_t g_seconds = 0;
/** Last tick count when we advanced time; used to detect TICKS_PER_SECOND elapsed. */
static uint32_t g_last_tick = 0;

/*******************************************************************************
 * SOLAR STATE (dusk/dawn detection and drift correction)
 ******************************************************************************/
static SolarState_t g_solar_state = SOLAR_STATE_UNKNOWN;
static bool g_is_dark = false;
static uint16_t g_dusk_time = 0;
static uint16_t g_dawn_time = 0;
static uint16_t g_last_dusk_time = 0;
static bool g_first_cycle_complete = false;
static bool g_dusk_recorded = false;
static uint8_t g_target_solar_midnight = SOLAR_MIDNIGHT_WINTER;

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES - Timekeeping
 ******************************************************************************/
static Time_t GetTime(void);
static uint16_t GetTotalMinutes(void);
static void SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
static void AdvanceTimeOneSecond(void);
static void ApplySync(int16_t adjustment_min);

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES - Solar
 ******************************************************************************/
static void HandleDuskTransition(uint16_t current_minutes);
static int16_t HandleDawnTransition(uint16_t current_minutes);
static int16_t CalculateDriftCorrection(uint16_t solar_midnight_calculated);
static void UpdateSeasonFromDayLength(uint16_t day_duration_minutes);
static int16_t SolarUpdate(uint16_t ldr_value, Time_t now);

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES - Application
 ******************************************************************************/
static bool IsInEnergySaveWindow(uint8_t hour);
static void SystemInit(void);

/*******************************************************************************
 * MAIN
 ******************************************************************************/
void main(void) {
    SystemInit();

    while (1) {
        uint32_t now_ticks = Timer_GetTicks();

        /* 1. Timekeeping: advance clock when TICKS_PER_SECOND have elapsed.
         * In production that is 1 tick = 1 second; in TEST_MODE, 3600 ticks = 1 second. */
        if (now_ticks - g_last_tick >= (uint32_t)TICKS_PER_SECOND) {
            g_last_tick += (uint32_t)TICKS_PER_SECOND;
            AdvanceTimeOneSecond();
        }

        Time_t now = GetTime();
        uint16_t light_level = ADC_ReadLDR();

        /* 2. Solar logic: state machine and drift correction at dawn. */
        int16_t drift = SolarUpdate(light_level, now);
        if (drift != 0) {
            ApplySync(drift);
        }

        /* 3. Energy-saving rule: light ON only if dark AND not in 1am-5am window. */
        bool light_on = g_is_dark && !IsInEnergySaveWindow(now.hours);
        LEDs_SetMainLight(light_on);

        /* 4. Binary clock display shows current hour. */
        LEDs_SetClockDisplay(now.hours);

        /* 5. Heartbeat LED toggled every many iterations to show loop is running. */
        static uint16_t heartbeat_counter = 0;
        heartbeat_counter++;
        if (heartbeat_counter >= 30000u) {
            LEDs_ToggleHeartbeat();
            heartbeat_counter = 0;
        }
    }
}

/*******************************************************************************
 * TIMEKEEPING IMPLEMENTATIONS
 ******************************************************************************/

static Time_t GetTime(void) {
    Time_t t;
    t.hours = g_hours;
    t.minutes = g_minutes;
    t.seconds = g_seconds;
    return t;
}

static uint16_t GetTotalMinutes(void) {
    /* hours * 60 + minutes without multiplication: (hours << 6) - (hours << 2) + minutes */
    return ((uint16_t)g_hours << 6) - ((uint16_t)g_hours << 2) + g_minutes;
}

static void SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (hours >= HOURS_PER_DAY)   hours = 0;
    if (minutes >= MINUTES_PER_HOUR) minutes = 0;
    if (seconds >= SECONDS_PER_MINUTE) seconds = 0;
    g_hours = hours;
    g_minutes = minutes;
    g_seconds = seconds;
}

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

static void ApplySync(int16_t adjustment_min) {
    /* Total minutes: (hours << 6) - (hours << 2) + minutes, no multiplication */
    int16_t total = (int16_t)(((uint16_t)g_hours << 6) - ((uint16_t)g_hours << 2) + g_minutes);
    total += adjustment_min;
    while (total < 0) total += (int16_t)MINUTES_PER_DAY;
    while (total >= (int16_t)MINUTES_PER_DAY) total -= (int16_t)MINUTES_PER_DAY;

    uint8_t new_hours = 0;
    uint16_t m = (uint16_t)total;
    while (m >= MINUTES_PER_HOUR) {
        m -= MINUTES_PER_HOUR;
        new_hours++;
    }
    g_hours = new_hours;
    g_minutes = (uint8_t)m;
    g_seconds = 0;
}

/*******************************************************************************
 * SOLAR LOGIC IMPLEMENTATIONS
 ******************************************************************************/

static void HandleDuskTransition(uint16_t current_minutes) {
    g_dusk_time = current_minutes;
    g_dusk_recorded = true;
    g_last_dusk_time = g_dusk_time;
}

static int16_t HandleDawnTransition(uint16_t current_minutes) {
    int16_t correction = 0;
    g_dawn_time = current_minutes;

    if (!g_dusk_recorded) return 0;

    uint16_t night_duration;
    if (g_dawn_time >= g_dusk_time) {
        night_duration = g_dawn_time - g_dusk_time;
    } else {
        night_duration = (MINUTES_PER_DAY - g_dusk_time) + g_dawn_time;
    }

    uint16_t day_duration = MINUTES_PER_DAY - night_duration;
    UpdateSeasonFromDayLength(day_duration);

    uint16_t half_night = night_duration >> 1;
    uint16_t solar_midnight = g_dusk_time + half_night;
    if (solar_midnight >= MINUTES_PER_DAY) solar_midnight -= MINUTES_PER_DAY;

    correction = CalculateDriftCorrection(solar_midnight);
    g_first_cycle_complete = true;
    return correction;
}

static int16_t CalculateDriftCorrection(uint16_t solar_midnight_calculated) {
    uint16_t target_min = (uint16_t)g_target_solar_midnight * MINUTES_PER_HOUR;
    int16_t diff = (int16_t)solar_midnight_calculated - (int16_t)target_min;

    int16_t error;
    if (diff > 720) {
        error = diff - (int16_t)MINUTES_PER_DAY;
    } else if (diff < -720) {
        error = diff + (int16_t)MINUTES_PER_DAY;
    } else {
        error = diff;
    }

    if (error > 2 || error < -2) {
        return (int16_t)(-error);
    }
    return 0;
}

static void UpdateSeasonFromDayLength(uint16_t day_duration_minutes) {
    uint8_t day_hours = 0;
    uint16_t rem = day_duration_minutes;
    while (rem >= MINUTES_PER_HOUR) {
        rem -= MINUTES_PER_HOUR;
        day_hours++;
    }
    if (day_hours > DAY_LENGTH_SUMMER_MIN) {
        g_target_solar_midnight = SOLAR_MIDNIGHT_SUMMER;
    } else if (day_hours < DAY_LENGTH_WINTER_MAX) {
        g_target_solar_midnight = SOLAR_MIDNIGHT_WINTER;
    }
}

static int16_t SolarUpdate(uint16_t ldr_value, Time_t now) {
    int16_t drift = 0;
    uint16_t current_minutes = ((uint16_t)now.hours << 6) - ((uint16_t)now.hours << 2) + now.minutes;

    switch (g_solar_state) {
        case SOLAR_STATE_UNKNOWN:
            if (ldr_value < LDR_THRESHOLD_DUSK) {
                g_solar_state = SOLAR_STATE_NIGHT;
                g_is_dark = true;
            } else if (ldr_value > LDR_THRESHOLD_DAWN) {
                g_solar_state = SOLAR_STATE_DAY;
                g_is_dark = false;
            }
            break;

        case SOLAR_STATE_DAY:
            g_is_dark = false;
            if (ldr_value < LDR_THRESHOLD_DUSK) {
                g_solar_state = SOLAR_STATE_NIGHT;
                g_is_dark = true;
                HandleDuskTransition(current_minutes);
            }
            break;

        case SOLAR_STATE_NIGHT:
            g_is_dark = true;
            if (ldr_value > LDR_THRESHOLD_DAWN) {
                g_solar_state = SOLAR_STATE_DAY;
                g_is_dark = false;
                drift = HandleDawnTransition(current_minutes);
            }
            break;
    }
    return drift;
}

/*******************************************************************************
 * APPLICATION RULES
 ******************************************************************************/

/** Energy saving: 1am-5am we turn the light off even when dark. */
static bool IsInEnergySaveWindow(uint8_t hour) {
    return (hour >= ENERGY_SAVE_START_HOUR && hour < ENERGY_SAVE_END_HOUR);
}

/*******************************************************************************
 * SYSTEM INIT
 ******************************************************************************/
static void SystemInit(void) {
    LEDs_Init();
    ADC_Init();
    Timer_Init();

    /* Initial time estimate from LDR: dark -> midnight, bright -> noon. */
    uint16_t initial_light = ADC_ReadLDR();
    if (initial_light < LDR_THRESHOLD_DUSK) {
        SetTime(0, 0, 0);
    } else {
        SetTime(12, 0, 0);
    }

    /* Solar state starts unknown; will settle on first clear dark/dawn. */
    g_solar_state = SOLAR_STATE_UNKNOWN;
    g_is_dark = false;
    g_first_cycle_complete = false;
    g_dusk_recorded = false;
    g_target_solar_midnight = SOLAR_MIDNIGHT_WINTER;

    /* Sync last_tick so we don't advance time immediately. */
    g_last_tick = Timer_GetTicks();
}
