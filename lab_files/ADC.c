#include <xc.h>
#include "ADC.h"

/************************************
 * ADC_init
 * Function used to initialise ADC module and set it up
 * to sample on pin RA3
 ************************************/
void ADC_init(void)
{
    TRISAbits.TRISA3 = 1;       // Select pin RA3 as input
    ANSELAbits.ANSELA3 = 1;     // Ensure analogue circuitry is active
    
    // Set up the ADC module - section 32 of datasheet
    ADREFbits.ADNREF = 0;       // Use Vss (0V) as negative reference
    ADREFbits.ADPREF = 0b00;    // Use Vdd (3.3V) as positive reference
    
    ADPCH = 0x03;               // Select channel RA3/ANA3 for ADC
    
    ADCON0bits.ADFM = 0;        // Left-justified result (8 MSB in ADRESH)
    ADCON0bits.ADCS = 1;        // Use internal Fast RC (FRC) oscillator
    
    // Set acquisition time (this was missing - critical for stable readings!)
    ADACQ = 0xFF;               // Maximum acquisition time
    
    ADCON0bits.ADON = 1;        // Enable ADC
}

/************************************
 * ADC_getval
 * Function to read the ADC value
 * Returns 8-bit result (0-255)
 ************************************/
unsigned int ADC_getval(void)
{
    unsigned int tmpval;
    
    ADCON0bits.GO = 1;          // Start ADC conversion
    while (ADCON0bits.GO);      // Wait until conversion done
    
    tmpval = ADRESH;            // Get 8 most significant bits
    return tmpval;
}
