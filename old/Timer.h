/* 

 * File:   Timer.h


 * Purpose:  This file creates a system tick timer which generates regular time intervals.  

 */

#ifndef TIMER_H

#define TIMER_H

#include <stdint.h> // for uint32_t 



void Timer_Init(void); // initialise timer where it sets up Timer0 and interrupts 

uint32_t Timer_GetTicks(void); // get current tick count by measuring elasped time

void Timer_ResetTicks(void); // resets tick count to 0 after 24 seconds

#endif 
