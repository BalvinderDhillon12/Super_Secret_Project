/*******************************************************************************
 * File: adc.c
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: ADC driver implementation.
 *              Owns all ADC-related code: initialization and LDR reading.
 *              No other peripheral code belongs here.
 ******************************************************************************/

#include <xc.h>
#include "adc.h"

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Initialize the ADC for LDR reading.
 *
 * We use ADCC (ADC with Computation) in Burst-Average mode so the hardware
 * accumulates 32 samples and we avoid software averaging and division.
 * LDR is on RA3 (Channel 3). Clock: internal FRC; acquisition time per spec.
 */
void ADC_Init(void) {
    TRISAbits.TRISA3 = 1;   /* LDR input on RA3 */
    ANSELAbits.ANSELA3 = 1; /* RA3 analog */
    ADPCH = ADC_LDR_CHANNEL; /* Channel 3 / ANA3 */

    ADCON0bits.ADCS = 1;    /* FRC (Fast RC) clock */
    ADACQ = 10;             /* 10 TAD acquisition time */

    ADCON0bits.ADFM = 1;    /* Result format: right-justified */

    /* Burst-Average mode: hardware takes multiple samples and accumulates. */
    ADCON2bits.ADMD = 0b010;  /* <-- Fixed: ADMD not MD */

    /* 32 samples: result is 32x raw value, so we shift right by 5 to get 0-1023. */
    ADRPT = 32;

    /* Positive reference VREF+ = VDD, negative = VSS. */
    ADREF = 0x00;

    /* Enable the ADC module. */
    ADCON0bits.ADON = 1;
}

/**
 * @brief Read LDR via ADC and return averaged value in 0-1023 range.
 *
 * We select the LDR channel (RA3/ANA3), start conversion, wait for
 * completion, then read the 16-bit filter register. In burst-average mode
 * the hardware accumulates 32 samples; we divide by 32 with a right shift
 * to get the average in the standard 0-1023 range.
 *
 * @return uint16_t LDR value: 0 = dark, 1023 = bright
 */
uint16_t ADC_ReadLDR(void) {
    uint16_t adc_result;

    ADPCH = ADC_LDR_CHANNEL;

    /* Start one conversion (in burst-average this runs the full burst). */
    ADCON0bits.ADGO = 1;

    /* Block until conversion complete. In a tick-driven system this is acceptable. */
    while (ADCON0bits.ADGO) {
        /* Wait */
    }

    /* Read 16-bit accumulated result. High byte then low byte. */
    adc_result = ADFLTRH;
    adc_result = (adc_result << 8) | ADFLTRL;

    /* Divide by 32 (shift right 5) to get average in 0-1023 range. */
    adc_result = adc_result >> 5;

    return adc_result;
}
