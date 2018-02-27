/*
 * File:   main.c
 * Author: Graham Hoyes
 *
 * Created on January 20, 2018, 6:27 PM
 */

/* TODO
 * Make a proper function for debouncing and counting IR break beam sensor
 */
#include <xc.h>
#include <string.h>
#include "configBits.h"
#include "lcd.h"
#include "I2C.h"
#include "helpers.h"
#include "menu.h"
#include "hardware.h"
#include "RTC.h"

#define maxB 20
#define maxN 25
#define maxS 20
#define maxW 35

/* Global constant declarations */
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

const char fastenerMatrix[21][4] = {
    {0,0,0,0}, // The default for an empty cell
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
    /* Because of the weird way I do things, these are backwards*/
    0b01010101, // 4 steps: C 7,5,3,1
    0b01011011, // 5 steps: C 7,5,4,2,1
    0b01110111, // 6 steps: C 7,6,5,3,2,1
    0b01111111, // 7 steps: C 7,6,5,4,3,2,1
    0b11111111  // 8 steps: C 8,7,6,5,4,3,2,1
};

const unsigned char nanoAddr = 0b00010000; // Arduino address LSL 1

struct amounts dispensed = {0, 0, 0, 0};

struct amounts extras = {0, 0, 0, 0};

int inputEntry(void) {
    __lcd_clear();
    __lcd_home();
   
    I2C_Send(nanoAddr, "Started input entry, hi\0");
    
    int compartmentNum = 0;
    char compartmentLabel[2] = "C0";
    int inputEntryStep = 0;
    int done = 0;
    
    int i, numPressed, doneCompartment, numB, numN, numS, numW, found, numFasteners;
    int setIsGood, doneMultiples;
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
            for (compartmentNum = 0; compartmentNum < 8; compartmentNum++) {
                STARTCOMPARTMENT: 
                if ((compartmentsToFill >> compartmentNum) & 0b1) { // Tells us if this compartment needs to be filled in
                    /* First, logic pertaining to which fastener set */
                    setIsGood = 0;
                    while (!setIsGood) {
                        numB = 0;
                        numN = 0;
                        numS = 0;
                        numW = 0;
                        char fastenerString[32];
                        strcpy(fastenerString, inputEntryQuestions[inputEntryStep]); // "Fasteners in Cx"
                        fastenerString[14] = compartmentNum + 1 + 48; // Replace 'x' with the compartment number

                        printStringLCD(fastenerString);
                        lcd_set_cursor(9, 1);
                        doneCompartment = 0;

                        while(doneCompartment < 4) {
                            pressed = pollKeypad();
                            if (pressed == 66 || pressed == 78 || pressed == 83 || pressed == 87) {
                                putch(pressed); // Put their selection to the LCD
                                if (pressed == 66) numB++;
                                else if (pressed == 78) numN++;
                                else if (pressed == 83) numS ++;
                                else if (pressed == 87) numW++;
                                doneCompartment++;
                            } else if (pressed == 35) { // #: done
                                if (numB != 0 || numN != 0 || numS != 0 || numW != 0) doneCompartment = 4;
                            } else if (pressed == 42) { // *: Go back
                                compartmentNum--;
                                goto STARTMULTIPLES;
                            }
                        }

                        /* Determine which fastener set we're dealing with */
                        found = 0;
                        for (i = 0; i < 21; i++) {
                            if (fastenerMatrix[i][0] == numB && 
                                fastenerMatrix[i][1] == numN &&
                                fastenerMatrix[i][2] == numS &&
                                fastenerMatrix[i][3] == numW) 
                            {
                                params.toFill[compartmentNum] = i; // i will correspond to the enum fS
                                found = 1;
                            }
                        }  
                        if (!found) {
                            printErrorLCD(errMsgs.badEntry);
                            // Compartment will be done again
                            //compartmentNum--; // So that it does this compartment again
                            continue;
                        } else {
                            setIsGood = 1;
                        }
                    }
                    /* Next, figure out how many multiples*/
                    
                    STARTMULTIPLES:
                    doneMultiples = 0;
                    while (!doneMultiples) {
                        printStringLCD(inputEntryQuestions[2]);
                        lcd_set_cursor(14, 1);

                        pressed = pollKeypad();
                        numPressed = pressed - 48;
                        
                        if (pressed == 42) goto STARTCOMPARTMENT; // *: go back
                        /* Caveat: If */
                        
                        putch(pressed);
                        __delay_ms(500); // For dramatic effect
                        
                        int sum = 0;
                        for (i = 0; i < 4; i++) {
                            sum += fastenerMatrix[params.toFill[compartmentNum]][i];
                        };
                        if (numPressed * sum > 4 || numPressed * sum <= 0) {
                            printErrorLCD(errMsgs.tooManyFasteners);
                        // There is also a max number of B, N, S, W in each compartment, need to check for that as well here
                        } else {
                            params.setMultiple[compartmentNum] = numPressed;
                            doneMultiples = 1;
                        }
                    }
                } else {
                    params.toFill[compartmentNum] = NONE;
                    params.setMultiple[compartmentNum] = 0;
                }
            }
            done = 1;
            // Right now on 4 steps, we are looping between C1 and C3
        }
        
    }
}

void packageCompartment(char b, char n, char s, char w) {
    // Activate all motors
    LATBbits.LATB3 = 1;
    LATCbits.LATC1 = 1;
    LATCbits.LATC5 = 1;
    LATCbits.LATC7 = 1;
    I2C_Send(nanoAddr, "\1Starting\0");
    
    int numB=0, numN=0, numS=0, numW=0;
    int doneB=0, doneN=0, doneS=0, doneW=0;
    long currTime, timeB=0, timeN=0, timeS=0, timeW=0;
    char resolution = 5;
    __lcd_clear();
    __lcd_home();
    printf("Now counting");
    __delay_ms(2000);
    
    while (1) {
        currTime = RTC_getSeconds();
        char currTimeString[33];
        sprintf(currTimeString, "%li", currTime);
        currTimeString[32] = "\0"; // Null-terminate
        __lcd_clear();
        __lcd_home();
        printf(currTimeString);
        __delay_ms(2000);
        I2C_Send(nanoAddr, currTimeString);
        
        if (PORTAbits.RA0 == 0 && currTime > timeB + resolution) {
            timeB = currTime;
            numB++;
            dispensed.b++;
            I2C_Send(nanoAddr, "\1Bolt Counted\0");
        }
        if (PORTAbits.RA1 == 0 && currTime > timeN + resolution) {
            timeN = currTime;
            numN++;
            dispensed.n++;
            I2C_Send(nanoAddr, "\1Nut Counted\0");
        }
        if (PORTAbits.RA2 == 0 && currTime > timeS + resolution) {
            timeS = currTime;
            numS++;
            dispensed.s++;
            I2C_Send(nanoAddr, "\1Spacer Counted\0");
        }
        if (PORTAbits.RA3 == 0 && currTime > timeW + resolution) {
            timeW = currTime;
            numW++;
            dispensed.w++;
            I2C_Send(nanoAddr, "\1Washer Counted\0");
        }
        
        if (numB >= b) {
            LATBbits.LATB3 = 0; 
            doneB=1;
        }
        if (numN >= n) {
            LATCbits.LATC5 = 0;
            doneN=1;
        }
        if (numS >= s) {
            LATCbits.LATC5 = 0;
            doneS=1;
        }
        if (numW >= w) {
            LATCbits.LATC7 = 0;
            doneW=1;
        }
        
        if (doneB && doneN && doneS && doneW) {
            I2C_Send(nanoAddr, "\1Done compartment\0");
            break;
        }
    }
    //I2C_Send(nanoAddr, "Done\0");
    // Rotate Nema 17 45deg CW
    I2C_Send(nanoAddr, 2); // We should actually wait until this completes, approx 4 * 50 ms * 200/8
    __delay_ms(5000); // Yeah, we'll want to speed this up after testing
}

void packaging(void) {
    int compartmentNum;
    /* We start packaging with Compartment 8, then 7, etc. down to 1
     * This is C, so compartments are actually zero-indexed in the code
     */
    for (compartmentNum = 8; compartmentNum > 0; compartmentNum--) {
        if (params.toFill[compartmentNum-1] == NONE) break;
        char * set = fastenerMatrix[params.toFill[compartmentNum-1]];
        packageCompartment(set[0], set[1], set[2], set[3]);
    }
}

void clearing(void) {
    /* This function involves a lot of coordination between the PIC and 
     * Arduino. For now, we're keeping track of how long motor operations
     * on the Arduino take, and delaying for the appropriate time here.*/
    
    I2C_Send(nanoAddr, 3); // Rotate Nema 17 359.5 deg CW, step micro servo over first dispensing bin
    
    int i;
    int spinTime=100; // Change this in practice
    
    LATBbits.LATB3 = 1; // DCB-F pin
    for (i = 0; i < spinTime; i++) {
        // Every 25 milliseconds, check if a bolt is passed and count it
        if (PORTAbits.RA0 == 0) {
            extras.b++;
        }
        if (extras.b + dispensed.b == maxB) break;
        __delay_ms(25);
    }
    LATBbits.LATB3 = 0;
    
    I2C_Send(nanoAddr, 4); // Step micro servo over second dispensing bin
    
    LATCbits.LATC1 = 1; // DCN-F pin
    for (i = 0; i < spinTime; i++) {
        if (PORTAbits.RA1 == 0) {
            extras.n++;
        }
        if (extras.n + dispensed.n == maxN) break;
        __delay_ms(25);
    }
    LATCbits.LATC1 = 0;
    
    I2C_Send(nanoAddr, 5); // Step micro servo over third dispensing bin
    
    LATCbits.LATC5 = 1; // DCS-F pin
    for (i = 0; i < spinTime; i++) {
        if (PORTAbits.RA2 == 0) {
            extras.s++;
        }
        if (extras.s + dispensed.s == maxS) break;
        __delay_ms(25);
    }   
    LATCbits.LATC5 = 0;
    
    I2C_Send(nanoAddr, 6); // Step micro servo over fourth dispensing bin
    
    LATCbits.LATC7 = 1; // DCW-F pin
    for (i = 0; i < spinTime; i++) {
        if (PORTAbits.RA3 == 0) {
            extras.w++;
        }
        if (extras.w + dispensed.w == maxW) break;
    }
    LATCbits.LATC7 = 0;
    
    I2C_Send(nanoAddr, 7); // Unset box
    // Retract 28BYJ stepper and DC motor holding box down
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
    TRISB = 0b11110111; // Inputs, necessary for keypad
    TRISC = 0x00; // Outputs initially. RC4 and RC5 used for I2C communication.
    TRISD = 0x00; // Outputs initially
    TRISE = 0x00; // Outputs initially
    // </editor-fold>
    
    I2C_Master_Init(100000); // Start I2C with a 100khz clock
    
    initLCD();
    
    __lcd_clear();
    __lcd_home();
    
    //hibernate();
    //mainMenu();
    printf("\1Counting");
    I2C_Send(nanoAddr, "\1This is a test\0");
    packageCompartment(4, 4, 4, 4);
    while(1);
    
}
