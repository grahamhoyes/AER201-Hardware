/* 
 * File:   newfile.h
 * Author: Graham
 *
 * Created on January 21, 2018, 11:14 PM
 */

#ifndef MENU_H
#define	MENU_H

typedef enum {B=66, N=78, S=83, W=87, steps, sets, x} fastener;

struct menuOption {
    char min;
    char max;
    char end;
    char name[5];
    fastener type; // Fastener currently being selected
    struct menuOption * next[4]; // Pointers to next menu screen, index by input
    struct menuOption * prev;  // Pointer to the previous menu
};

void initMenu(void);
#endif	/* NEWFILE_H */

