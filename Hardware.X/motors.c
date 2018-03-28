#include <xc.h>
#include "hardware.h"
#include "helpers.h"
#include "motors.h"
#include "timer.h"

void motorControl(fastenerType motor, motorDirection dir) {
    switch(motor) {
        case SPACER:
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
            currentMotorDir.s = dir;
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
            currentMotorDir.n = dir;
            break;
        case WASHER:
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
            currentMotorDir.w = dir;
            break;
        case BOLT:
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
            currentMotorDir.b = dir;
            break;
        default:
            break;
    }
}

motorDirection inverseDir(motorDirection dir) {
    return (dir == FORWARD) ? REVERSE : FORWARD;
}
