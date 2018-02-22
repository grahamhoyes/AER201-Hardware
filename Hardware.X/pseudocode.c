#include <xc.h>
#include "hardware.h"

void config(void) {
    TRISA = 0b11111100;
    TRISB = 0b11101111;
    TRISC = 0b00000000;
    TRISD = 0b00000000;
    TRISE = 0b00000000;
}
int pollMicroswitch(void) {
        // Wait until the microswitch on RA4 is pressed
        while (PORTAbits.RA4 == 0) {continue;} 
        
        __delay_ms(100);
        
        // Wait until switch has been released
        while (PORTAbits.RA4 == 1) {continue;}
        
        Nop();  // Apply breakpoint here to prevent compiler optimizations
        
        return 1;
}

void boxSetting(void) { // This is all on Arduino
    // Send Arduino "Rotate Nema 17 CCW" instrcution (probably on RE2)
    pollMicroswitch(); // Waiting for a click (and release?)
    // Send Arduino "Stop Nema 17" instruction
    // Step 28BYJ motor to lock box in place
    // Step Nema 17 45deg CW
    // Rotate Nema 17 CW
    pollReflectiveIR();
    // Stop Nema 17
}

void packaging(int b, int n, int s, int w) {
    // Activate all motors
    LATBBits.LATB3 = 1;
    LATCBits.LATC1 = 1;
    LATCBits.LATC5 = 1;
    LATCBits.LATC7 = 1;
    
    int numB=0, numN=0, numS=0, numW=0, done=0;
    while (1) {
        if (PORTAbits.RA0 == 1) numB++;
        if (PORTAbits.RA1 == 1) numN++;
        if (PORTAbits.RA2 == 1) numS++;
        if (PORTAbits.RA3 == 1) numW++;
        
        if (numB >= b) {
            LATBBits.LATB3 = 0; 
            done++;
        }
        if (numN >= n) {
            LATCBits.LATC5 = 0;
            done++;
        }
        if (numS >= s) {
            LATCBits.LATC5 = 0;
            done++;
        }
        if (numW >= w) {
            LATCBits.LATC7 = 0;
            done++;
        }
        
        if (done >= 3) break;
    }
    // Rotate Nema 17 45deg CW
}

void clearing(void) {
    // Rotate Nema 17 359.5deg CW
    // Step microservo over first dispensing bin
    int i;
    int spinTime;
    int numB=0, numN=0, numS=0, numW=0;
    
    LATBbits.LATB3 = 1;
    for (i = 0; i < spinTime; i++) {
        // Each millisecond, check if a bolt is passed and count it
        if (PORTAbits.RA0 == 0) {
            numB++;
        }
        // if max number of bolts: break;
        __delay_ms(1);
    }
    LATBbits.LATB3 = 0;
    
    // Rotate Servo to second dispensing bin
    // repeat counting of remaining nuts
    // Repeat for spacers & washers.
    // Retract 28BYJ stepper and DC motor holding box down
}

void autonomous(void) {

    boxSetting();
    for compartments {
        packaging(/*The right number of stuff*/)
    }
    clearing();
    displayStatistics(); // Currently in mainMenu();
        
}
