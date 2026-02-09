/* 

 * File:   Config.h


 * Purpose:  

 */
#define _XTAL_FREQ 64000000

#ifndef Config_H
#define Config_H

#include <stdint.h>
#include <stdbool.h>

 // test mode configuration 
#define TEST_MODE


#ifdef TEST_MODE
    #define TMR0_RELOAD_HIGH    0x0B
    #define TMR0_RELOAD_LOW     0xDB
    #define TICKS_PER_SECOND    1
    #define TICKS_PER_HOUR      1
    #define SENSOR_INTERVAL     TICKS_PER_HOUR
#else
    #define TMR0_RELOAD_HIGH    0x0B
    #define TMR0_RELOAD_LOW     0xDB
    #define TICKS_PER_SECOND    1       /* Normal: 1 tick = 1 real second */
    #define TICKS_PER_HOUR      3600    /* 1 hour = 3600 ticks */
    #define SENSOR_INTERVAL     60
#endif


/* LDR: calibrated delta used for binary day/night (see Main.c calibration) */

#define ENERGY_SAVE_START_HOUR  1       // 1am - turn light off 
#define ENERGY_SAVE_END_HOUR    5       // 5am - turn light back on 

/* Start date - change before flashing for DST/leap year demos */
#define START_YEAR   2026
#define START_MONTH  3     /* March - near DST spring-forward */
#define START_DAY    25    /* Last Sun Mar 2026 = Mar 29 */

#define SECONDS_PER_MINUTE      60
#define MINUTES_PER_HOUR        60
#define HOURS_PER_DAY           24
#define ADC_LDR_CHANNEL 0x03

#endif 


