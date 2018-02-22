/* 
 * File:   helpers.h
 * Author: Graham
 *
 * Created on February 22, 2018, 4:49 PM
 */

#ifndef HELPERS_H
#define	HELPERS_H

const char keypadChars[] = "123B456N789S*0#W"; 

void printStringLCD(char * string);
void printErrorLCD(char * string);
char pollKeypad(void);
void I2C_Send(unsigned char address, char * data);

#endif	/* HELPERS_H */

