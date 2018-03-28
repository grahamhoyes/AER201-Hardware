/* 
 * File:   hardware.h
 * Author: Graham
 *
 * Created on January 20, 2018, 8:47 PM
 */

#ifndef HARDWARE_H
#define	HARDWARE_H
/* Operating modes 
 * These weren't implemented early enough to be used everywhere, but they are useful in interrupt handlers
 */
typedef enum operatingMode {MENU, LOGS, PACKAGING, CLEARING, WAITING} operatingMode;
int currentMode = MENU;

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
} extras, dispensed;

// Total supplied maximums
#define maxSuppliedB 20
#define maxSuppliedN 25
#define maxSuppliedS 20
#define maxSuppliedW 35

// Compartmental maximums
#define maxCompB 2
#define maxCompN 3
#define maxCompS 2
#define maxCompW 4

/* Global constant declarations */
const char inputEntryQuestions[4][33] = { // Putting in an extra bit for the null terminator need to adjust code elsewhere
    "Assembly steps\n"        "*<-  (4-8)     \0",
    "Fasteners in Cx\n"       "*<-(BNSW)    ->#\0",
    "How many sets?\n"        "*<-  (1-4)     \0",
    "Confirm?\n"              "*<-  (A:Y/B:N) \0"
};

const char LCDMenuIcons[16] = "*<-   ( - )    \0";

const struct errorMessages {
    const char badEntry[32];
    const char tooManyFasteners[32];
    const char tooManyBolts[32];
    const char tooManyNuts[32];
    const char tooManyWashers[32];
    const char tooManySpacers[32];
    const char noFasteners[32];
} errMsgs = {
    "Invalid entry\nPlease try again\0",
    "Too many fasteners\0",
    "Too many bolts\0",
    "Too many nuts\0",
    "Too many washers\0",
    "Too many spacers\0",
    "No fasteners selected\0"
}; 

const char fastenerMatrix[21][4] = {
    {0,0,0,0}, // The default for an empty cell
    {1,0,0,0},
    {0,1,0,0},
    {0,0,1,0},
    {0,0,0,1},
    {1,1,0,0},
    {1,0,1,0},
    {1,0,0,1},
    {2,1,0,0},
    {2,0,1,0},
    {2,0,0,1},
    {1,1,0,1},
    {1,0,1,1},
    {1,0,0,2},
    {1,1,0,2},
    {1,0,1,2},
    {2,0,1,1},
    {2,1,0,1},
    {1,2,0,1},
    {1,3,0,0},
    {1,0,0,3}
};

/* Encoding of which compartments to fill in */
const char assemblyStepEncoding[5] = {
    /* Note that array indices are 5 below, compartment indices are 1 below*/    
    /* Because of the weird way I do things, these are backwards*/
    0b01010101, // 4 steps: C 7,5,3,1
    0b01011011, // 5 steps: C 7,5,4,2,1
    0b01110111, // 6 steps: C 7,6,5,3,2,1
    0b01111111, // 7 steps: C 7,6,5,4,3,2,1
    0b11111111  // 8 steps: C 8,7,6,5,4,3,2,1
};

/*************************Public Function Declarations*************************/
void packageCompartment(char b, char n, char s, char w);
void packaging(void);
void clearing(void);

#endif	/* HARDWARE_H */

