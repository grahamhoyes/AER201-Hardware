/* 
 * File:   lcd_extras.h
 * Author: Graham
 *
 * Created on January 25, 2018, 10:06 PM
 */

#ifndef LCD_EXTRAS_H
#define	LCD_EXTRAS_H

const char keypadChars[] = "123B456N789S*0#W"; 

void printStringLCD(char * string);
void printErrorLCD(char * string);
char pollKeypad(void);
void I2C_Send(unsigned char address, char * data);

#endif	/* LCD_EXTRAS_H */

