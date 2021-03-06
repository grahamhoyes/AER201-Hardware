/* 
 * File:   motors.h
 * Author: Graham
 *
 * Created on March 10, 2018, 5:41 PM
 */

#ifndef MOTORS_H
#define	MOTORS_H

typedef enum fastenerType {BOLT, NUT, SPACER, WASHER} fastenerType; // Really hope these names don't conflict with something
typedef enum motorDirection {FORWARD, REVERSE, STOPMOTOR} motorDirection;

struct currMotorDir {
    motorDirection b;
    motorDirection n;
    motorDirection s;
    motorDirection w;
} currentMotorDir = {STOPMOTOR, STOPMOTOR, STOPMOTOR, STOPMOTOR};

void motorControl(fastenerType motor, motorDirection dir);
motorDirection inverseDir(motorDirection dir);
#endif	/* MOTORS_H */

