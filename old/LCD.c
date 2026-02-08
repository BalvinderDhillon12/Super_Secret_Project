#define _XTAL_FREQ 64000000 


#include <xc.h>
#include "LCD.h"
#include "Config.h"


// PIN DEFINITIONS - Where the LCD is connected on the PIC


// Control pins
#define LCD_RS  LATCbits.LATC2    // Register Select: 0=command, 1=data
#define LCD_E   LATCbits.LATC6    // Enable: pulse this to send data

// Data pins (4-bit mode uses only D4-D7)
#define LCD_D4  LATBbits.LATB3
#define LCD_D5  LATBbits.LATB2
#define LCD_D6  LATEbits.LATE3
#define LCD_D7  LATEbits.LATE1

static void LCD_Delay_ms(uint16_t milliseconds) {
    for (uint16_t i = 0; i < milliseconds; i++) {
        __delay_ms(1);
    }
}

static void LCD_Send4Bits(uint8_t data) {
    // Put the 4 bits on the data pins
    LCD_D4 = (data >> 0) & 1;  // Bit 0
    LCD_D5 = (data >> 1) & 1;  // Bit 1
    LCD_D6 = (data >> 2) & 1;  // Bit 2
    LCD_D7 = (data >> 3) & 1;  // Bit 3
    
    // Pulse the Enable pin to tell LCD to read the data
    LCD_E = 1;
    __delay_us(1);  // Short delay
    LCD_E = 0;
    __delay_us(100);  // LCD needs time to process
}

static void LCD_SendCommand(uint8_t command) {
    LCD_RS = 0;  // RS=0 means we're sending a command (not data)
    
    // Send high 4 bits first
    LCD_Send4Bits(command >> 4);
    
    // Then send low 4 bits
    LCD_Send4Bits(command & 0x0F);
    
    LCD_Delay_ms(2);  // Give LCD time to execute the command
}

static void LCD_SendData(uint8_t data) {
    LCD_RS = 1;  // RS=1 means we're sending data (a character)
    
    // Send high 4 bits first
    LCD_Send4Bits(data >> 4);
    
    // Then send low 4 bits
    LCD_Send4Bits(data & 0x0F);
    
    __delay_us(100);  // Give LCD time to display the character
}

static void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address;
    
    if (row == 0) {
        address = 0x80 + col;  // Top row starts at address 0x80
    } else {
        address = 0xC0 + col;  // Bottom row starts at address 0xC0
    }
    
    LCD_SendCommand(address);
}

static void LCD_PrintAt(uint8_t row, uint8_t col, const char* text) {
    LCD_SetCursor(row, col);
    
    // Send each character one by one
    while (*text) {
        LCD_SendData(*text);
        text++;
    }
}

static void LCD_PrintNumberAt(uint8_t row, uint8_t col, uint8_t number, uint8_t digits) {
    char buffer[10];
    
    // Convert number to string with leading zeros if needed
    if (digits == 2) {
        buffer[0] = '0' + (number / 10);  // Tens digit
        buffer[1] = '0' + (number % 10);  // Ones digit
        buffer[2] = '\0';
    } else {
        buffer[0] = '0' + number;
        buffer[1] = '\0';
    }
    
    LCD_PrintAt(row, col, buffer);
}



void LCD_Init(void) {
    // Configure all LCD pins as outputs
    TRISCbits.TRISC2 = 0;  // RS
    TRISCbits.TRISC6 = 0;  // E
    TRISBbits.TRISB3 = 0;  // D4
    TRISBbits.TRISB2 = 0;  // D5
    TRISEbits.TRISE3 = 0;  // D6
    TRISEbits.TRISE1 = 0;  // D7
    
    // Start with everything low
    LCD_RS = 0;
    LCD_E = 0;
    LCD_D4 = 0;
    LCD_D5 = 0;
    LCD_D6 = 0;
    LCD_D7 = 0;
    
    // Wait for LCD to power up (LCD needs time after power on)
    LCD_Delay_ms(50);
    
    // Initialize LCD in 4-bit mode (special sequence required)
    LCD_Send4Bits(0x03);  
    LCD_Delay_ms(5);
    LCD_Send4Bits(0x03);  
    LCD_Delay_ms(1);
    LCD_Send4Bits(0x03);  
    LCD_Delay_ms(1);
    LCD_Send4Bits(0x02);  
    
    // Configure LCD settings
    LCD_SendCommand(0x28);  // 4-bit mode, 2 lines, 5x8 font
    LCD_SendCommand(0x0C);  // Display ON, cursor OFF
    LCD_SendCommand(0x06);  // Auto-increment cursor
    LCD_SendCommand(0x01);  // Clear screen
    LCD_Delay_ms(2);
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);  // Clear display command
    LCD_Delay_ms(2);
}

void LCD_UpdateDisplay(uint8_t hours, uint8_t minutes, bool is_summer, uint16_t light_adc) {
    // Convert 24-hour time to 12-hour with AM/PM
    uint8_t display_hours = hours;
    bool is_pm = false;
    
    if (hours == 0) {
        display_hours = 12;  // Midnight = 12 AM
    } else if (hours == 12) {
        is_pm = true;  // Noon = 12 PM
    } else if (hours > 12) {
        display_hours = hours - 12;  // 13:00 = 1 PM, etc.
        is_pm = true;
    }
    
    // Convert ADC reading (0-1023) to voltage (0.0-5.0V)
    // Formula: voltage = (ADC / 1023) * 5.0
    // To avoid floating point, we use: voltage_x10 = (ADC * 50) / 1023
    uint16_t voltage_x10 = (light_adc * 50) / 1023;  // Result in tenths of volts
    uint8_t volts = voltage_x10 / 10;
    uint8_t decimal = voltage_x10 % 10;
    
    // top line
    LCD_SetCursor(0, 0);
    
    // Print hours (with space for single digit)
    if (display_hours < 10) {
        LCD_SendData(' ');
        LCD_PrintNumberAt(0, 1, display_hours, 1);
    } else {
        LCD_PrintNumberAt(0, 0, display_hours, 2);
    }
    
    // Print colon
    LCD_SendData(':');
    
    // Print minutes (always 2 digits)
    LCD_PrintNumberAt(0, 3, minutes, 2);
    
    // Print AM or PM
    if (is_pm) {
        LCD_PrintAt(0, 5, "PM");
    } else {
        LCD_PrintAt(0, 5, "AM");
    }
    
    // Print DST status (WINTER or SUMMER)
    if (is_summer) {
        LCD_PrintAt(0, 11, "SUMR");  
    } else {
        LCD_PrintAt(0, 11, "WINT");  
    }
    
    // light volt
    // Format: "Light: 2.5V    "
    LCD_PrintAt(1, 0, "Light:");
    LCD_PrintNumberAt(1, 7, volts, 1);
    LCD_SendData('.');
    LCD_PrintNumberAt(1, 9, decimal, 1);
    LCD_SendData('V');
    
    // Clear rest of line
    LCD_PrintAt(1, 11, "     ");
}
