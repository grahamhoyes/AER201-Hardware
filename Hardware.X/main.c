/*
 * File:   main.c
 * Author: Graham Hoyes
 *
 * Created on January 20, 2018, 6:27 PM
 */

#include <xc.h>
#include <string.h>
#include "configBits.h"
#include "lcd.h"
#include "lcd_extras.h"
#include "hardware.h"

const char inputEntryQuestions[4][33] = { // Putting in an extra bit for the null terminator need to adjust code elsewhere
    "Assembly steps\n"        "*<-  (4-8)     \0",
    "Fasteners in Cx\n"       "*<-(BNSW)    ->#\0",
    "How many sets?\n"        "*<-  (1-4)     \0",
    "Confirm?\n"              "*<-  (A:Y/B:N) \0"
};

const char LCDMenuIcons[16] = "*<-   ( - )    \0";

const struct errorMessages {
    const char badEntry[32];
    const char tooManyFasteners[32];
    const char tooManyBolts[32];
    const char tooManyNuts[32];
    const char tooManyWashers[32];
    const char tooManySpacers[32];
    const char noFasteners[32];
} errMsgs = {
    "Invalid entry\nPlease try again\0",
    "Too many fasteners\0",
    "Too many bolts\0",
    "Too many nuts\0",
    "Too many washers\0",
    "Too many spacers\0",
    "No fasteners selected\0"
};

struct inputParams{
    int steps;
    fS toFill[8];
    int setMultiple[8];
} params;  

const char fastenerMatrix[20][4] = {
    {1,0,0,0},
    {0,1,0,0},
    {0,0,1,0},
    {0,0,0,1},
    {1,1,0,0},
    {1,0,1,0},
    {1,0,0,1},
    {2,1,0,0},
    {2,0,1,0},
    {2,0,0,1},
    {1,1,0,1},
    {1,0,1,1},
    {1,0,0,2},
    {1,1,0,2},
    {1,0,1,2},
    {2,0,1,1},
    {2,1,0,1},
    {1,2,0,1},
    {1,3,0,0},
    {1,0,0,3}
};

/* Encoding of which compartments to fill in */
const char assemblyStepEncoding[5] = {
    /* Note that array indices are 5 below, compartment indices are 1 below*/
    /*
    0b10101010, // 4 steps: C 1,3,5,7
    0b11011010, // 5 steps: C 1,2,4,5,7
    0b11101110, // 6 steps: C 1,2,3,5,6,7
    0b11111110, // 7 steps: C 1,2,3,4,5,6,7
    0b11111111  // 8 steps: C 1,2,3,4,5,6,7,8*/
    
    /* Because of the weird way I do things, these are backwards*/
    0b01010101, // 4 steps: C 7,5,3,1
    0b01011011, // 5 steps: C 7,5,4,2,1
    0b01110111, // 6 steps: C 7,6,5,3,2,1
    0b01111111, // 7 steps: C 7,6,5,4,3,2,1
    0b11111111  // 8 steps: C 8,7,6,5,4,3,2,1
};

int inputEntry(void) {
    __lcd_clear();
    __lcd_home();
    
    int compartmentNum = 0;
    char compartmentLabel[2] = "C0";
    int inputEntryStep = 0;
    int done = 0;
    
    int i, numPressed, doneCompartment, numB, numN, numS, numW, found;
    unsigned char pressed;
    
    while (!done) {
        /* Getting # of assembly steps */
        if (inputEntryStep == 0) {
            printStringLCD(inputEntryQuestions[inputEntryStep]);
            
            pressed = pollKeypad();
            numPressed = pressed - 48;
            
            putch(pressed); // Put their selection to to the LCD
            __delay_ms(500); // Delay for dramatic effect
            
            if (numPressed >= 4 && numPressed <= 8) {
                params.steps = numPressed;
                inputEntryStep++;
            } else printErrorLCD(errMsgs.badEntry);
        } else if (inputEntryStep == 1) {
            /* Getting fastener set for each compartment */
            char compartmentsToFill = assemblyStepEncoding[params.steps - 4];
            for (compartmentNum = 0; compartmentNum < params.steps; compartmentNum++) {
                if ((compartmentsToFill >> compartmentNum) & 0b1) { // Tells us if this compartment needs to be filled in
                    /* First, logic pertaining to which fastener set */
                    int setIsGood = 0;
                    numB = 0;
                    numN = 0;
                    numS = 0;
                    numW = 0;
                    char fastenerString[32];
                    strcpy(fastenerString, inputEntryQuestions[inputEntryStep]);
                    fastenerString[14] = compartmentNum + 1 + 48; // Replace 'x' with the compartment number
                    
                    printStringLCD(fastenerString);
                    lcd_set_cursor(9, 1);
                    doneCompartment = 0;
                    
                    while(doneCompartment < 4) {
                        pressed = pollKeypad();
                        if (pressed == 66 || pressed == 78 || pressed == 83 || pressed == 87) {
                            putch(pressed); // Put their selection to the LCD
                            switch(pressed) {
                                case 66:
                                    numB++;
                                    break;
                                case 78:
                                    numN++;
                                    break;
                                case 83:
                                    numS++;
                                    break;
                                case 87:
                                    numW++;
                                    break;
                                default:
                                    break;
                            }
                            doneCompartment++;
                        } else if (pressed == 35) { // #: done
                            doneCompartment = 4;
                        } else if (pressed == 42) { // *: Go back
                            compartmentNum--;
                            break;
                        }
                    }
                    
                    /* Determine which fastener set we're dealing with */
                    found = 0;
                    for (i = 0; i < 20; i++) {
                        if (fastenerMatrix[i][0] == numB && 
                            fastenerMatrix[i][1] == numN &&
                            fastenerMatrix[i][2] == numS &&
                            fastenerMatrix[i][3] == numW) 
                        {
                            params.toFill[compartmentNum] == i; // i will correspond to the enum fS
                            found = 1;
                        }
                    }  
                    if (!found) {
                        printErrorLCD(errMsgs.badEntry);
                        compartmentNum--; // So that it does this compartment again
                    }
                }
            }
            inputEntryStep++;
        }
    }
    
}

void main(void) {
    // <editor-fold defaultstate="collapsed" desc="Machine Configuration">
    /* Write outputs to LATx, read inputs from PORTx depending if pin has been
     * configured through TRISx to be input (=1) or output (=0). Write all pins
     * to logic low (0) initially to prevent startup issues. */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;

    /* Set the data direction registers */
    TRISA = 0xFF; // Inputs
    TRISB = 0xFF; // Inputs, necessary for keypad
    TRISC = 0x00; // Outputs initially. RC4 and RC5 used for I2C communication.
    TRISD = 0x00; // Outputs initially
    TRISE = 0x00; // Outputs initially
    // </editor-fold>
    
    /* Get current run machine configuration from user */
    //inputEntry();
    
    initLCD();
    inputEntry();
    
//    const char doneString[] = "All done!";
//    printStringLCD(doneString);
    
    while(1);
}
