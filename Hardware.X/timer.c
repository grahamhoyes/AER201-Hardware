/* Timer.c
 * Contains functions for interfacing with the TMR0 module
 * By: Graham Hoyes
 * Date: March 6, 2018
 */

#include <xc.h>
#include "configBits.h"
#include "hardware.h"
#include "timer.h"

static float timeSinceLastTic; // Number of seconds since the last call to tic() was made
void tmr0Init(void) {
    /* Starts the timer module with an interrupt period of 0.1 seconds
     * Arguments: none
     * Returns: none
     * On the PIC18F4620, we should be using the 1 MHz internal clock. However,
     * nothing makes sense and it looks like it's using an 8.6 MHz clock so
     * we're rolling with that.
     */
    
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
void tic(void) {
    /* Resets the timeSinceLastTic counter 
     * Arguments: none
     * Returns: none
     */
    timeSinceLastTic = 0;
}

float tock(void) {
    /* Returns: timeSinceLastTic to any scope
     * Argument: none
     */
    
    return timeSinceLastTic;
}

void tmr0_ISR(void) {
    timeSinceLastTic += 0.1;
    TMR0H = 0xF2;
    TMR0L = 0xC0;
}
