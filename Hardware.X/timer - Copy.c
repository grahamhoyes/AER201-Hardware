/* Timer.c
 * Contains functions for interfacing with the TMR0 module
 * By: Graham Hoyes
 * Date: March 6, 2018
 */

#include <xc.h>
#include "configBits.h"
#include "hardware.h"
#include "timer.h"
#include "motors.h"
#include "I2C.h"
#include "stdio.h"
#include "helpers.h"

static long longTolerance = 60;
static long shortTolerance = 12;

static unsigned char operating=0; // Is the machine in operating mode? Should probably make this a global state variable
static unsigned char timerInit = 0; // Has the timer module been initialized?
static volatile double operatingTime=-1; // Number of seconds since the operation began. Volatile because modified in ISR
static volatile double timeSinceLastTic=-1; // Number of seconds since the last call to tic() was made. Volatile because modified in ISR
static volatile long test = 0;
static volatile long motorControlTimer = 0;
static volatile struct MGB {
    char b;
    char n;
    char s;
    char w;
} motorGoingBackwards = {0, 0, 0, 0};

void tmr0Init(void) {
    /* Starts the timer module with an interrupt period of 0.1 seconds
     * Arguments: none
     * Returns: none
     * On the PIC18F4620, we should be using the 1 MHz internal clock. However,
     * nothing makes sense and it looks like it's using an 8.6 MHz clock so
     * we're rolling with that.
     */
    
    if (!timerInit) {
        T0CONbits.T08BIT = 0;
        T0CONbits.T0CS = 0;
        T0CONbits.PSA = 0;

        // Prescale of 1:256
        T0CONbits.T0PS2 = 1;
        T0CONbits.T0PS1 = 1;
        T0CONbits.T0PS0 = 1;

        // The following for an interrupt ever 0.1s
        TMR0H = 0xF9;
        TMR0L = 0x5F;
        T0CONbits.TMR0ON = 1;
        TMR0IE = 1;

        // Enable all interrupts
        ei();
    }
}
void tic(void) {
    /* Resets the timeSinceLastTic counter 
     * Arguments: none
     * Returns: none
     */
    timeSinceLastTic = 0;
}

double tock(void) {
    /* Returns: timeSinceLastTic to any scope
     * Argument: none
     */
    
    return timeSinceLastTic;
}

void Timer_startOperation(void) {
    /* Starts the timer for timing the machine operation
     * Returns: none
     * Arguments: none
     */
    tmr0Init();
    operatingTime = 0;
    operating = 1;
}

double Timer_getOperatingTime(void) {
    /* Arguments: none
     * Returns: The number of seconds since the operation began
     */
    
    return operatingTime;
}

long getTest(void) {
    return test;
}

void resetMotorTimer(void) {
    motorControlTimer = 0;
}

void tmr0_ISR(void) {
    timeSinceLastTic += 0.1;
    test++;
    if (operating) operatingTime += 0.1;
    TMR0H = 0xF9;
    TMR0L = 0x5F;
    motorControlTimer++;
    /* Motor direction changing */
    switch (currentMode) {
        case PACKAGING:
            if (currentMotorDir.b != STOPMOTOR) {
                if (!motorGoingBackwards.b) {
                    if (motorControlTimer == longTolerance) {
                        motorControl(BOLT, inverseDir(currentMotorDir.b));
                        motorGoingBackwards.b = 1;
                        motorControlTimer = 0;
                    }
                } else {
                    if (motorControlTimer == shortTolerance) {
                        motorControl(BOLT, inverseDir(currentMotorDir.b));
                        motorGoingBackwards.b = 0;
                        motorControlTimer = 0;
                    }
                }
            }
            
            if (currentMotorDir.n != STOPMOTOR) {
                if (!motorGoingBackwards.n) {
                    if (motorControlTimer == longTolerance) {
                        motorControl(NUT, inverseDir(currentMotorDir.n));
                        motorGoingBackwards.n = 1;
                        motorControlTimer = 0;
                    }
                } else {
                    if (motorControlTimer == shortTolerance) {
                        motorControl(NUT, inverseDir(currentMotorDir.b));
                        motorGoingBackwards.n = 0;
                        motorControlTimer = 0;
                    }
                }
            }

            if (currentMotorDir.s != STOPMOTOR) {
                if (!motorGoingBackwards.s) {
                    if (motorControlTimer == longTolerance) {
                        motorControl(SPACER, inverseDir(currentMotorDir.s));
                        motorGoingBackwards.s = 1;
                        motorControlTimer = 0;
                    }
                } else {
                    if (motorControlTimer == shortTolerance) {
                        motorControl(SPACER, inverseDir(currentMotorDir.s));
                        motorGoingBackwards.s = 0;
                        motorControlTimer = 0;
                    }
                }
            }

            if (currentMotorDir.w != STOPMOTOR) {
                if (!motorGoingBackwards.w) {
                    if (motorControlTimer == longTolerance) {
                        motorControl(WASHER, inverseDir(currentMotorDir.w));
                        motorGoingBackwards.w = 1;
                        motorControlTimer = 0;
                    }
                } else {
                    if (motorControlTimer == shortTolerance) {
                        motorControl(WASHER, inverseDir(currentMotorDir.w));
                        motorGoingBackwards.w = 0;
                        motorControlTimer = 0;
                    }
                }
            }
            break;
        case CLEARING:
            break;
        default:
            resetMotorTimer();
            break;
    }
}
