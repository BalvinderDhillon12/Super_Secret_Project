#include "adc.h"

void ADC_Init(void) {
    // ADC Setup
    ANSEL_LDR = 1;  // RA0 as analog
    TRIS_LDR = 1;   // RA0 as input
    
    ADCON0bits.ON = 1;      // Turn on ADC
    ADCON0bits.CONT = 0;    // Single conversion
    ADCON0bits.CS = 1;      // FRC clock
    ADCON0bits.FRM = 1;     // Right justified
    
    ADPCH = LDR_CHANNEL;    // Select AN0
    
    // Default reference is VDD/VSS
    ADREFbits.NREF = 0;
    ADREFbits.PREF = 0;
}

uint16_t ADC_Read(uint8_t channel) {
    ADPCH = channel;
    __delay_us(10);         // Acquisition time
    ADCON0bits.GO = 1;      // Start conversion
    while (ADCON0bits.GO);  // Wait for completion
    return ((uint16_t)ADRESH << 8) | ADRESL;
}

uint16_t ADC_GetLDRValue(void) {
    return ADC_Read(LDR_CHANNEL);
}

