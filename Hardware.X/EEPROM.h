/* 
 * File:   EEPROM.h
 * Author: Graham
 *
 * Created on March 31, 2018, 2:31 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

struct operationInfo {
    unsigned char year;
    unsigned char day;
    unsigned char month;
    unsigned char minutes;
    unsigned char seconds;
    unsigned char packagedB;
    unsigned char packagedN;
    unsigned char packagedS;
    unsigned char packagedW;
    unsigned char remainingB;
    unsigned char remainingN;
    unsigned char remainingS;
    unsigned char remainingW;
};

void EEPROM_write(unsigned int address, unsigned char data);
unsigned char EEPROM_read(unsigned int address);
void EEPROM_logOperation(struct operationInfo *data);
int EEPROM_readLog(unsigned int logNum, struct operationInfo *data);
void EEPROM_clear(void);

#endif	/* EEPROM_H */

