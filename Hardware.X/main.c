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
#include "timer.h"
#include "motors.h"

#define debouncingResolution 2

void packageCompartment(char b, char n, char s, char w) {
    TRISA = 0xFF;
    
    // Activate all motors
    motorControl(BOLT, FORWARD);
    motorControl(NUT, FORWARD);
    motorControl(SPACER, FORWARD);
    motorControl(WASHER, FORWARD);
    
    int numB=0, numN=0, numS=0, numW=0;
    int doneB=0, doneN=0, doneS=0, doneW=0;
    double currTime, timeB=0, timeN=0, timeS=0, timeW=0; // This was a long before?
    
    tic(); // Start the timer
    while (1) {
        currTime = tock();
        /*
        char currTimeString[33];
        sprintf(currTimeString, "%li", currTime);
        currTimeString[32] = "\0"; // Null-terminate
        __lcd_clear();
        __lcd_home();
        printf(currTimeString);
        __delay_ms(2000);
        I2C_Send(nanoAddr, currTimeString);*/
        
        // Bolts
        if (PORTAbits.RA0 == 0 && currTime > timeB + debouncingResolution) {
            timeB = currTime;
            numB++;
            dispensed.b++;
            I2C_Send(nanoAddr, "\1Bolt Counted\0");
        }
        
        // Nuts
        if (PORTAbits.RA1 == 0 && currTime > timeN + debouncingResolution) {
            timeN = currTime;
            numN++;
            dispensed.n++;
            I2C_Send(nanoAddr, "\1Nut Counted\0");
        }
        
        // Spacer
        if (PORTAbits.RA2 == 0 && currTime > timeS + debouncingResolution) {
            timeS = currTime;
            numS++;
            dispensed.s++;
            I2C_Send(nanoAddr, "\1Spacer Counted\0");
        }
        
        // Washer
        if (PORTAbits.RA3 == 0 && currTime > timeW + debouncingResolution) {
            timeW = currTime;
            numW++;
            dispensed.w++;
            I2C_Send(nanoAddr, "\1Washer Counted\0");
        }
        
        // Stop the motors
        if (numB >= b) {
            motorControl(BOLT, STOPMOTOR);
            doneB=1;
        }
        if (numN >= n) {
            motorControl(NUT, STOPMOTOR);
            doneN=1;
        }
        if (numS >= s) {
            motorControl(SPACER, STOPMOTOR);
            doneS=1;
        }
        if (numW >= w) {
            motorControl(WASHER, STOPMOTOR);
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
    
    dispensed.b = 0;
    dispensed.n = 0;
    dispensed.s = 0;
    dispensed.w = 0;
    
    I2C_Send(nanoAddr, "\1Entered the packaging function\0");
    char instr[2] = {8, 0};
    I2C_Send(nanoAddr, &instr); // Instruction to start box setting
    
    for (compartmentNum = 8; compartmentNum > 0; compartmentNum--) {
        
        char msg[] = "\1Started packaging compartment x\0";
        msg[31] = compartmentNum + 48;
        I2C_Send(nanoAddr, msg);
        // This is the line that breaks things
        if (params.toFill[compartmentNum-1] == 0) continue; // Skip compartments that are to be empty
        char * set = fastenerMatrix[params.toFill[compartmentNum-1]];
        char msg2[32];
        int mult = params.setMultiple[compartmentNum - 1];
        sprintf(msg2, "\1B:%d N:%d S:%d W:%d\0", set[0]*mult, set[1]*mult, set[2]*mult, set[3]*mult);
        I2C_Send(nanoAddr, msg2);
        __lcd_clear();
        __lcd_home();
        printf("Compartment %d", compartmentNum);
        packageCompartment(set[0]*mult, set[1]*mult, set[2]*mult, set[3]*mult);
    }
}

void clearing(void) {
    /* This function involves a lot of coordination between the PIC and 
     * Arduino. For now, we're keeping track of how long motor operations
     * on the Arduino take, and delaying for the appropriate time here.*/
    
    I2C_Send(nanoAddr, 3); // Rotate Nema 17 359.5 deg CW, step micro servo over first dispensing bin
    
    int i;
    double spinTime=20.0; // Change this in practice
    long startTime;
    double currTime, debounceTime=0;
    
    extras.b = 0;
    extras.n = 0;
    extras.s = 0;
    extras.w = 0;
    
    tic(); // Initialize the debouncing timer
    
    motorControl(BOLT, FORWARD);
    while (extras.b + dispensed.b < maxSuppliedB) { // Make sure this doesn't miss one
        currTime = tock();
        if (PORTAbits.RA0 == 0 && currTime > debounceTime + debouncingResolution) {
            debounceTime = currTime;
            extras.b++;
            I2C_Send(nanoAddr, "\1Bolt counted\0");
        }
        
        if (currTime >= spinTime) break; // We've waited long enough
    }
    motorControl(BOLT, STOPMOTOR);
    
    I2C_Send(nanoAddr, 4); // Step micro servo over second dispensing bin
    __delay_ms(5000); // These delays should probably be a wait until complete message from the Arduino
    
    tic();
    debounceTime = 0;
    motorControl(NUT, FORWARD);
    while (extras.n + dispensed.n < maxSuppliedN) {
        currTime = tock();
        if (PORTAbits.RA1 == 0 && currTime > debounceTime + debouncingResolution) {
            debounceTime = currTime;
            extras.n++;
            I2C_Send(nanoAddr, "\1Nut counted\0");
        }
        
        if (currTime >= spinTime) break;
    }
    
    motorControl(NUT, STOPMOTOR);
    
    I2C_Send(nanoAddr, 5); // Step micro servo over third dispensing bin
    __delay_ms(5000);
    
    tic();
    debounceTime = 0;
    motorControl(SPACER, FORWARD);
    while (extras.s + dispensed.s < maxSuppliedS) {
        currTime = tock();
        if (PORTAbits.RA2 == 0 && currTime > debounceTime + debouncingResolution) {
            debounceTime = currTime;
            extras.s++;
            I2C_Send(nanoAddr, "\1Spacer counted\0");
        }
        
        if (currTime >= spinTime) break;
    }

    motorControl(SPACER, STOPMOTOR);
    
    I2C_Send(nanoAddr, 6); // Step micro servo over fourth dispensing bin
    __delay_ms(5000);
    
    tic();
    debounceTime = 0;
    motorControl(WASHER, FORWARD);
    while (extras.w + dispensed.w < maxSuppliedW) {
        currTime = tock();
        if (PORTAbits.RA3 == 0 && currTime > debounceTime + debouncingResolution) {
            debounceTime = currTime;
            extras.w++;
            I2C_Send(nanoAddr, "\1Washer counted\0");
        }
        
        if (currTime >= spinTime) break;
    }
    
    motorControl(WASHER, STOP);
    
    I2C_Send(nanoAddr, 7); // Unset box
    // Retract 28BYJ stepper and DC motor holding box down
}

void interrupt interruptHandler(void) {
    if (T0IE && T0IF) {
        T0IF = 0;
        tmr0_ISR();
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
    TRISB = 0b11110111; // Inputs, necessary for keypad
    TRISC = 0x00; // Outputs initially. RC4 and RC5 used for I2C communication.
    TRISD = 0x00; // Outputs initially
    TRISE = 0x00; // Outputs initially
    
    ADCON0 = 0x00;  // Disable ADC
    ADCON1 = 0x0F;  // Digital pins
    // </editor-fold>
    
    I2C_Master_Init(100000); // Start I2C with a 100khz clock
    
    initLCD();
    
    __lcd_clear();
    __lcd_home();
    
    hibernate();
    mainMenu();
    //printf("\1Counting");
    //packageCompartment(4, 4, 4, 4);
}
