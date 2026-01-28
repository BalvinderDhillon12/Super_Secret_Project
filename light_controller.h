#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include "timekeeping.h"

void LightController_Update(uint16_t ldr_value, DateTime_t *time);

#endif // LIGHT_CONTROLLER_H

