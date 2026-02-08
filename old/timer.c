/*******************************************************************************
 * File: timer.c
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: Hardware timer driver implementation.
 *              Owns the Timer0 peripheral and the system tick ISR. The only
 *              variable modified in the ISR is the tick counter; it is
 *              volatile so the compiler does not optimize away reads in main.
 ******************************************************************************/

#include <xc.h>
#include "timer.h"
#include "config.h"

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * Tick counter incremented in the ISR. We use volatile because it is modified
 * in an interrupt and read in main loop; without volatile the compiler might
 * cache the value and never see updates.
 */
static volatile uint32_t s_tick_count = 0;

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINE
 ******************************************************************************/

/**
 * Single interrupt vector for the application. We only handle Timer0 here so
 * that all timer-related code stays in the timer driver. Clear the flag,
 * reload the timer for the next period, then increment the tick count.
 */
void __interrupt() ISR(void) {
    if (PIR0bits.TMR0IF) {
        PIR0bits.TMR0IF = 0;  /* Clear interrupt flag so we don't re-enter */

        /* Reload 0x0BDB for 1 s at 64 MHz (see config.h). */
        TMR0H = TMR0_RELOAD_HIGH;
        TMR0L = TMR0_RELOAD_LOW;

        /* One tick elapsed; main loop uses this to drive timekeeping. */
        s_tick_count++;
    }
}

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Initialize Timer0 for periodic interrupts.
 *
 * Timer is configured for 16-bit mode with prescaler 1:256. Reload values
 * come from config.h (different for TEST_MODE vs production). We enable
 * the Timer0 interrupt and global interrupts so the ISR runs on overflow.
 */
void Timer_Init(void) {
    /* Disable timer while configuring. */
    T0CON0bits.T0EN = 0;

    T0CON0bits.T016BIT = 1;   /* 16-bit mode */
    T0CON1bits.T0CS = 0b010;  /* Clock source = Fosc/4 */
    T0CON1bits.T0ASYNC = 0;   /* Synchronized to system clock */
    T0CON1bits.T0CKPS = 0b1000;  /* Prescaler 1:256 */

    /* Load initial value for first period. */
    TMR0H = TMR0_RELOAD_HIGH;
    TMR0L = TMR0_RELOAD_LOW;

    /* Enable Timer0 interrupt and global interrupt. */
    PIR0bits.TMR0IF = 0;
    PIE0bits.TMR0IE = 1;
    INTCONbits.PEIE = 1;   /* Peripheral interrupt enable (Timer0, etc.) */
    INTCONbits.GIE = 1;    /* Global interrupt enable */

    /* Start the timer. */
    T0CON0bits.T0EN = 1;
}

uint32_t Timer_GetTicks(void) {
    uint32_t ticks;
    uint8_t gie_save;

    gie_save = INTCONbits.GIE;
    INTCONbits.GIE = 0;   /* Disable interrupts for atomic 32-bit read */
    ticks = s_tick_count;
    INTCONbits.GIE = gie_save;
    return ticks;
}

void Timer_ResetTicks(void) {
    uint8_t gie_save;

    gie_save = INTCONbits.GIE;
    INTCONbits.GIE = 0;   /* Disable interrupts for atomic 32-bit write */
    s_tick_count = 0;
    INTCONbits.GIE = gie_save;
}
