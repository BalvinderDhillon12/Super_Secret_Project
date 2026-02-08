/*******************************************************************************
 * File:   ADC.c
 * Purpose: ADC driver for LDR (light sensor) on RA3. Single conversion mode
 *          (burst-average disabled for board compatibility), result 0-1023.
 *          Includes timeout to avoid hang if ADC fails.
 ******************************************************************************/

#include <xc.h>
#include "ADC.h"
#include "Config.h"

void ADC_Init(void) {
    TRISAbits.TRISA3 = 1;   // LDR input on RA3
    ANSELAbits.ANSELA3 = 1; // RA3 analog
    ADPCH = ADC_LDR_CHANNEL; // RA3

    ADCON0bits.ADCS = 1;    // FRC (Fast RC) clock
    ADACQ = 0xFF;           // Max acquisition time (critical for stable LDR readings)

    ADCON0bits.ADFM = 1;    // Right-justified result (10-bit in ADRESH/ADRESL)

    ADREF = 0x00;            // VREF+ = VDD, VREF- = VSS

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

    /* Single conversion: 10-bit result in ADRESH (high 2 bits) and ADRESL (low 8 bits). */
    adc_result = (uint16_t)ADRESH << 8;
    adc_result |= ADRESL;

    return adc_result;
}
