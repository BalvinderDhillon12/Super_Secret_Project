/*******************************************************************************
 * File: bsp.c
 * Project: Energy Saving Automatic Outside Light
 * Target: PIC18F67K40
 * 
 * Description: Board Support Package Implementation
 *              Hardware abstraction layer for PIC18F67K40
 ******************************************************************************/

#include <xc.h>
#include "bsp.h"
#include "config.h"
#include "rtc_soft.h"

/*******************************************************************************
 * CONFIGURATION BITS
 ******************************************************************************/
// CONFIG1L
#pragma config FEXTOSC = OFF    // External Oscillator mode selection: Oscillator not enabled
#pragma config RSTOSC = HFINTOSC_64MHZ  // Power-up default value for COSC: HFINTOSC (64MHz)

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock Out Enable: CLKOUT function is disabled
#pragma config CSWEN = ON       // Clock Switch Enable: Writing to NOSC and NDIV is allowed
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable: FSCM timer enabled

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // Master Clear Enable: MCLR pin is Master Clear with weak pull-up
#pragma config PWRTE = OFF      // Power-up Timer Enable: PWRT disabled
#pragma config LPBOREN = OFF    // Low-Power BOR enable: ULPBOR disabled
#pragma config BOREN = SBORDIS  // Brown-out reset enable: Brown-out Reset enabled

// CONFIG2H
#pragma config BORV = LO        // Brown-out Reset Voltage: Brown-out Reset Voltage (VBOR) set to 1.9V
#pragma config ZCD = OFF        // Zero-cross detect disable
#pragma config PPS1WAY = ON     // PPSLOCKED One-Way Set Enable: The PPSLOCK bit can be cleared and set only once
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable: Stack Overflow or Underflow will cause a reset

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31   // WDT Period Select: Divider ratio 1:65536
#pragma config WDTE = OFF       // WDT operating mode: WDT Disabled

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7    // WDT Window Select: window always open (100%)
#pragma config WDTCCS = SC      // WDT input clock selector: Software Control

// CONFIG4L
#pragma config WRT0 = OFF       // Write Protection Block 0: Block 0 not write-protected
#pragma config WRT1 = OFF       // Write Protection Block 1: Block 1 not write-protected
#pragma config WRT2 = OFF       // Write Protection Block 2: Block 2 not write-protected
#pragma config WRT3 = OFF       // Write Protection Block 3: Block 3 not write-protected

// CONFIG4H
#pragma config WRTC = OFF       // Configuration Register Write Protection: Configuration registers not write-protected
#pragma config WRTB = OFF       // Boot Block Write Protection: Boot Block not write-protected
#pragma config WRTD = OFF       // Data EEPROM Write Protection: Data EEPROM not write-protected
#pragma config SCANE = ON       // Scanner Enable: Scanner module is available for use

// CONFIG5L
#pragma config LVP = ON         // Low Voltage Programming Enable: Low Voltage programming enabled

// CONFIG5H
#pragma config CP = OFF         // UserNVM Program Memory Code Protection: UserNVM code protection disabled
#pragma config CPD = OFF        // DataNVM Memory Code Protection: DataNVM code protection disabled

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ******************************************************************************/
static void BSP_InitOscillator(void);
static void BSP_InitGPIO(void);
static void BSP_InitADC(void);
static void BSP_InitTimer0(void);

/*******************************************************************************
 * INTERRUPT SERVICE ROUTINE
 ******************************************************************************/
void __interrupt() ISR(void) {
    // Timer0 Interrupt - RTC Tick
    if (PIR0bits.TMR0IF) {
        PIR0bits.TMR0IF = 0;        // Clear interrupt flag
        
        // Reload Timer0 for next tick
        TMR0H = TMR0_RELOAD_HIGH;
        TMR0L = TMR0_RELOAD_LOW;
        
        // Call RTC tick handler
        RTC_TickISR();
    }
}

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

void BSP_Init(void) {
    BSP_InitOscillator();
    BSP_InitGPIO();
    BSP_InitADC();
    BSP_InitTimer0();
}

uint16_t BSP_GetLDR(void) {
    uint16_t adc_result;
    
    // Select LDR channel
    ADPCH = LDR_ADC_CHANNEL;
    
    // Start conversion
    ADCON0bits.ADGO = 1;
    
    // Wait for conversion complete
    while (ADCON0bits.ADGO);
    
    // Read 16-bit accumulated result (32 samples averaged)
    // Result is in ADFLTR register
    adc_result = ADFLTRH;
    adc_result = (adc_result << 8) | ADFLTRL;
    
    // Divide by 32 (shift right 5) to get average in 0-1023 range
    adc_result = adc_result >> 5;
    
    return adc_result;
}

void BSP_SetMainLight(bool state) {
    LIGHT_LAT = state ? 1 : 0;
}

void BSP_SetClockDisplay(uint8_t hour) {
    // Ensure hour is in valid range (0-23)
    if (hour >= HOURS_PER_DAY) {
        hour = 0;
    }
    
    // Clear the display bits and set new value
    // Only use lower 5 bits (0-31 range covers 0-23 hours)
    CLOCK_DISPLAY_LAT = (CLOCK_DISPLAY_LAT & ~CLOCK_DISPLAY_MASK) | 
                        (hour & CLOCK_DISPLAY_MASK);
}

void BSP_ToggleHeartbeat(void) {
    HEARTBEAT_LAT ^= 1;  // XOR to toggle
}

/*******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

static void BSP_InitOscillator(void) {
    // Configure for 16MHz operation
    // HFINTOSC is 64MHz by default, divide by 4 to get 16MHz
    OSCCON1bits.NDIV = 0b0010;  // Divide by 4: 64MHz/4 = 16MHz
    OSCCON1bits.NOSC = 0b110;   // HFINTOSC
    
    // Wait for oscillator to be ready
    while (!OSCCON3bits.ORDY);
}

static void BSP_InitGPIO(void) {
    // Initialize Main Light Control
    LIGHT_TRIS = 0;     // Output
    LIGHT_LAT = 0;      // Initially OFF
    
    // Initialize Heartbeat LED
    HEARTBEAT_TRIS = 0; // Output
    HEARTBEAT_LAT = 0;  // Initially OFF
    
    // Initialize Clock Display (5 LEDs on PORTB)
    CLOCK_DISPLAY_TRIS &= ~CLOCK_DISPLAY_MASK;  // Set as outputs
    CLOCK_DISPLAY_LAT &= ~CLOCK_DISPLAY_MASK;   // Initially all OFF
    
    // Initialize LDR input
    LDR_TRIS = 1;       // Input
    LDR_ANSEL = 1;      // Analog
}

static void BSP_InitADC(void) {
    // Configure ADCC (ADC with Computation)
    
    // Clock: FOSC/64 (16MHz/64 = 250kHz, within spec)
    ADCLK = 0x1F;  // FOSC/64
    
    // Acquisition time: 10 TAD
    ADACQ = 10;
    
    // Result format: right justified
    ADCON0bits.ADFM = 1;
    
    // Configure for Burst-Average mode (hardware accumulation)
    ADCON2bits.MD = 0b010;  // Burst-Average mode
    
    // Configure for 32 samples accumulation (2^5 = 32)
    ADRPT = 32;  // Repeat count
    
    // Positive reference: VDD
    ADREF = 0x00;
    
    // Enable ADC
    ADCON0bits.ADON = 1;
}

static void BSP_InitTimer0(void) {
    // Configure Timer0 for periodic interrupts
    
    // Timer0 Control Register
    T0CON0bits.T0EN = 0;    // Disable during configuration
    
    // 16-bit mode, post-scaler 1:1, prescaler 1:256
    T0CON0bits.T016BIT = 1; // 16-bit mode
    T0CON1bits.T0CS = 0b010; // Clock source = Fosc/4
    T0CON1bits.T0ASYNC = 0;  // Synchronized
    T0CON1bits.T0CKPS = 0b1000; // Prescaler 1:256
    
    // Load initial value
    TMR0H = TMR0_RELOAD_HIGH;
    TMR0L = TMR0_RELOAD_LOW;
    
    // Enable Timer0 interrupt
    PIR0bits.TMR0IF = 0;    // Clear interrupt flag
    PIE0bits.TMR0IE = 1;    // Enable Timer0 interrupt
    INTCON0bits.GIE = 1;    // Enable global interrupts
    INTCON0bits.IPEN = 0;   // Disable priority levels (use legacy mode)
    
    // Enable Timer0
    T0CON0bits.T0EN = 1;
}
