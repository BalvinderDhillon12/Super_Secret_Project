#include <xc.h>
#include <stdio.h>
#include "LCD.h"

/************************************
 * Function to toggle LCD enable bit on then off
 * when this function is called the LCD screen reads the data lines
************************************/
void LCD_E_TOG(void)
{
    LCD_E = 1;          // Turn the LCD enable bit on
    __delay_us(2);      // Wait a short delay
    LCD_E = 0;          // Turn the LCD enable bit off again
}

/************************************
 * Function to set the 4-bit data line levels for the LCD
************************************/
void LCD_sendnibble(unsigned char number)
{
    // Set each data line based on bits of 'number'
    // number contains the nibble in the lower 4 bits
    LCD_DB4 = (number >> 0) & 1;  // Bit 0 -> DB4
    LCD_DB5 = (number >> 1) & 1;  // Bit 1 -> DB5
    LCD_DB6 = (number >> 2) & 1;  // Bit 2 -> DB6
    LCD_DB7 = (number >> 3) & 1;  // Bit 3 -> DB7

    LCD_E_TOG();        // Toggle the enable bit to instruct the LCD to read the data lines
    __delay_us(5);      // Delay 5uS
}

/************************************
 * Function to send full 8-bit commands/data over the 4-bit interface
 * high nibble (4 most significant bits) are sent first, then low nibble sent
************************************/
void LCD_sendbyte(unsigned char Byte, char type)
{
    // Set RS pin: 0 for Command, 1 for Data/Character
    LCD_RS = type;
    
    // Send high nibble (upper 4 bits) first
    LCD_sendnibble((Byte >> 4) & 0x0F);
    
    // Send low nibble (lower 4 bits)
    LCD_sendnibble(Byte & 0x0F);
    
    __delay_us(50);     // Delay 50uS (minimum for command to execute)
}

/************************************
 * Function to initialise the LCD after power on
************************************/
void LCD_Init(void)
{
    // Define LCD Pins as Outputs (clear TRIS bits)
    TRISCbits.TRISC6 = 0;   // RS
    TRISCbits.TRISC2 = 0;   // E
    TRISBbits.TRISB3 = 0;   // DB4
    TRISBbits.TRISB2 = 0;   // DB5
    TRISEbits.TRISE3 = 0;   // DB6
    TRISEbits.TRISE1 = 0;   // DB7
    
    // Set all pins low initially
    LCD_RS = 0;
    LCD_E = 0;
    LCD_DB4 = 0;
    LCD_DB5 = 0;
    LCD_DB6 = 0;
    LCD_DB7 = 0;
    
    // Wait more than 40ms after power on
    __delay_ms(50);
    
    // Initialisation sequence for 4-bit mode (from datasheet flowchart)
    // LCD starts in 8-bit mode, so we send nibbles directly
    
    // First Function Set: send 0x3 (DB5=1, DB4=1)
    LCD_RS = 0;
    LCD_sendnibble(0x3);
    __delay_us(39);         // Wait more than 39us
    
    // Second Function Set: send 0x3 again
    LCD_sendnibble(0x3);
    __delay_us(39);         // Wait more than 39us
    
    // Third Function Set: send 0x3 again
    LCD_sendnibble(0x3);
    __delay_us(39);         // Wait more than 37us
    
    // Set to 4-bit mode: send 0x2
    LCD_sendnibble(0x2);
    __delay_us(39);         // Wait for command to execute
    
    // Now in 4-bit mode - use LCD_sendbyte from here on
    
    // Function Set: 0x28 = 4-bit mode, 2 lines, 5x8 font
    LCD_sendbyte(0x28, 0);
    __delay_us(39);
    
    // Display OFF: 0x08
    LCD_sendbyte(0x08, 0);
    __delay_us(39);
    
    // Display Clear: 0x01
    LCD_sendbyte(0x01, 0);
    __delay_ms(2);          // Clear command needs >1.53ms
    
    // Entry Mode Set: 0x06 = increment cursor, no shift
    LCD_sendbyte(0x06, 0);
    __delay_us(39);
    
    // Display ON: 0x0C = display on, cursor off, blink off
    LCD_sendbyte(0x0C, 0);
    __delay_us(39);
}

/************************************
 * Function to set the cursor to beginning of line 1 or 2
************************************/
void LCD_setline(char line)
{
    if (line == 1) {
        LCD_sendbyte(0x80, 0);  // Set DDRAM address to 0x00 (line 1)
    } else if (line == 2) {
        LCD_sendbyte(0xC0, 0);  // Set DDRAM address to 0x40 (line 2)
    }
}

/************************************
 * Function to send string to LCD screen
************************************/
void LCD_sendstring(char *string)
{
    while (*string != 0) {              // While not at null terminator
        LCD_sendbyte(*string++, 1);     // Send character and increment pointer
    }
}

/************************************
 * Function to scroll the display left
************************************/
void LCD_scroll(void)
{
    // Shift display left: 0x18
    LCD_sendbyte(0x18, 0);
}

/************************************
 * Function to clear the LCD display
************************************/
void LCD_clear(void)
{
    LCD_sendbyte(0x01, 0);  // Clear display command
    __delay_ms(2);          // Clear needs >1.53ms to execute
}

/************************************
 * Function takes an ADC value and works out the voltage to 2 dp
 * the result is stored in buf as ascii text ready for display on LCD
 * Assumes 8-bit ADC (0-255) with 3.3V reference
************************************/
void ADC2String(char *buf, unsigned int ADC_val)
{
    unsigned int int_part;
    unsigned int frac_part;
    
    // 1. Calculate Integer Part
    // We can stick to standard int here as 255 * 33 = 8415 (fits in 16-bit)
    int_part = (ADC_val * 33) / 2550;
    
    // 2. Calculate Fractional Part
    unsigned long calc_temp = ((unsigned long)ADC_val * 3300) / 2550;
    
    frac_part = calc_temp - (int_part * 100);
    
    // 3. Format as string
    // Added spaces at the end to overwrite any old characters on the LCD line
    sprintf(buf, "%u.%02u V    ", int_part, frac_part);
}