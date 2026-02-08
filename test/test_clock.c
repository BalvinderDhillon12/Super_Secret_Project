/*******************************************************************************
 * File:   test_clock.c
 * Purpose: Standalone test - clock counts 0-23 on LEDs 1-5 only. No LDR, no LCD.
 *          In TEST_MODE, 1 tick = 1 hour (24 seconds = full day).
 ******************************************************************************/

#include <xc.h>
#include "../new/Config.h"
#include "../new/Timer.h"
#include "../new/LEDS.h"

#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000

int main(void) {
    uint8_t hour = 0;
    uint32_t last_tick;

    LEDs_Init();
    Timer_Init();

    last_tick = Timer_GetTicks();
    LEDs_SetClockDisplay(hour);

    for (;;) {
        uint32_t now = Timer_GetTicks();

        if ((now - last_tick) >= TICKS_PER_HOUR) {
            last_tick += TICKS_PER_HOUR;
            hour++;
            if (hour >= HOURS_PER_DAY) {
                hour = 0;
            }
            LEDs_SetClockDisplay(hour);
        }

        __delay_ms(10);
    }
}
