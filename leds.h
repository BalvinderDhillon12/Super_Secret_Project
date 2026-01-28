#ifndef LEDS_H
#define LEDS_H

#include "config.h"

void LEDs_Init(void);
void LEDs_UpdateHourDisplay(uint8_t hours);
void LEDs_SetOutsideLight(bool state);
void LEDs_SetStatus(uint8_t index, bool state);

#endif // LEDS_H

