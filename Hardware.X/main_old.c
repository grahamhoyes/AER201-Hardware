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
#include "hardware.h"
#include "menu.h"

const char inputEntryQuestions[7][32] = {
    "Assembly steps\n"        "*<-  (4-8)     \0",
    "Sets per step\n"         "*<-  (1-4)     \0",
    "# of bolts\n"            "*<-  (0-2)     \0",
    "# of nuts\n"             "*<-  (0-3)     \0",
    "# of spacers\n"          "*<-  (0-1)     \0",
    "# of washers\n"          "*<-  (0-3)     \0",
    "Confirm?\n"              "*<-  (A:Y/B:N) \0"
};

struct inputEntryPrompts {
    char steps[32];
    char sets[32];
    char bolts[32];
    char nuts[32];
    char spacers[32];
    char washers[32];
    char confirm[32];
} inputPrompts = {
    "Assembly steps\n"        "*<-  (4-8)     \0",
    "Sets per step\n"         "*<-  (1-4)     \0",
    "# of bolts\n"            "*<-  ( - )     \0",
    "# of nuts\n"             "*<-  ( - )     \0",
    "# of spacers\n"          "*<-  ( - )     \0",
    "# of washers\n"          "*<-  ( - )     \0",
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
    int steps, sets, bolts, nuts, spacers, washers;
    char setString;
} params;

const char keypadChars[] = "123A456B789C*0#D";   

/*enum fastenerSets {B, N, S, W, BN, BS, BW, BBN, BBS, BBW, BNW, BSW, 
                   BWW, BNWW, BSWW, BBSW, BBNW, BBNW, BBNW, BNNW, BNNN, BWWW};*/
int printStringLCD(char *string) {
    /* Input: char array pointer to a string of up to 32 digits to be printed
     * to the LCD 
     * Output: 0 if the whole string was printed, 1 if not
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

char * getMessage(struct menuOption * opt) {
    switch (opt->type) {
        case B:
            return &inputPrompts.bolts;
        case N:
            return &inputPrompts.nuts;
        case S:
            return &inputPrompts.spacers;
        case W:
            return &inputPrompts.washers;
        case steps:
            return &inputPrompts.steps;
        case sets:
            return &inputPrompts.sets;
        default:
            return 0;
    }
}

int inputEntryNew (struct menuOption * start) { // We start with assembly steps
    __lcd_clear();
    __lcd_home();
    
    initMenu();
    struct menuOption * curr = start;
    int done = 0;
    char chosenSet[5];
    
    while (!done) {
        char msgA[32], msgB[16];
        strcpy(msgA, getMessage(curr->type));
        strcpy(msgB, LCDMenuIcons); 
        msgB[6] = curr->min + 48;
        msgB[8] = curr->max + 48;
        strcat(msgA, msgB);
        
        printStringLCD(msgA);
        unsigned char pressed = pollKeypad();
        int numPressed = pressed - 48;
        
        putch(pressed);
        
        if (curr->end) {
            done = 1;
            strcpy(chosenSet, curr->name);
            continue;
        } else {
            if (pressed == 42) {
                curr = curr->prev;
                continue;
            } else if (numPressed >= curr->min && numPressed <= curr->max) {
                switch(curr->type) {
                    case steps:
                        params.steps = numPressed;
                        break;
                    case sets:
                        params.sets = numPressed;
                        break;
                    case B:
                        params.bolts = numPressed;
                        break;
                    case N:
                        params.nuts = numPressed;
                        break;
                    case S:
                        params.spacers = numPressed;
                        break;
                    case W:
                        params.spacers = numPressed;
                        break;
                    default: // done case?
                        break;
                }
                __delay_ms(500);
            } else printErrorLCD(errMsgs.badEntry);
        }
        
    }
    printStringLCD(chosenSet);
    return 0;
}

//int inputEntry(void) {
//    /*
//     * Output: 1 if invalid entry, 0 if successful
//     */
//    
//    // Currently it's possible to recognize bad sets, e.g. BNN
//    // Solution: just scroll through options?
//    /*
//     *  ------------------   ------------------  ------------------
//     *  |1:B 2:N 3:S 4:W |   |1:BN 2:BS 3:BW  |  |1:BBN 2:BBS     |
//     *  |*<-   1/9   2->#|   |*<-1  2/9   3->#|  |*<-2  3/9   4->#|
//     *  ------------------   ------------------  ------------------
//     *  ------------------   ------------------  ------------------
//     *  |1:BBW 2:BNW     |   |1:BSW 2:BWW     |  |1:BNWW 2:BSWW   |
//     *  |*<-3  4/9   4->#|   |*<-4  5/9   6->#|  |*<-5  6/9   7->#|
//     *  ------------------   ------------------  ------------------
//     *  ------------------   ------------------  ------------------
//     *  |1:BBSW 2:BBNW   |   |1:BBNW 2:BNNW   |  |1:BNNN 2:BWWW   |
//     *  |*<-6  7/9   8->#|   |*<-7  8/9   9->#|  |*<-8  9/9    ->#|
//     *  ------------------   ------------------  ------------------
//     */
//    
//    //initLCD(); // Initialize the LCD
//    __lcd_clear(); // Clear the LCD
//    __lcd_home();
//    
//    int inputEntryStep = 0;
//    int done = 0;
//    while (!done) {
//        
//        printStringLCD(inputEntryQuestions[inputEntryStep]);
//                
//        unsigned char pressed = pollKeypad();
//        int numPressed = pressed - 48;
//        
//        putch(pressed); // Put their selection to the LCD;
//        
//        if (pressed == 42) {
//            inputEntryStep = inputEntryStep == 0 ? 0 : inputEntryStep-1;
//            continue;
//        } else __delay_ms(500); // Delay for dramatic effect
//        
//        if (inputEntryStep == 0 && numPressed >= 4 && numPressed <= 8) {
//            params.steps = numPressed;
//            inputEntryStep++;
//        } else if (inputEntryStep == 1 && numPressed >= 1 && numPressed <= 4) {
//            params.sets = numPressed;
//            inputEntryStep++;
//        } else if (inputEntryStep == 2 && numPressed >= 0 && numPressed <= 2) {
//            params.bolts = numPressed;
//            inputEntryStep++;
//        } else if (inputEntryStep == 3 && numPressed >= 0 && numPressed <= 3) {
//            params.nuts = numPressed;
//            inputEntryStep++;
//        } else if (inputEntryStep == 4 && numPressed >= 0 && numPressed <= 1) {
//            params.spacers = numPressed;
//            inputEntryStep++;
//        } else if (inputEntryStep == 5 && numPressed >= 0 && numPressed <= 3) {
//            params.washers = numPressed;
//            inputEntryStep++;
//            done = 1;
//        } else if (inputEntryStep == 6) {
//            continue; // This is now below
//        } else printErrorLCD(errMsgs.badEntry);
//    }
//    
//    /* Check that inputs conform to greater constraints:
//     *  Max number of fasteners per step is 4
//     *      sum(B, N, S, W)*sets
//     *  Max number of fasteners is 2B, 3N, 2S, 4W
//     */
//    
//    if ((params.bolts + params.nuts + params.spacers + params.washers)*params.sets > 4) {
//        printErrorLCD(errMsgs.tooManyFasteners);
//        return 1;
//    } else if (params.bolts + params.nuts + params.spacers + params.washers == 0) {
//        printErrorLCD(errMsgs.noFasteners);
//        return 1;
//    } else if (params.bolts * params.sets > 2) {
//        printErrorLCD(errMsgs.tooManyBolts);
//        return 1;
//    } else if (params.nuts * params.sets > 3) {
//        printErrorLCD(errMsgs.tooManyNuts);
//        return 1;
//    } else if (params.spacers * params.sets > 2) {
//        printErrorLCD(errMsgs.tooManySpacers);
//        return 1;
//    } else if (params.washers * params.sets > 4) {
//        printErrorLCD(errMsgs.tooManyWashers);
//        return 1;
//    } else {
//        // Assembling the fastener set string
//        // This is also a final check for too many fasteners
//        char confirmation[32] = "Confirm:     \n*<-  (A:Y/B:N) \0";
//        
//        int j = 9;
//        int k;
//        char thisOne[] = "We got to 1";
//        char thisTwo[] = "We got to 2";
//        for (k = 0; k < params.bolts; k++) {
//            if (j < 13) confirmation[j++] = 66; // B
//            else {
//                
//                printErrorLCD(thisOne);
//                return 1;
//            }
//        }
//        for (k = 0; k < params.nuts; k++) {
//            if (j < 13) confirmation[j++] = 78; // N
//            else {
//                printErrorLCD(thisTwo);
//                return 1;
//            }
//        }
//        for (k = 0; k < params.spacers; k++) {
//            if (j < 13) confirmation[j++] = 83; // S
//            else { 
//                printErrorLCD(errMsgs.tooManyFasteners);
//                return 1;
//            }
//        }
//        for (k = 0; k < params.washers; k++) {
//            if (j < 13) confirmation[j++] = 87; // W
//            else {
//                printErrorLCD(errMsgs.tooManyFasteners);
//                return 1;
//            }
//        }
//        
//        while (1) {
//            printStringLCD(confirmation);
//            unsigned char pressed = pollKeypad();
//            if (pressed == 65) {
//                return 0;
//            } else if (pressed == 66) {
//                return 1;
//            } else {
//                printErrorLCD(errMsgs.badEntry);
//            }
//        }
//    }
//    return 0;
//}
struct menuOption mBBNW = {0, 0, 1, "BBNW\0"};
struct menuOption mBBN = {0, 0, 1, "BBN\0"};
struct menuOption mBBSW = {0, 0, 1, "BBSW\0"};
struct menuOption mBBS = {0, 0, 1, "BBS\0"};
struct menuOption mBBW = {0, 0, 1, "BBW\0"};
struct menuOption mBNNN = {0, 0, 1, "BNNN\0"};
struct menuOption mBNNW = {0, 0, 1, "BNNW\0"};
struct menuOption mBNWW = {0, 0, 1, "BNWW\0"};
struct menuOption mBNW = {0, 0, 1, "BNW\0"};
struct menuOption mBN = {0, 0, 1, "BN\0"};
struct menuOption mBSWW = {0, 0, 1, "BSWW\0"};
struct menuOption mBSW = {0, 0, 1, "BSW\0"};
struct menuOption mBS = {0, 0, 1, "BS\0"};
struct menuOption mBWWW = {0, 0, 1, "BWWW\0"};
struct menuOption mBWW = {0, 0, 1, "BWW\0"};
struct menuOption mBW = {0, 0, 1, "BW\0"};
struct menuOption mB = {0, 0, 1, "B\0"};
struct menuOption mW = {0, 0, 1, "W\0"};
struct menuOption mS = {0, 0, 1, "S\0"};
struct menuOption mN = {0, 0, 1, "N\0"};

/* Intermediate states */
struct menuOption mOne = {0, 1, 0, "\0", W, {&mBBN, &mBBNW}};
struct menuOption mTwo = {0, 1, 0, "\0", W, {&mBBS, &mBBSW}};
// mThree goes straight to BBW
struct menuOption mFour = {0, 1, 0, "\0", S, {&mBBW, &mTwo}};
// mFive goes straight to BNNN
// mSix goes straight to BNNW
struct menuOption mSeven = {0, 2, 0, "\0", W, {&mBN, &mBNW, &mBNWW}};
struct menuOption mEight = {0, 2, 0, "\0", W, {&mBS, &mBSW, &mBSWW}};
struct menuOption mNine = {0, 3, 0, "\0", W, {&mB, &mBW, &mBWW, &mBWWW}};
struct menuOption mTen = {0, 1, 0, "\0", S, {&mNine, &mEight}};
// mEleven goes straight to W
struct menuOption mTwelve = {0, 1, 0, "\0", S, {&mW, &mS}};
struct menuOption mThirteen = {0, 1, 0, "\0", N, {&mFour, &mOne}};
struct menuOption mFourteen = {0, 3, 0, "\0", N, {&mTen, &mSeven, &mBNNW, &mBNNN}};
struct menuOption mFifteen = {0, 1, 0, "\0", N, {&mTwelve, &mN}};
struct menuOption mStart = {0, 2, 0, "\0", B, {&mFifteen, &mFourteen, &mThirteen}};

/* The beginning options */
struct menuOption mSets = {1, 4, 0, "\0", sets, {&mStart, &mStart, &mStart, &mStart}};
struct menuOption mSteps = {4, 8, 0, "\0", steps, {&mSets, &mSets, &mSets, &mSets}};
void initMenu(void) {
    mBBNW.prev = &mOne;
    mBBN.prev = &mOne;
    mBBSW.prev = &mTwo;
    mBBS.prev = &mTwo;
    mBBW.prev = &mFour;
    mBNNN.prev = &mFourteen;
    mBNNW.prev = &mFourteen;
    mBNWW.prev = &mSeven;
    mBNW.prev = &mSeven;
    mBN.prev = &mSeven;
    mBSWW.prev = &mEight;
    mBSW.prev = &mEight;
    mBS.prev = &mEight;
    mBWWW.prev = &mFour;
    mBWW.prev = &mFour;
    mBW.prev = &mFour;
    mB.prev = &mFour;
    mW.prev = &mTwelve;
    mS.prev = &mTwelve;
    mN.prev = &mFifteen;
    
    /* Intermediate states */
    mOne.prev = &mThirteen;
    mTwo.prev = &mFour;
    mFour.prev = &mThirteen;
    mSeven.prev = &mFourteen;
    mEight.prev = &mTen;
    mNine.prev = &mTen;
    mTen.prev = &mFourteen;
    mTwelve.prev = &mFifteen;
    mThirteen.prev = &mStart;
    mFourteen.prev = &mStart;
    mFifteen.prev = &mStart;
    mStart.prev = &mSets;
    
    /* In the beginning... */
    mSteps.prev = &mSteps;
    mSets.prev = &mSteps;
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
    initMenu();
    inputEntryNew(&mSteps);
    
//    const char doneString[] = "All done!";
//    printStringLCD(doneString);
    
    while(1);
}
