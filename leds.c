#include "leds.h"

void LEDs_Init(void) {
    // Set all LED pins as output
    TRIS_H0 = 0;
    TRIS_H1 = 0;
    TRIS_H2 = 0;
    TRIS_H3 = 0;
    TRIS_H4 = 0;
    TRIS_OUTSIDE = 0;
    TRIS_STATUS1 = 0;
    TRIS_STATUS2 = 0;
    TRIS_STATUS3 = 0;
    
    // Turn all off initially
    LED_H0 = 0;
    LED_H1 = 0;
    LED_H2 = 0;
    LED_H3 = 0;
    LED_H4 = 0;
    LED_OUTSIDE = 0;
    LED_STATUS1 = 0;
    LED_STATUS2 = 0;
    LED_STATUS3 = 0;
}

void LEDs_UpdateHourDisplay(uint8_t hours) {
    LED_H0 = (hours >> 0) & 0x01;
    LED_H1 = (hours >> 1) & 0x01;
    LED_H2 = (hours >> 2) & 0x01;
    LED_H3 = (hours >> 3) & 0x01;
    LED_H4 = (hours >> 4) & 0x01;
}

void LEDs_SetOutsideLight(bool state) {
    LED_OUTSIDE = state ? 1 : 0;
}

void LEDs_SetStatus(uint8_t index, bool state) {
    switch (index) {
        case 1: LED_STATUS1 = state ? 1 : 0; break;
        case 2: LED_STATUS2 = state ? 1 : 0; break;
        case 3: LED_STATUS3 = state ? 1 : 0; break;
    }
}

