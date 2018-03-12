/* Timer.c
 * Contains functions for interfacing with the TMR0 module
 * By: Graham Hoyes
 * Date: March 6, 2018
 */

#include <xc.h>
#include "configBits.h"
#include "hardware.h"
#include "timer.h"

static unsigned char operating=0; // Is the machine in operating mode? Should probably make this a global state variable
static unsigned char timerInit = 0; // Has the timer module been initialized?
static volatile double operatingTime=-1; // Number of seconds since the operation began. Volatile because modified in ISR
static volatile double timeSinceLastTic=-1; // Number of seconds since the last call to tic() was made. Volatile because modified in ISR
static volatile long test = 0;

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
        TMR0H = 0xF2;
        TMR0L = 0xC0;
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
    tmr0Init();
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

void tmr0_ISR(void) {
    timeSinceLastTic += 0.1;
    test++;
    if (operating) operatingTime += 0.1;
    TMR0H = 0xF2;
    TMR0L = 0xC0;
}
