/*******************************************************************************
 * File:   Buttons.h
 * Description: Button driver for RF2 push button (1 when pressed).
 ******************************************************************************/

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

/** Configure RF2 as digital input for the push button. */
void Buttons_Init(void);

/** Returns 1 when RF2 is pressed, 0 when released. */
uint8_t Button_RF2_Read(void);

/** Block until RF2 is pressed (with debounce), then until released. Single clean press. */
void Button_RF2_WaitPress(void);

#endif /* BUTTONS_H */
