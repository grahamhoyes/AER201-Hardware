/* 
 * File:   menu.h
 * Author: Graham
 *
 * Created on February 6, 2018, 1:52 AM
 */

#ifndef MENU_H
#define	MENU_H

volatile unsigned char interruptKeypress = 0xFF;

void inputEntry(void);
void hibernate(void);
void mainMenu(void);
void viewLogs(void);

#endif	/* MENU_H */

