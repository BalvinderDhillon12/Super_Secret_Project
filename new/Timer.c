/*******************************************************************************
 * File:   Timer.c
 * Purpose: System tick timer; generates periodic interrupts. Tick period
 *          and TICKS_PER_SECOND are defined in Config.h (TEST_MODE vs production).
 ******************************************************************************/

#include <xc.h>
#include "Timer.h"
#include "Config.h"

static volatile uint32_t s_tick_count = 0;
void __interrupt() ISR(void) {
    if (PIR0bits.TMR0IF) {
        PIR0bits.TMR0IF = 0;  // Clear interrupt flag so we don't re-enter 

        /* Reload for next period (values from Config.h). */
        TMR0H = TMR0_RELOAD_HIGH;
        TMR0L = TMR0_RELOAD_LOW;

        // One tick elapsed
        s_tick_count++;
    }
}

void Timer_Init(void) {
    // Disable timer while configuring. 
    T0CON0bits.T0EN = 0;

    T0CON0bits.T016BIT = 1;   // 16-bit mode 
    T0CON1bits.T0CS = 0b010;  // Fosc/4
    T0CON1bits.T0ASYNC = 1;   // Datasheet errata: needed for correct operation when Fosc/4 used
    T0CON1bits.T0CKPS = 0b1000;  /* Prescaler 1:256 */


    // Load initial value for first period. 
    TMR0H = TMR0_RELOAD_HIGH;
    TMR0L = TMR0_RELOAD_LOW;

    // Enable Timer0 interrupt and global interrupt. 
    PIR0bits.TMR0IF = 0;
    PIE0bits.TMR0IE = 1;
    INTCONbits.PEIE = 1;   // Peripheral interrupt enabled
    INTCONbits.GIE = 1;    // Global interrupt enabled

    // Timer started
    T0CON0bits.T0EN = 1;
}

uint32_t Timer_GetTicks(void) {
    uint32_t ticks;
    uint8_t gie_save;

    gie_save = INTCONbits.GIE;
    INTCONbits.GIE = 0;   /* Disable interrupts for atomic read */
    ticks = s_tick_count;
    INTCONbits.GIE = gie_save;
    return ticks;
}

void Timer_ResetTicks(void) {
    uint8_t gie_save;

    gie_save = INTCONbits.GIE;
    INTCONbits.GIE = 0;   // Disable interrupts for writing 
    s_tick_count = 0;
    INTCONbits.GIE = gie_save;
}