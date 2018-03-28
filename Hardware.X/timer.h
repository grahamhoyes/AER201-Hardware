/* 
 * File:   timer.h
 * Author: Graham
 *
 * Created on March 6, 2018, 1:49 PM
 */

#ifndef TIMER_H
#define	TIMER_H

void tmr0Init(void);
void tic(void);
double tock(void);
void tmr0_ISR(void);
void Timer_startOperation(void);
double Timer_getOperatingTime(void);
long getTest(void);
void resetMotorTimer(void);

#endif	/* TIMER_H */

