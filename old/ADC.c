/* 
 * File:   ADC.h
 * Purpose: This file is to convert analog voltage to a digital number through an ADC (analogue to digital converter) driver which reads the light sensor 
 */

#include <xc.h>
#include "ADC.h"
#include "Config.h"

void ADC_Init(void) {
    TRISAbits.TRISA3 = 1;   // LDR input on RA3
    ANSELAbits.ANSELA3 = 1; // RA3 analog 
    ADPCH = ADC_LDR_CHANNEL; // RA3 

    ADCON0bits.ADCS = 1;    // FRC (Fast RC) clock
    ADACQ = 10;             // 10 TAD acquisition time 

    ADCON0bits.ADFM = 1;   

    // Takes several samples to average it 
    ADCON2bits.ADMD = 0b010;  

    // 32 samples: result is 32x raw value, so we shift right by 5 to get 0-1023. 
    ADRPT = 32;

    // Positive reference VREF+ = VDD, negative = VSS. 
    ADREF = 0x00;

    // Enable the ADC module. 
    ADCON0bits.ADON = 1;
}

uint16_t ADC_ReadLDR(void) {
    uint16_t adc_result;

    ADPCH = ADC_LDR_CHANNEL;

    // Start one conversion
    ADCON0bits.ADGO = 1;

    // Block until conversion is complete
    while (ADCON0bits.ADGO) {

    }

    // Read 16-bit accumulated result
    adc_result = ADFLTRH;
    adc_result = (adc_result << 8) | ADFLTRL;

    // Divide by 32  to get average in 0-1023 range. 
    adc_result = adc_result >> 5;

    return adc_result;
}
