/*
;    Project:       Smart EVSE
;

 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UTILS_H
#define	UTILS_H

extern unsigned long pow_10[10];

uint32_t MacId();
unsigned char crc8(unsigned char *buf, unsigned char len);
unsigned int crc16(unsigned char *buf, unsigned char len);
void sprintfl(char *str, const char *Format, signed long Value, unsigned char Divisor, unsigned char Decimal);
unsigned char triwave8(unsigned char in);
unsigned char scale8(unsigned char i, unsigned char scale);
unsigned char ease8InOutQuad(unsigned char i);

#endif	/* UTILS_H */
