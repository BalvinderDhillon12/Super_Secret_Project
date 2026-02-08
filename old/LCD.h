/* 
 * File:   LCD.h

 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

void LCD_Init(void);
void LCD_UpdateDisplay(uint8_t hours, uint8_t minutes, bool is_summer, uint16_t light_adc);
void LCD_Clear(void);

#endif /* LCD_H */


