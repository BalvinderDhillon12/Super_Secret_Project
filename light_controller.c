#include "light_controller.h"
#include "leds.h"

void LightController_Update(uint16_t ldr_value, DateTime_t *time) {
    bool is_night = (ldr_value < LDR_THRESHOLD_DUSK);
    bool in_shutoff_period = (time->hours >= 1 && time->hours < 5);
    
    if (is_night && !in_shutoff_period) {
        LEDs_SetOutsideLight(true);
    } else {
        LEDs_SetOutsideLight(false);
    }
}

