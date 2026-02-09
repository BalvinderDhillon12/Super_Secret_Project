/*******************************************************************************
 * File:   Buttons.c
 * Description: Button driver for RF2 push button (1 when pressed).
 ******************************************************************************/

#include <xc.h>
#include "Buttons.h"
#include "Config.h"

void Buttons_Init(void) {
    TRISFbits.TRISF2 = 1;   // RF2 as input
    ANSELFbits.ANSELF2 = 0; // Digital mode
}

uint8_t Button_RF2_Read(void) {
    return (uint8_t)(PORTFbits.RF2 ? 1u : 0u);
}
