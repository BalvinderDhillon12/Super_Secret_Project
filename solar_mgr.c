/*******************************************************************************
 * File: solar_mgr.c
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Solar Logic Manager Implementation
 ******************************************************************************/

#include "solar_mgr.h"
#include "config.h"

/*******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/

typedef enum {
    STATE_UNKNOWN,      // Initial state (uncalibrated)
    STATE_DAY,          // Daytime (bright)
    STATE_NIGHT         // Nighttime (dark)
} SolarState_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/
static SolarState_t current_state = STATE_UNKNOWN;
static bool is_dark = false;

// Timestamps for cycle tracking (in minutes since midnight)
static uint16_t dusk_time = 0;          // When we transitioned to night
static uint16_t dawn_time = 0;          // When we transitioned to day
static uint16_t last_dusk_time = 0;     // Previous dusk for day length calculation

// Cycle completion flags
static bool first_cycle_complete = false;
static bool dusk_recorded = false;

// Season detection (for DST)
static uint8_t target_solar_midnight = SOLAR_MIDNIGHT_WINTER;  // Default to winter

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
static void HandleDuskTransition(uint16_t current_minutes);
static void HandleDawnTransition(uint16_t current_minutes);
static int16_t CalculateDriftCorrection(uint16_t solar_midnight_calculated);
static void UpdateSeasonFromDayLength(uint16_t day_duration_minutes);

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

void SOLAR_Init(void) {
    current_state = STATE_UNKNOWN;
    is_dark = false;
    first_cycle_complete = false;
    dusk_recorded = false;
    target_solar_midnight = SOLAR_MIDNIGHT_WINTER;
}

int16_t SOLAR_Update(uint16_t ldr_value, Time_t now) {
    int16_t drift_correction = 0;
    uint16_t current_minutes = ((uint16_t)now.hours << 6) - 
                               ((uint16_t)now.hours << 2) + 
                               now.minutes;
    
    // State machine with hysteresis filtering
    switch (current_state) {
        case STATE_UNKNOWN:
            // Initial state - determine current condition
            if (ldr_value < LDR_THRESHOLD_DUSK) {
                current_state = STATE_NIGHT;
                is_dark = true;
            } else if (ldr_value > LDR_THRESHOLD_DAWN) {
                current_state = STATE_DAY;
                is_dark = false;
            }
            // Remain in UNKNOWN if in hysteresis band
            break;
            
        case STATE_DAY:
            is_dark = false;
            
            // Check for transition to night (dusk)
            if (ldr_value < LDR_THRESHOLD_DUSK) {
                current_state = STATE_NIGHT;
                is_dark = true;
                HandleDuskTransition(current_minutes);
            }
            break;
            
        case STATE_NIGHT:
            is_dark = true;
            
            // Check for transition to day (dawn)
            if (ldr_value > LDR_THRESHOLD_DAWN) {
                current_state = STATE_DAY;
                is_dark = false;
                drift_correction = HandleDawnTransition(current_minutes);
            }
            break;
    }
    
    return drift_correction;
}

bool SOLAR_IsDark(void) {
    return is_dark;
}

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

static void HandleDuskTransition(uint16_t current_minutes) {
    // Record the time of dusk
    dusk_time = current_minutes;
    dusk_recorded = true;
    
    // If we have a previous dusk time, calculate day duration
    if (first_cycle_complete && last_dusk_time != 0) {
        uint16_t day_duration_minutes;
        
        // Calculate time between consecutive dusks (24 hours)
        if (dusk_time >= last_dusk_time) {
            day_duration_minutes = dusk_time - last_dusk_time;
        } else {
            // Wrapped around midnight
            day_duration_minutes = (MINUTES_PER_DAY - last_dusk_time) + dusk_time;
        }
        
        // This gives us the period, but we need the daylight duration
        // For now, we'll update this when we have both dusk and dawn
    }
    
    last_dusk_time = dusk_time;
}

static int16_t HandleDawnTransition(uint16_t current_minutes) {
    int16_t correction = 0;
    
    dawn_time = current_minutes;
    
    // We need a valid dusk time to calculate anything meaningful
    if (!dusk_recorded) {
        return 0;
    }
    
    // Calculate night duration
    uint16_t night_duration;
    if (dawn_time >= dusk_time) {
        night_duration = dawn_time - dusk_time;
    } else {
        // Dawn occurred after midnight rollover
        night_duration = (MINUTES_PER_DAY - dusk_time) + dawn_time;
    }
    
    // Calculate day duration (time from last dawn to this dusk)
    // For simplicity, estimate as 24 hours - night duration
    uint16_t day_duration = MINUTES_PER_DAY - night_duration;
    
    // Update season detection based on day length
    UpdateSeasonFromDayLength(day_duration);
    
    // Calculate solar midnight (midpoint of night)
    // Solar midnight = dusk_time + (night_duration / 2)
    uint16_t half_night = night_duration >> 1;  // Divide by 2 using shift
    uint16_t solar_midnight_calculated = dusk_time + half_night;
    
    // Handle wraparound past midnight
    if (solar_midnight_calculated >= MINUTES_PER_DAY) {
        solar_midnight_calculated -= MINUTES_PER_DAY;
    }
    
    // Calculate drift correction
    correction = CalculateDriftCorrection(solar_midnight_calculated);
    
    // Mark that we've completed at least one full cycle
    first_cycle_complete = true;
    
    return correction;
}

static int16_t CalculateDriftCorrection(uint16_t solar_midnight_calculated) {
    // Target solar midnight in minutes
    uint16_t target_midnight_minutes = target_solar_midnight * MINUTES_PER_HOUR;
    
    // Calculate error
    int16_t error;
    
    // Handle wraparound cases
    int16_t diff = (int16_t)solar_midnight_calculated - (int16_t)target_midnight_minutes;
    
    // Find shortest path around the circle (24-hour clock)
    if (diff > 720) {  // More than 12 hours ahead
        error = diff - MINUTES_PER_DAY;
    } else if (diff < -720) {  // More than 12 hours behind
        error = diff + MINUTES_PER_DAY;
    } else {
        error = diff;
    }
    
    // Only apply correction if first cycle is complete
    // (avoid large snap on uncalibrated startup)
    if (!first_cycle_complete) {
        // On first sync, apply full correction to snap into place
        return -error;  // Negative because we subtract the error
    }
    
    // For subsequent corrections, apply incrementally
    // Apply correction if error exceeds 2 minutes (configurable threshold)
    if (error > 2 || error < -2) {
        // Apply full correction (could be limited to prevent large jumps)
        return -error;
    }
    
    return 0;  // No correction needed
}

static void UpdateSeasonFromDayLength(uint16_t day_duration_minutes) {
    // Convert to hours for comparison
    // day_duration_hours = day_duration_minutes / 60
    // Use subtraction to avoid division
    uint8_t day_duration_hours = 0;
    uint16_t remaining = day_duration_minutes;
    
    while (remaining >= MINUTES_PER_HOUR) {
        remaining -= MINUTES_PER_HOUR;
        day_duration_hours++;
    }
    
    // DST Logic based on day length
    if (day_duration_hours > DAY_LENGTH_SUMMER_MIN) {
        // Long days = Summer = DST active
        target_solar_midnight = SOLAR_MIDNIGHT_SUMMER;  // 1am
    } else if (day_duration_hours < DAY_LENGTH_WINTER_MAX) {
        // Short days = Winter = DST inactive
        target_solar_midnight = SOLAR_MIDNIGHT_WINTER;  // Midnight
    }
    // If between 10-14 hours, maintain current setting (spring/autumn transition)
}
