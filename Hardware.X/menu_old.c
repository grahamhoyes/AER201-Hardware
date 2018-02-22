#include "menu.h"
/*
typedef enum {B=66, N=78, S=83, W=87, steps, sets, x} fastener;

typedef struct {
    char min;
    char max;
    char end;
    char name[5];
    fastener type; // Fastener currently being selected
    menuOption * next[4]; // Pointers to next menu screen, index by input
    menuOption * prev;  // Pointer to the previous menu
} menuOption;*/

/* Start assembling the tree */
/* Final states */ 
//struct menuOption mBBNW = {0, 0, 1, "BBNW\0"};
//struct menuOption mBBN = {0, 0, 1, "BBN\0"};
//struct menuOption mBBSW = {0, 0, 1, "BBSW\0"};
//struct menuOption mBBS = {0, 0, 1, "BBS\0"};
//struct menuOption mBBW = {0, 0, 1, "BBW\0"};
//struct menuOption mBNNN = {0, 0, 1, "BNNN\0"};
//struct menuOption mBNNW = {0, 0, 1, "BNNW\0"};
//struct menuOption mBNWW = {0, 0, 1, "BNWW\0"};
//struct menuOption mBNW = {0, 0, 1, "BNW\0"};
//struct menuOption mBN = {0, 0, 1, "BN\0"};
//struct menuOption mBSWW = {0, 0, 1, "BSWW\0"};
//struct menuOption mBSW = {0, 0, 1, "BSW\0"};
//struct menuOption mBS = {0, 0, 1, "BS\0"};
//struct menuOption mBWWW = {0, 0, 1, "BWWW\0"};
//struct menuOption mBWW = {0, 0, 1, "BWW\0"};
//struct menuOption mBW = {0, 0, 1, "BW\0"};
//struct menuOption mB = {0, 0, 1, "B\0"};
//struct menuOption mW = {0, 0, 1, "W\0"};
//struct menuOption mS = {0, 0, 1, "S\0"};
//struct menuOption mN = {0, 0, 1, "N\0"};
//
///* Intermediate states */
//struct menuOption mOne = {0, 1, 0, "\0", W, {&mBBN, &mBBNW}};
//struct menuOption mTwo = {0, 1, 0, "\0", W, {&mBBS, &mBBSW}};
//// mThree goes straight to BBW
//struct menuOption mFour = {0, 1, 0, "\0", S, {&mBBW, &mTwo}};
//// mFive goes straight to BNNN
//// mSix goes straight to BNNW
//struct menuOption mSeven = {0, 2, 0, "\0", W, {&mBN, &mBNW, &mBNWW}};
//struct menuOption mEight = {0, 2, 0, "\0", W, {&mBS, &mBSW, &mBSWW}};
//struct menuOption mNine = {0, 3, 0, "\0", W, {&mB, &mBW, &mBWW, &mBWWW}};
//struct menuOption mTen = {0, 1, 0, "\0", S, {&mNine, &mEight}};
//// mEleven goes straight to W
//struct menuOption mTwelve = {0, 1, 0, "\0", S, {&mW, &mS}};
//struct menuOption mThirteen = {0, 1, 0, "\0", N, {&mFour, &mOne}};
//struct menuOption mFourteen = {0, 3, 0, "\0", N, {&mTen, &mSeven, &mBNNW, &mBNNN}};
//struct menuOption mFifteen = {0, 1, 0, "\0", N, {&mTwelve, &mN}};
//struct menuOption mStart = {0, 2, 0, "\0", B, {&mFifteen, &mFourteen, &mThirteen}};
//
///* The beginning options */
//struct menuOption mSets = {1, 4, 0, "\0", sets, {&mStart, &mStart, &mStart, &mStart}};
//struct menuOption mSteps = {4, 8, 0, "\0", steps, {&mSets, &mSets, &mSets, &mSets}};

/* Now that everything has been declared, backwards link the tree together */
void initMenu(void) {
    mBBNW.prev = &mOne;
    mBBN.prev = &mOne;
    mBBSW.prev = &mTwo;
    mBBS.prev = &mTwo;
    mBBW.prev = &mFour;
    mBNNN.prev = &mFourteen;
    mBNNW.prev = &mFourteen;
    mBNWW.prev = &mSeven;
    mBNW.prev = &mSeven;
    mBN.prev = &mSeven;
    mBSWW.prev = &mEight;
    mBSW.prev = &mEight;
    mBS.prev = &mEight;
    mBWWW.prev = &mFour;
    mBWW.prev = &mFour;
    mBW.prev = &mFour;
    mB.prev = &mFour;
    mW.prev = &mTwelve;
    mS.prev = &mTwelve;
    mN.prev = &mFifteen;
    
    /* Intermediate states */
    mOne.prev = &mThirteen;
    mTwo.prev = &mFour;
    mFour.prev = &mThirteen;
    mSeven.prev = &mFourteen;
    mEight.prev = &mTen;
    mNine.prev = &mTen;
    mTen.prev = &mFourteen;
    mTwelve.prev = &mFifteen;
    mThirteen.prev = &mStart;
    mFourteen.prev = &mStart;
    mFifteen.prev = &mStart;
    mStart.prev = &mSets;
    
    /* In the beginning... */
    mSteps.prev = &mSteps;
    mSets.prev = &mSteps;
}