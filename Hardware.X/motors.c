#include <xc.h>
#include "hardware.h"
#include "helpers.h"
#include "motors.h"

void motorControl(fastenerType motor, motorDirection dir) {
    switch(motor) {
        case BOLT:
            if (dir == FORWARD) {
                LATBbits.LATB3 = 1;
                LATCbits.LATC0 = 0;
            } else if (dir == REVERSE) {
                LATBbits.LATB3 = 0;
                LATCbits.LATC0 = 1;
            } else if (dir == STOPMOTOR) {
                LATBbits.LATB3 = 1;
                LATCbits.LATC0 = 1;
            }
            break;
        case NUT:
            if (dir == FORWARD) {
                LATCbits.LATC1 = 1;
                LATCbits.LATC2 = 0;
            } else if (dir == REVERSE) {
                LATCbits.LATC1 = 0;
                LATCbits.LATC2 = 1;
            } else if (dir == STOPMOTOR) {
                LATCbits.LATC1 = 1;
                LATCbits.LATC2 = 1;
            }
            break;
        case SPACER:
            if (dir == FORWARD) {
                LATCbits.LATC5 = 1;
                LATCbits.LATC6 = 0;
            } else if (dir == REVERSE) {
                LATCbits.LATC5 = 0;
                LATCbits.LATC6 = 1;
            } else if (dir == STOPMOTOR) {
                LATCbits.LATC5 = 1;
                LATCbits.LATC6 = 1;
            }
            break;
        case WASHER:
            if (dir == FORWARD) {
                LATCbits.LATC7 = 1;
                LATEbits.LATE0 = 0;
            } else if (dir == REVERSE) {
                LATCbits.LATC7 = 0;
                LATEbits.LATE0 = 1;
            } else if (dir == STOPMOTOR) {
                LATCbits.LATC7 = 1;
                LATEbits.LATE0 = 1;
            }
            break;
        default:
            break;
    }
}