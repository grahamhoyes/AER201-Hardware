/* 
 * File:   hardware.h
 * Author: Graham
 *
 * Created on January 20, 2018, 8:47 PM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H
/* Fastener sets */
typedef enum fs {B=0, N=1, S=2, W=3, BN=4, BS=5, BW=6, BBN=7, BBS=8, BBW=9, BNW=10, 
                BSW=11, BWW=12, BNWW=13, BSWW=14, BBSW=15, BBNW=16, BNNW=17,
                BNNN=18, BWWW=19} fS;

/*************************Public Function Declarations*************************/
//int printStringLCD(char *string);
int inputEntry(void);

#endif	/* HARDWARE_H */

