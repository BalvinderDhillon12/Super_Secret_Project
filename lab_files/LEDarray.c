#include <xc.h>
#include "LEDarray.h"

/************************************
/ LEDarray_init
/ Initialise pins connected to the LED array
************************************/
void LEDarray_init(void)
{
    // Set up TRIS registers (0 = Output, 1 = Input)
    TRISGbits.TRISG0 = 0;
    TRISGbits.TRISG1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISFbits.TRISF6 = 0;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA5 = 0;
    TRISFbits.TRISF0 = 0;
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    TRISCbits.TRISC6 = 0;
    
    // Set initial output LAT values to 0 (off)
    LATGbits.LATG0 = 0;
    LATGbits.LATG1 = 0;
    LATAbits.LATA2 = 0;
    LATFbits.LATF6 = 0;
    LATAbits.LATA4 = 0;
    LATAbits.LATA5 = 0;
    LATFbits.LATF0 = 0;
    LATBbits.LATB0 = 0;
    LATBbits.LATB1 = 0;
    LATCbits.LATC6 = 0;
}

/************************************
/ Button_init
/ Initialise the button on RF2
************************************/
void Button_init(void)
{
    TRISFbits.TRISF2 = 1;   // Set RF2 as input
    ANSELFbits.ANSELF2 = 0; // Disable analogue circuitry on RF2 (Digital mode)
}

/************************************
/ LEDarray_disp_bin
/ Display a number on the LED array in binary
************************************/
void LEDarray_disp_bin(unsigned int number)
{
    // Check individual bits of the number and set the corresponding LED
    
    if (number & 1)   { LATGbits.LATG0 = 1; } else { LATGbits.LATG0 = 0; }
    if (number & 2)   { LATGbits.LATG1 = 1; } else { LATGbits.LATG1 = 0; }
    if (number & 4)   { LATAbits.LATA2 = 1; } else { LATAbits.LATA2 = 0; }
    if (number & 8)   { LATFbits.LATF6 = 1; } else { LATFbits.LATF6 = 0; }
    if (number & 16)  { LATAbits.LATA4 = 1; } else { LATAbits.LATA4 = 0; }
    if (number & 32)  { LATAbits.LATA5 = 1; } else { LATAbits.LATA5 = 0; }
    if (number & 64)  { LATFbits.LATF0 = 1; } else { LATFbits.LATF0 = 0; }
    if (number & 128) { LATBbits.LATB0 = 1; } else { LATBbits.LATB0 = 0; }
    if (number & 256) { LATBbits.LATB1 = 1; } else { LATBbits.LATB1 = 0; }
    if (number & 512) { LATCbits.LATC6 = 1; } else { LATCbits.LATC6 = 0; }
}

/************************************
/ Function LEDarray_disp_dec
/ Used to display a number on the LEDs
/ where each LED is a value of 10
************************************/
void LEDarray_disp_dec(unsigned int number)
{
    unsigned int disp_val = 0; // Start with all LEDs off
    
    // Check thresholds and build the bitmask
    if (number >= 10) { disp_val |= 1; }      // LED 1
    if (number >= 20) { disp_val |= 2; }      // LED 2
    if (number >= 30) { disp_val |= 4; }      // LED 3
    if (number >= 40) { disp_val |= 8; }      // LED 4
    if (number >= 50) { disp_val |= 16; }     // LED 5
    if (number >= 60) { disp_val |= 32; }     // LED 6
    if (number >= 70) { disp_val |= 64; }     // LED 7
    if (number >= 80) { disp_val |= 128; }    // LED 8
    if (number >= 90) { disp_val |= 256; }    // LED 9
    if (number >= 100){ disp_val |= 512; }    // LED 10

    // Reuse the binary display function to output the pattern
    LEDarray_disp_bin(disp_val);
}

/************************************
/ LEDarray_disp_PPM
/ Function used to display a level on the LED array with peak hold
/ cur_val is the current level from the most recent sample, and max is the peak value for the last second
/ these input values need to calculated else where in your code
************************************/
void LEDarray_disp_PPM(unsigned int cur_val, unsigned int max)
{
    unsigned int disp_val = 0;
    unsigned int peak_val = 0;
    
    // Set up the bar graph for the current value (cur_val)
    if (cur_val >= 10) { disp_val |= 1; }
    if (cur_val >= 20) { disp_val |= 2; }
    if (cur_val >= 30) { disp_val |= 4; }
    if (cur_val >= 40) { disp_val |= 8; }
    if (cur_val >= 50) { disp_val |= 16; }
    if (cur_val >= 60) { disp_val |= 32; }
    if (cur_val >= 70) { disp_val |= 64; }
    if (cur_val >= 80) { disp_val |= 128; }
    if (cur_val >= 90) { disp_val |= 256; }
    if (cur_val >= 100){ disp_val |= 512; }

    // Set up the single dot for the peak value (max)
    // We use '=' instead of '|=' here because we only want the single highest dot, 
    // not a full bar for this part.
    if (max >= 10) { peak_val = 1; }
    if (max >= 20) { peak_val = 2; }
    if (max >= 30) { peak_val = 4; }
    if (max >= 40) { peak_val = 8; }
    if (max >= 50) { peak_val = 16; }
    if (max >= 60) { peak_val = 32; }
    if (max >= 70) { peak_val = 64; }
    if (max >= 80) { peak_val = 128; }
    if (max >= 90) { peak_val = 256; }
    if (max >= 100){ peak_val = 512; }

    // Combine them
    // Use OR to mix the bar graph (disp_val) and the peak dot (peak_val)
    LEDarray_disp_bin(disp_val | peak_val);
}