#include <xc.h>
#include "EEPROM.h"
#include "hardware.h"

void initEEPROM(void) {
    // Set all completion flag bits to 0
}

void EEPROM_write(unsigned int address, unsigned char data) {
    EECON1bits.WREN = 1;    // Allow write cycles to EEPROM / Flash
    
    // Set address registers
    EEADRH = (unsigned char)(address >> 8);     // Take the most significant 8 bits (6 MSB are empty)
    EEADR = (unsigned char)address;
    
    EEDATA = data;
    EECON1bits.EEPGD = 0;   // Access EEPROM instead of flash memory
    EECON1bits.CFGS = 0;    // Access EEPROM/Flash, not config registers
    
    di();                   // Disable interrupts during initial sequence
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;      // Write control bit: initiate EEPROM write cycle
    ei();
    
    // Poll for EEPROM completion, on the EEIF interrupt flag
    while (PIR2bits.EEIF == 0) {continue;}
    PIR2bits.EEIF = 0;      // Clear the interrupt flag
    EECON1bits.WREN = 0;    // Disallow write cycles to EEPROM / Flash   
}

unsigned char EEPROM_read(unsigned int address) {
    // Set address registers
    EEADRH = (unsigned char)(address >> 8);
    EEADR = (unsigned char)address;
    
    EECON1bits.EEPGD = 0;    // Access EEPROM instead of flash memory
    EECON1bits.RD = 1;       // Read control bit: initiate EEPROM read cycle
    
    // Read control bit cleared by hardware after one cycle
    while (EECON1bits.RD == 1) {continue;}
    return EEDATA;
}

void EEPROM_logOperation(struct operationInfo *data) { // This might be an issue if the variables in data are longer than they are supposed to be... should probably & with stuff to make sure
    // Perform magic to get everything to fit in 8 bytes
    unsigned char line0 = (data->year << 3) | ((data->day & 0b00011111) >> 2);
    unsigned char line1 = (data->day << 6) | ((data->month & 0b00001111) << 2) | ((data->packagedN & 0b00011111) >> 3);
    unsigned char line2 = (data->packagedN << 5) | (data->packagedW & 0b00011111);
    unsigned char line3 = (data->packagedB << 4) | (data->packagedS & 0b00001111);
    unsigned char line4 = (data->remainingB << 4) | ((data->remainingN &0b00011111) >> 2);
    unsigned char line5 = (data->remainingN << 6) | ((data->remainingS & 0b00011111) << 1) | ((data->remainingW & 0b00011111) >> 5);
    unsigned char line6 = (data->remainingW << 3) | (data->minutes & 0b00000111);
    unsigned char line7 = (data->seconds << 2) | 0b1;
    
    // Figure out what the next unused block of EEPROM is
    unsigned int address;
    for (address = 0; address < 1016; address+=8) {
        if (EEPROM_read(address+7) & 0b00000001 == 0) break; // Found an address without completion flag for entry
    }
    
    // Write the data to be logged, line by line
    EEPROM_write(address, line0);
    EEPROM_write(address+1, line1);
    EEPROM_write(address+2, line2);
    EEPROM_write(address+3, line3);
    EEPROM_write(address+4, line4);
    EEPROM_write(address+5, line5);
    EEPROM_write(address+6, line6);
    EEPROM_write(address+7, line7);
}

int EEPROM_readLog(unsigned int logNum, struct operationInfo *data) {
    unsigned int address = logNum*8;
    
    unsigned char line0 = EEPROM_read(address);
    unsigned char line1 = EEPROM_read(address+1);
    unsigned char line2 = EEPROM_read(address+2);
    unsigned char line3 = EEPROM_read(address+3);
    unsigned char line4 = EEPROM_read(address+4);
    unsigned char line5 = EEPROM_read(address+5);
    unsigned char line6 = EEPROM_read(address+6);
    unsigned char line7 = EEPROM_read(address+7);
    
    if ((line7 & 0b1) == 0) {
        return 0; // Entry is empty
    }
    
    // Extract all the data
    data->year = (unsigned char)(line0 >> 3);
    data->day = (unsigned char)(((line0 & 0b00000111) << 2) | (line1 >> 6));
    data->month = (unsigned char)((line1 >> 2) & 0b00001111);
    data->packagedN = (unsigned char)(((line1 & 0b00000011) << 3) | (line2 >> 5));
    data->packagedW = (unsigned char)(line2 & 0b00011111);
    data->packagedB = (unsigned char)(line3 >> 4);
    data->packagedS = (unsigned char)(line3 & 0b00001111);
    data->remainingB = (unsigned char)(line4 >> 3);
    data->remainingN = (unsigned char)(((line4 & 0b00000111) << 2) | (line5 >> 6));
    data->remainingS = (unsigned char)((line5 >> 1) & 0b00011111);
    data->remainingW = (unsigned char)(((line5 & 0b00000001) << 5) | (line6 >> 3)); // double check this one
    data->minutes = (unsigned char)(line6 & 0b00000111);
    data->seconds = (unsigned char)(line7 >> 2);
    
    return 1;
}

void EEPROM_clear(void) {
    unsigned int addr;
    for (addr = 0; addr < 1024; addr++) {
        EEPROM_write(addr, 0b00000000);
    }
}