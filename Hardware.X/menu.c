#include "menu.h"
#include "lcd.h"
#include "helpers.h"
#include "hardware.h"

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
        if (pressed - 48 == 0) {
            hibernate();
            needToPrint = 1;
        } else if (pressed - 48 == 1) {
            inputEntry();
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
            }
        } else if (pressed - 48 == 2) {
            viewLogs();
            needToPrint = 1;
        }
    }
}
