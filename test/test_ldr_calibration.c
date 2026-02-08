/*******************************************************************************
 * File:   test_ldr_calibration.c
 * Purpose: Standalone test - LDR calibration via RF2 (two presses: dark then light),
 *          then LED 9 on when dark, off when light. Only LED 9 is used.
 ******************************************************************************/

#include <xc.h>
#include <stdbool.h>
#include "../new/Config.h"
#include "../new/ADC.h"
#include "../new/LEDS.h"
#include "../new/Buttons.h"

#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC_4PLL
#pragma config WDTE = OFF

#define _XTAL_FREQ 64000000

#define BLINK_MS 300
#define NUM_SAMPLES 32
#define DAY_DELTA   150   /* change needed to detect day vs night (from partner) */

static uint16_t ReadLDR_Averaged(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        sum += ADC_ReadLDR();
        __delay_ms(2);
    }
    return (uint16_t)(sum / NUM_SAMPLES);
}

int main(void) {
    uint16_t dark_value;
    uint16_t light_value;
    uint16_t delta;
    uint8_t led_on;

    LEDs_Init();
    ADC_Init();
    Buttons_Init();

    /* Phase 1: Blink LED 9 - user covers LDR, then presses RF2 to save dark value */
    while (1) {
        LEDs_SetMainLight(1);
        __delay_ms(BLINK_MS);
        LEDs_SetMainLight(0);
        __delay_ms(BLINK_MS);
        if (Button_RF2_Read() == 1) {
            break;
        }
    }
    __delay_ms(50);
    dark_value = ReadLDR_Averaged();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    /* Phase 2: LED 9 solid ON - user exposes LDR, then presses RF2 to save light value */
    LEDs_SetMainLight(1);
    while (Button_RF2_Read() == 0) {
    }
    __delay_ms(50);
    light_value = ReadLDR_Averaged();
    while (Button_RF2_Read() == 1) {
    }
    __delay_ms(50);

    if (dark_value > light_value)
        delta = (dark_value - light_value) / 2u;
    else
        delta = (light_value - dark_value) / 2u;
    if (delta < 10u) delta = 10u;

    led_on = 1;
    LEDs_SetMainLight(1);
    __delay_ms(500);  /* let reading settle before run loop (like partner's startup) */

    /* Running: partner's relative-change comparison; dark_value = baseline */
    while (1) {
        uint16_t reading = ReadLDR_Averaged();

        if (reading > dark_value + delta || reading + delta < dark_value) {
            led_on = 0;   /* reading far from dark baseline → light → LED OFF */
        } else {
            led_on = 1;   /* reading near dark baseline → dark → LED ON */
        }

        LEDs_SetMainLight(led_on);
        __delay_ms(500);
    }
}
