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
int RTC_getOperatingTime(void);

/* Macros */
#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10

#endif	/* RTC_H */

