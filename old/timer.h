/*******************************************************************************
 * File: timer.h
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: Hardware timer driver interface.
 *              Sets up the system tick timer and owns the interrupt service
 *              routine (ISR) that increments the tick counter. Configured for
 *              64 MHz and 1 s tick (reload 0x0BDB) per config.h.
 *              Application code uses Timer_GetTicks() to drive timekeeping
 *              (e.g. one "second" every TICKS_PER_SECOND ticks).
 ******************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize the hardware timer for periodic system ticks.
 *
 * Configures Timer0 and enables its interrupt. The ISR increments an internal
 * tick counter. In production TICKS_PER_SECOND is 1 (one tick per second); in
 * TEST_MODE it is 3600 (3600 ticks per virtual second).
 */
void Timer_Init(void);

/**
 * @brief Return the current tick count.
 *
 * The count is incremented in the Timer0 ISR. Use it together with
 * TICKS_PER_SECOND (from config.h) to detect elapsed seconds: e.g. when
 * (current_ticks - last_ticks) >= TICKS_PER_SECOND, one second has elapsed.
 * We return uint32_t so the counter can run for many days without rollover
 * at 1Hz; in test mode 3600 ticks per second still fits.
 *
 * @return uint32_t Current tick count (volatile semantics: read from ISR-updated variable)
 */
uint32_t Timer_GetTicks(void);

/**
 * @brief Reset the tick counter to zero.
 *
 * Optional; use if the application needs to synchronize or recalibrate.
 */
void Timer_ResetTicks(void);

#endif /* TIMER_H */
