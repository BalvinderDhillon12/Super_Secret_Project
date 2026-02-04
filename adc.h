/*******************************************************************************
 * File: adc.h
 * Project: Solar-Synchronized Light Controller
 * Target: PIC18F67K40
 *
 * Description: ADC driver interface.
 *              Initializes the ADC peripheral and provides LDR (Light Dependent
 *              Resistor) reading via burst-average for noise reduction.
 ******************************************************************************/

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialize the ADC peripheral for LDR reading.
 *
 * Configures ADCC in burst-average mode with 32 samples for noise reduction.
 * Call once at startup before ADC_ReadLDR().
 */
void ADC_Init(void);

/**
 * @brief Read the LDR (Light Dependent Resistor) value.
 *
 * Uses the ADC channel and burst-average settings from config.h.
 * Result is in range 0 (dark) to 1023 (bright).
 *
 * @return uint16_t ADC value (0 = dark, 1023 = bright)
 */
uint16_t ADC_ReadLDR(void);

#endif /* ADC_H */
