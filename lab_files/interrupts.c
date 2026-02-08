#include <xc.h>
#include "interrupts.h"

/************************************
 * Function to turn on interrupts and set if priority is used
 * Note you also need to enable peripheral interrupts in the INTCON register to use CM1IE.
************************************/
void Interrupts_init(void)
{
	// turn on global interrupts, peripheral interrupts and the interrupt source 
	// It's a good idea to turn on global interrupts last, once all other interrupt configuration is done.
    // PIE2bits.C1IE = 1;      // Enable Comparator 1 Interrupt
    PIE0bits.TMR0IE = 1;    // Enable Timer0 Interrupt
    INTCONbits.PEIE = 1;    // Enable Peripheral Interrupts
    INTCONbits.GIE = 1;     // Turn on Global Interrupts
}

/************************************
 * High priority interrupt service routine
 * Make sure all enabled interrupts are checked and flags cleared
************************************/
void __interrupt(high_priority) HighISR()
{
	//add your ISR code here i.e. check the flag, do something (i.e. toggle an LED), clear the flag...
    
    /* Exercise 1 
    if (PIR2bits.C1IF) { // Check flag
        LATHbits.LATH3 = !LATHbits.LATH3; //Invert the output on RH3, 1 -> 0 or 0 -> 1
        PIR2bits.C1IF = 0; // Clear the flag
    */
    
    // Check if Timer0 Overflow caused the interrupt
    if (PIR0bits.TMR0IF) { 
        TMR0H = 0x0B; //Initialize timmer to value 3035 to count one second precisely
        TMR0L = 0xDB;
        LATHbits.LATH3 = !LATHbits.LATH3; // Invert the output on RH3, 1 -> 0 or 0 -> 1
        PIR0bits.TMR0IF = 0;              // Clear the flag
}