#ifndef TIMERS_H
#define TIMERS_H

#include "config.h"

void Timers_Init(void);
volatile bool Timers_IsTickPending(void);
void Timers_ClearTick(void);

#endif // TIMERS_H

