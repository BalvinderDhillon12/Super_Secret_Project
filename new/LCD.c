#define _XTAL_FREQ 64000000 


#include <xc.h>
#include "LCD.h"
#include "Config.h"


// PIN DEFINITIONS - Where the LCD is connected on the PIC


// Control pins
#define LCD_RS  LATCbits.LATC6    // Register Select: 0=command, 1=data
#define LCD_E   LATCbits.LATC2    // Enable: pulse this to send data

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
    // Force digital mode on LCD pins (PIC18 may default some to analog)
    ANSELBbits.ANSELB2 = 0;  // RB2 (D5) digital
    ANSELBbits.ANSELB3 = 0;  // RB3 (D4) digital
    ANSELEbits.ANSELE1 = 0;  // RE1 (D7) digital
    ANSELEbits.ANSELE3 = 0;  // RE3 (D6) digital

    // Configure all LCD pins as outputs
    TRISCbits.TRISC2 = 0;  // E
    TRISCbits.TRISC6 = 0;  // RS
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

static void LCD_PrintNumber2Digit(uint8_t n) {
    LCD_SendData('0' + (n / 10));
    LCD_SendData('0' + (n % 10));
}

static void LCD_PrintNumber4Digit(uint16_t n) {
    LCD_SendData('0' + (n / 1000));
    LCD_SendData('0' + ((n / 100) % 10));
    LCD_SendData('0' + ((n / 10) % 10));
    LCD_SendData('0' + (n % 10));
}

void LCD_UpdateDisplay(uint8_t hours, uint8_t minutes,
                      uint8_t day, uint8_t month, uint16_t year,
                      bool is_dst) {
    uint8_t display_hours = hours;
    bool is_pm = false;

    if (hours == 0) {
        display_hours = 12;
    } else if (hours == 12) {
        is_pm = true;
    } else if (hours > 12) {
        display_hours = hours - 12;
        is_pm = true;
    }

    /* Row 0: Time + DST indicator */
    LCD_SetCursor(0, 0);
    if (display_hours < 10) {
        LCD_SendData(' ');
        LCD_SendData('0' + display_hours);
    } else {
        LCD_PrintNumber2Digit(display_hours);
    }
    LCD_SendData(':');
    LCD_PrintNumber2Digit(minutes);
    if (is_pm) {
        LCD_PrintAt(0, 5, "PM ");
    } else {
        LCD_PrintAt(0, 5, "AM ");
    }
    if (is_dst) {
        LCD_PrintAt(0, 8, "BST ");
    } else {
        LCD_PrintAt(0, 8, "GMT ");
    }

    /* Row 1: Date DD/MM/YYYY */
    LCD_SetCursor(1, 0);
    LCD_PrintNumber2Digit(day);
    LCD_SendData('/');
    LCD_PrintNumber2Digit(month);
    LCD_SendData('/');
    LCD_PrintNumber4Digit(year);
    LCD_PrintAt(1, 10, "      ");
}
