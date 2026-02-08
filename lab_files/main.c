// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL // Power-up default value for COSC bits (EXTOSC with 4x PLL)
// CONFIG3L
#pragma config WDTE = OFF       // WDT operating mode (WDT enabled regardless of sleep)

#include <xc.h>
#include <stdio.h>
#include "LCD.h"
#include "ADC.h" // Required for Exercise 3

#define _XTAL_FREQ 64000000 

// ==========================================
//           EXERCISE 1 & 2 (COMMENTED OUT)
// ==========================================
/*
void main(void) {
    LCD_Init();  
    
    // ===== EXERCISE 2.1 =====
    // Using LCD_sendstring() instead of individual LCD_sendbyte() calls
    LCD_setline(1);
    LCD_sendstring("Hello World!");
    
    // ===== EXERCISE 2.2 =====
    // Write message to line 2 as well
    LCD_setline(2);
    LCD_sendstring("minions unite!");
    
    __delay_ms(2000);  // Display for 2 seconds
    
    // ===== EXERCISE 2.3 =====
    // Long scrolling message (40 char memory, 16 visible)
    LCD_clear();
    LCD_setline(1);
    LCD_sendstring("This message will be longer than 16 chars and it will scroll like a ball!");

    LCD_setline(2);
    LCD_sendstring("More scrolling for exercise 2...");
    
    // Continuous scroll loop
    while (1) {
        __delay_ms(300);  // Scroll speed (lower = faster)
        LCD_scroll();
    }
}
*/

/*
// ==========================================
//           EXERCISE 4 (COMMENTED OUT)
// ==========================================
// Custom character definitions (5 pixels wide x 8 pixels tall)
// Each byte is one row, only lower 5 bits used

// Walking frame 1 - right leg forward
const unsigned char walk_frame1[8] = {
    0b00100,  //   * (head)
    0b01110,  //  *** (arms)
    0b00100,  //   * (body)
    0b00100,  //   * (body)
    0b01010,  //  * * (legs)
    0b01000,  //  * (left foot back)
    0b00010,  //    * (right foot forward)
    0b00000   //       (empty)
};

// Walking frame 2 - left leg forward
const unsigned char walk_frame2[8] = {
    0b00100,  //   * (head)
    0b01110,  //  *** (arms)
    0b00100,  //   * (body)
    0b00100,  //   * (body)
    0b01010,  //  * * (legs)
    0b00010,  //    * (right foot back)
    0b01000,  //  * (left foot forward)
    0b00000   //       (empty)
};

void LCD_create_char(unsigned char char_num, const unsigned char *pattern)
{
    // Set CGRAM address: 0x40 + (char_num * 8)
    LCD_sendbyte(0x40 + (char_num * 8), 0);
    
    // Write 8 bytes of pattern data
    for (unsigned char i = 0; i < 8; i++) {
        LCD_sendbyte(pattern[i], 1);
    }
    
    // Return to DDRAM mode
    LCD_sendbyte(0x80, 0);
}

void main(void) {
    LCD_Init();
    
    // Create custom characters in CGRAM slots 0 and 1
    LCD_create_char(0, walk_frame1);
    LCD_create_char(1, walk_frame2);
    
    // Display title on line 1
    LCD_setline(1);
    LCD_sendstring("walking man");
    
    unsigned char position = 0;   // Current position (0-15)
    unsigned char frame = 0;      // Current animation frame (0 or 1)
    
    while (1) {
        // Clear line 2
        LCD_setline(2);
        LCD_sendstring("                ");  // 16 spaces to clear
        
        // Set cursor to current position on line 2
        LCD_sendbyte(0xC0 + position, 0);  // 0xC0 is line 2 start address
        
        // Display current walking frame (character 0 or 1)
        LCD_sendbyte(frame, 1);
        
        // Toggle animation frame
        frame = !frame;
        
        // Move position
        position++;
        if (position > 15) {
            position = 0;  // Wrap around to start
        }
        
        // Animation speed
        __delay_ms(200);
    }
}
*/


// ==========================================
//           EXERCISE 3 (ACTIVE)
// ==========================================
void main(void) {
    // 1. Initialise the modules
    LCD_Init(); // Initialise LCD screen
    ADC_init(); // Initialise ADC module
    
    // Variables to store results
    unsigned int adc_result;
    char buf[20]; // Buffer string for LCD display

    LCD_clear(); // Clear any garbage from screen

    while(1) {
        // 2. Monitor voltage using ADC
        adc_result = ADC_getval(); // Get the 8-bit result (0-255)
        
        // --- Requirement 1: Output 8 MSB as decimal integer ---
        LCD_setline(1); // Move to top line
        sprintf(buf, "Raw Val: %u    ", adc_result); // Pad with spaces to clear old digits
        LCD_sendstring(buf); 
        
        // --- Requirement 2: Convert to Volts (max 3.3V) ---
        LCD_setline(2); // Move to second line
        
        // Use the function in LCD.c which handles the integer math for 3.3V scaling
        ADC2String(buf, adc_result);
        LCD_sendstring(buf); 
        
        // Small delay
        __delay_ms(200);
    }
}
