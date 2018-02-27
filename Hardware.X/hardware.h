/* 
 * File:   hardware.h
 * Author: Graham
 *
 * Created on January 20, 2018, 8:47 PM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H
/* Fastener sets */
typedef enum fs {NONE=0, B=1, N=2, S=3, W=4, BN=5, BS=6, BW=7, BBN=8, BBS=9, BBW=10, BNW=11, 
                BSW=12, BWW=13, BNWW=14, BSWW=15, BBSW=16, BBNW=17, BNNW=18,
                BNNN=19, BWWW=20} fS;

char fSLookup[21][5] = {"NONE", "B", "N", "S", "W", "BN", "BS", "BW", "BBN", "BBS", "BBW", "BNW",
                "BSW", "BWW", "BNNW", "BSWW", "BBSW", "BBNW", "BNNW",
                "BNNN", "BWWW"};

struct inputParams{
    int steps; // Number of assembly steps
    fS toFill[8]; // Which fastener set goes in each compartment
    int setMultiple[8]; // How many of each set in each compartment
} params; 

struct amounts { // For storing counts of things
    int b;
    int n;
    int s;
    int w;
};
/*************************Public Function Declarations*************************/
//int printStringLCD(char *string);
int inputEntry(void);

#endif	/* HARDWARE_H */

