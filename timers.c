#include "timers.h"

static volatile bool tick_pending = false;

void Timers_Init(void) {
    // Timer0 Setup
    // T0CON0: T0EN=1, T016BIT=1, T0OUTPS=1:1
    // T0CON1: T0CS=Fosc/4 (010), T0ASYNC=0, T0CKPS=1:256 (1000) for normal, or 1:16 for test
    
    T0CON0 = 0x90; // Enabled, 16-bit
    
#ifdef TEST_MODE
    T0CON1 = 0x44; // Fosc/4, 1:16 prescaler
    // For 10ms: 16MHz / 16 = 1MHz -> 10,000 counts
    // 65536 - 10000 = 55536 (0xD8F0)
    TMR0H = 0xD8;
    TMR0L = 0xF0;
#else
    T0CON1 = 0x48; // Fosc/4, 1:256 prescaler
    // For 1000ms: 16MHz / 256 = 62.5kHz -> 62,500 counts
    // 65536 - 62500 = 3036 (0x0BDC)
    TMR0H = 0x0B;
    TMR0L = 0xDC;
#endif

    // Clear interrupt flag and enable interrupt
    PIR3bits.TMR0IF = 0;
    PIE3bits.TMR0IE = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

volatile bool Timers_IsTickPending(void) {
    return tick_pending;
}

void Timers_ClearTick(void) {
    tick_pending = false;
}

void __interrupt() ISR(void) {
    if (PIR3bits.TMR0IF) {
        PIR3bits.TMR0IF = 0;
        
#ifdef TEST_MODE
        TMR0H = 0xD8;
        TMR0L = 0xF0;
#else
        TMR0H = 0x0B;
        TMR0L = 0xDC;
#endif
        tick_pending = true;
    }
}

