/*******************************************************************************
 * File:   ADC.c
 * Purpose: ADC driver for LDR (light sensor) on RA3. Burst-average mode,
 *          32 samples, result 0-1023. Includes timeout to avoid hang if ADC fails.
 ******************************************************************************/

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

    /* Burst-average mode (use MD for XC8 header compatibility). */
    ADCON2bits.MD = 0b010;  

    // 32 samples: result is 32x raw value, so we shift right by 5 to get 0-1023. 
    ADRPT = 32;

    // Positive reference VREF+ = VDD, negative = VSS. 
    ADREF = 0x00;

    // Enable the ADC module. 
    ADCON0bits.ADON = 1;
}

#define ADC_READ_TIMEOUT  65535U   /* Max loops; prevents hang if ADC never completes */

uint16_t ADC_ReadLDR(void) {
    uint16_t adc_result;
    uint16_t timeout = ADC_READ_TIMEOUT;

    ADPCH = ADC_LDR_CHANNEL;

    ADCON0bits.ADGO = 1;

    while (ADCON0bits.ADGO && timeout) {
        timeout--;
    }

    if (timeout == 0) {
        /* ADC did not complete; return mid-scale to avoid hang and allow system to run. */
        return 512;
    }

    adc_result = ADFLTRH;
    adc_result = (adc_result << 8) | ADFLTRL;
    adc_result = adc_result >> 5;

    return adc_result;
}
