/*
 * File:   RTC.c
 * Author: Graham Hoyes
 *
 * Created on February 22, 2018, 6:18 PM
 */

#include "hardware.h"
#include "I2C.h"
#include "RTC.h"

void RTC_getTime(unsigned char * time) {
    /* Reset RTC memory pointer */
    I2C_Master_Start(); // Start
    I2C_Master_Write(0b11010000); // 7 bit RTC address + write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    I2C_Master_Stop(); // Stop
    
    /* Read current time */
    char i;
    
    I2C_Master_Start(); // Start
    I2C_Master_Write(0b11010001); // 7 bit RTC address + read
    
    for (i = 0; i < 6; i++) {
        time[i] = I2C_Master_Read(ACK); // Read with ACK (0) to continue reading
    }
    time[6] = I2C_Master_Read(NACK); // Read with NACK (1) to stop reading
    I2C_Master_Stop(); // Stop
}

long RTC_getSeconds(void) {
    /*  Returns: The number of seconds since the start of the day 
     */
    
    unsigned char time[7];
    RTC_getTime(time);
    
    long seconds = time[0] + 60*time[1] + 60*60*time[2] + 60*60*24*time[3];
    return seconds;
}

