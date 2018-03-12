/* 
 * File:   RTC.h
 * Author: Graham
 *
 * Created on February 22, 2018, 6:31 PM
 */

#ifndef RTC_H
#define	RTC_H

void RTC_getTime(unsigned char * time);
long RTC_getSeconds(void);
void RTC_startOperation(void);
long RTC_getOperatingTime(void);

#endif	/* RTC_H */

