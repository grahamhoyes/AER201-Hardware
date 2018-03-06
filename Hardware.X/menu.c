#include "menu.h"
#include "lcd.h"
#include "I2C.h"
#include "helpers.h"
#include "RTC.h"
#include "hardware.h"
#include "stdio.h"

void hibernate(void) {
    printStringLCD("Press 1 to begin");
    int pressed;
    while (1) {
        pressed = pollKeypad();
        if (pressed - 48 == 1) {
            return;
        }
    }
}

void viewLogs() {
    printStringLCD("Not there yet\n* to return");
    while (1) {
        int pressed = pollKeypad();
        if (pressed == 42) return;
    }
}

void mainMenu(void) {
    int pressed;
    int needToPrint = 1; 
    
    while (1) {
        if (needToPrint) { // Save some processor cycles
            printStringLCD("0:Sleep 1:Begin \n2: View Logs");
            needToPrint = 0;
        }
        pressed = pollKeypad();
        putch(pressed);
        if (pressed - 48 == 0) { // Sleep
            hibernate();
            needToPrint = 1;
        } else if (pressed - 48 == 1) { // Begin
            inputEntry(); // Get user input parameters
            long startTime = RTC_getSeconds();
            __lcd_clear();
            __lcd_home();
            printf("Starting packaging");
            I2C_Send(nanoAddr, "\1Starting packaging\0");
            packaging(); // Fill the box
            clearing(); // Empty and reset the machine
            long endTime = RTC_getSeconds();
            long totalTime = endTime - startTime;
            
            printf("Summary:");
            __delay_ms(1000);
            int i;
            while(1) {
                for (i = 0; i < 8; i++) {
                    __lcd_clear();
                    __lcd_home();
                    printf("C%d: %s x%d", i+1, fSLookup[params.toFill[i]], params.setMultiple[i]);
                    __delay_ms(2000);
                }
                // Print remaining amounts
                __lcd_clear();
                __lcd_home();
                printf("Remaining:");
                __lcd_newline();
                printf("B%d N%d S%d W%d", extras.b, extras.n, extras.s, extras.w);
                __delay_ms(2000);
                
            }
        } else if (pressed - 48 == 2) { // View logs
            viewLogs();
            needToPrint = 1;
        }
    }
}
