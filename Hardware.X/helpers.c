#include "lcd.h"
#include "I2C.h"
#include "helpers.h"

/* This file contains various helper functions for sanity */

void printStringLCD(char *string) {
    /* Input: char array pointer to a string of up to 32 digits to be printed
     * to the LCD
     */
    
    int i;
    int j = 1;
    int newlineIndex = 16;
    int newlineCharIndex = -1;
    int spaceIndex = 0;
    
    __lcd_clear();
    __lcd_home();
    
    /* Figure out where the string need to be split for new line */
    for (i = 0; i < 17; i++) { // i: character index
        if (string[i] == 0) break;
        if (string[i] == 32) {
            spaceIndex = i;
        }
        
        if (string[i] == 10) { // Set the new line index for new line character
            newlineCharIndex = i;
            newlineIndex = newlineCharIndex;
        }
        
        // No matter what, 16th index will have to go on a new line
        if (i == 16 && string[i] != 32 && newlineCharIndex < 0) {
            newlineIndex = spaceIndex; // The current word must be split
        }
    }    
    
    /* Printing */
    for (i = 0; i < 32; i++) {
        if (string[i] == 0) break;  // null terminator
        /*if (string[i] == 10) {  // New line
            if (j < LCD_SIZE_VERT) {
                __lcd_newline();
                j++;
                continue;
            } else {
                continue;
            }
        }*/
        
        if (i == newlineIndex && j < LCD_SIZE_VERT) {
            j++;
            __lcd_newline();
            if (string[i] == 32 || string[i] == 10) continue;
        }
        
        putch(string[i]); // Print the current letter to the LCD
    }
}

void printErrorLCD(char *string) {
    printStringLCD(string);
    __delay_ms(2000);
} 

/* This gets printed to the LCD eventually, might as well go here */
char pollKeypad(void) {
        // Wait until the keypad is pressed
        while (PORTBbits.RB1 == 0) {continue;} 
        
        // Read from the keypad
        unsigned char keypress = (unsigned char)(PORTB & 0xF0) >> 4;
        
        // Wait until key has been released
        while (PORTBbits.RB1 == 1) {continue;}
        
        Nop();  // Apply breakpoint here to prevent compiler optimizations
        
        return keypadChars[keypress];
}

void I2C_Send(unsigned char address, char * data) {
    /* Writes a sequence of bytes to the device currently addressed
     * Arguments: address, the device to send to
     *            data, pointer to data to write (null-terminated)
     */
    int i=0;
    
    I2C_Master_Start();  // Start condition
    I2C_Master_Write(address); // 7-bit Slave address + write
    
    while (data[i] != 0) {
        I2C_Master_Write(data[i]);
        i++;
    }
    
    I2C_Master_Stop();
    return;
}