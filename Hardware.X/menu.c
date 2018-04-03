#include "menu.h"
#include <string.h>
#include "lcd.h"
#include "I2C.h"
#include "helpers.h"
#include "RTC.h"
#include "hardware.h"
#include "stdio.h"
#include "timer.h"
#include "EEPROM.h"

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
    // We will display 4 logs
    // Total of 4*4+1 = 17 screens to display
    unsigned int i = 0;
    unsigned int entry = 0;
    printStringLCD("0:Return\n<>:Navigate");
    int pressed;
    while (1) {
        pressed = pollKeypad();
        if (pressed == '0') {
            return;
        } else if (pressed == '>' || pressed == '#') {
            i = (i < 16) ? i+1 : 0;
        } else if (pressed == '<' || pressed == '*') {
            i = (i > 0) ? i-1 : 16;
        }
        
        if (i == 0) {
            printStringLCD("0:Return\n<>:Navigate");
        } else {
            entry = (unsigned int)(i-1)/4;
            struct operationInfo run;
            int success = EEPROM_readLog(entry, &run);
            if (success) {
                char msgs[4][32];
                sprintf(msgs[0], "Date:\n20%02d-%02d-%02d", run.year, run.minutes, run.seconds);
                sprintf(msgs[1], "Packaged:\nB%d N%d S%d W%d", run.packagedB, run.packagedN, run.packagedS, run.packagedW);
                sprintf(msgs[2], "Remaining:\nB%d N%d S%d W%d", run.remainingB, run.remainingN, run.remainingS, run.remainingW);
                sprintf(msgs[3], "Time:\n%d:%d", run.minutes, run.seconds);
                int j = (i-1)%4;
                printStringLCD(msgs[j]);
            } else {
                char msg[32];
                sprintf(msg, "Entry %u\nis empty", entry);
                printStringLCD(msg);
            }
        }
    }
}

void inputEntry(void) {
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
                    char msg[] = "\1Started compartment x\0";
                    msg[21] = compartmentNum + 1 + 48;
                    I2C_Send(nanoAddr, msg);
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
                        if (numPressed * sum > 4) {
                            printErrorLCD(errMsgs.tooManyFasteners);
                        } else if (numPressed * sum <= 0) {
                            printErrorLCD(errMsgs.noFasteners);
                            
                        // Using numB, etc. here (and elsewhere) prevents us from going backwards.
                        // Solution: either always go back to start of compartment, or store these in an array somewhere
                        } else if (numB * numPressed > maxCompB) {
                            printErrorLCD(errMsgs.tooManyBolts);
                        } else if (numN * numPressed > maxCompN) {
                            printErrorLCD(errMsgs.tooManyNuts);
                        } else if (numS * numPressed > maxCompS) {
                            printErrorLCD(errMsgs.tooManySpacers);
                        } else if (numW * numPressed > maxCompW) {
                            printErrorLCD(errMsgs.tooManyWashers);
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
            I2C_Send(nanoAddr, "\1Done inputs\0");
        }
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
            printStringLCD("Press 1 to start\npackaging");
            int startPress = pollKeypad();
            while (startPress - 48 != 1) {
                startPress = pollKeypad();
            }
            RTC_startOperation(); // Begin global timing
            printStringLCD("Packaging\nPlease wait...");
            I2C_Send(nanoAddr, "\1Starting packaging\0");
            packaging(); // Fill the box
            clearing(); // Empty and reset the machine
            long operationTime = RTC_getOperatingTime();
            int operationMinutes = (int)(operationTime/60);
            int operationSeconds = (int)operationTime - 60*operationMinutes;
            
            //while (pollKeypad() != "1") {continue;} // Make sure this works
            
            char summaries[3][32];
            char savedToEEPROM = 0;
            
            sprintf(summaries[0], "0:return 1:save\n<>:view summary");
            printStringLCD(summaries[0]);
            
            // Packaging parameters
            int i;
            // This is now done below dynamically, to save on memory
//            for (i = 1; i < 9; i++) {
//                sprintf(summaries[i], "C%d: %s x%d", i+1, fSLookup[params.toFill[i]], params.setMultiple[i]);
//            }
            
            // Remaining quantities
            sprintf(summaries[1], "Remaining:\nB%d N%d S%d W%d", extras.b, extras.n, extras.s, extras.w);
            
            // Operating time
            sprintf(summaries[2], "Operating time:\n%d:%d", operationMinutes, operationSeconds);
            
            unsigned char pressed;
            i = 0;
            while (1) {
                pressed = pollKeypad();
                if (pressed == '>' || pressed == '#') {
                    i = (i < 10) ? i+1 : 0;
                } else if (pressed == '<' || pressed == '*') {
                    i = (i > 0) ? i-1 : 10;
                } else if (pressed == '0') {
                    // Going to have to break out of here with flags or an interrupt
                    // Nope - can just return, main function now has a proper loop in it to restart from hibernation
                    return;
                } else if (pressed == '1') {
                    if (!savedToEEPROM) { // Save to EEPROM
                        // Get the date
                        unsigned char time[7];
                        RTC_getTime(time);
                        struct operationInfo run;
                        
                        run.year = __bcd_to_num(time[6]);
                        run.month = __bcd_to_num(time[5]);
                        run.day = __bcd_to_num(time[4]);
                        
                        run.minutes = operationMinutes;
                        run.seconds = operationSeconds;
                        
                        run.packagedB = dispensed.b;
                        run.packagedN = dispensed.n;
                        run.packagedS = dispensed.s;
                        run.packagedW = dispensed.w;
                        
                        run.remainingB = extras.b;
                        run.remainingN = extras.n;
                        run.remainingS = extras.s;
                        run.remainingW = extras.w;
                        
                        EEPROM_logOperation(&run);
                        printStringLCD("Saved to EEPROM");
                        __delay_ms(2000);
                        
                    } else {
                        printStringLCD("Already saved");
                        __delay_ms(2000);
                    }
                    
                }
                
                if (i == 0) {
                    printStringLCD(summaries[0]);
                } else if (i > 0 && i <= 8) { // Print packaging parameters
                    char msg[32];
                    sprintf(msg, "C%d: %s x%d", i, fSLookup[params.toFill[i-1]], params.setMultiple[i-1]);
                    printStringLCD(msg);
                } else if (i > 8) {
                    printStringLCD(summaries[i-8]);
                }
            }
            
            /* Print run parameters - old*/
            /*
            while(1) {
                // Loading parameters
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
                
                // Print operating time
                __lcd_clear();
                __lcd_home();
                printf("Time:");
                __lcd_newline();
                printf("%ld", operationTime);
                __delay_ms(2000);
                
            }*/
        } else if (pressed - 48 == 2) { // View logs
            viewLogs();
            needToPrint = 1;
        }
    }
}
