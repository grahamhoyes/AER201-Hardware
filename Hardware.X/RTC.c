/*
 * File:   RTC.c
 * Author: Graham Hoyes
 *
 * Created on February 22, 2018, 6:18 PM
 */

#include "hardware.h"
#include "I2C.h"
#include "RTC.h"

typedef struct time {
    int h;
    int m;
    int s;
} Time;

static long operatingTime;
static Time startTime;
static Time endTime;

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
    
    char time[7];
    RTC_getTime(time);
    
    long seconds = __bcd_to_num(time[0]) + 60*__bcd_to_num(time[1]) + 60*60*__bcd_to_num(time[2]);
    //long seconds = __bcd_to_num(time[0]) + 60*__bcd_to_num(time[1]) + 60*60*__bcd_to_num(time[2]) + 60*60*24*__bcd_to_num(time[3]);
    //long seconds = __bcd_to_num(time[0]);
    return seconds;
}

void RTC_startOperation(void) {
    /* Starts timing the operation */
    char time[7];
    RTC_getTime(time);
    startTime.h = __bcd_to_num(time[2]);
    startTime.m = __bcd_to_num(time[1]);
    startTime.s = __bcd_to_num(time[0]);
}

int RTC_getOperatingTime(void) {
    /* Returns: the number of seconds the operation took*/
    char time[7];
    RTC_getTime(time);
    endTime.h = __bcd_to_num(time[2]);
    endTime.m = __bcd_to_num(time[1]);
    endTime.s = __bcd_to_num(time[0]);
    
    Time operatingTimeLocal;
    operatingTimeLocal.h = endTime.h - startTime.h;
    operatingTimeLocal.m = endTime.m - startTime.m;
    operatingTimeLocal.s = endTime.s - startTime.s;
    
    int res = 60*60*operatingTimeLocal.h + 60*operatingTimeLocal.m + operatingTimeLocal.s;
    return res;
}

