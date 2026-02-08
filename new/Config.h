/* 

 * File:   Config.h


 * Purpose:  

 */


#ifndef Config_H
#define Config_H

#include <stdint.h>
#include <stdbool.h>


 // test mode configuration 
#define TEST_MODE


#ifdef TEST_MODE
    /* Test mode: 3600 ticks = 1 virtual second (24 s real = 1 day). Prescaler 1:256. */
    #define TMR0_RELOAD_HIGH    0xFF
    #define TMR0_RELOAD_LOW     0xFC
    #define TICKS_PER_SECOND    3600   /* 3600 ticks = 1 virtual second */
#else
    /* Production: 1 tick = 1 real second. Prescaler 1:256, 64 MHz. */
    #define TMR0_RELOAD_HIGH    0x0B
    #define TMR0_RELOAD_LOW     0xDB
    #define TICKS_PER_SECOND    1      /* 1 tick = 1 real second */
#endif


#define LDR_THRESHOLD_DUSK      400     // Below this = transition to dark 
#define LDR_THRESHOLD_DAWN      600     // Above this = transition to light 

#define ENERGY_SAVE_START_HOUR  1       // 1am - turn light off 
#define ENERGY_SAVE_END_HOUR    5       // 5am - turn light back on 


#define SOLAR_MIDNIGHT_WINTER   0       // 00:00 (midnight) in winter 
#define SOLAR_MIDNIGHT_SUMMER   1       // 01:00 (1am) in summer (DST) 

//Day length thresholds for season detection (in hours) 
#define DAY_LENGTH_SUMMER_MIN   14      // > 14 hours = summer (DST active) 
#define DAY_LENGTH_WINTER_MAX   10     

#define SECONDS_PER_HOUR        3600  
#define SECONDS_PER_MINUTE      60
#define MINUTES_PER_HOUR        60
#define HOURS_PER_DAY           24
#define MINUTES_PER_DAY         1440    // 24 * 60 
#define ADC_LDR_CHANNEL 0x03

#endif 


