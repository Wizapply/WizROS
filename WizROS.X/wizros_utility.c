/**************************************************************************
 *
 *              Copyright (c) Wizapply.
 *              The license on GitHub : GPL-3.0 and Dual-License
 *
 *  This software is copyrighted by and is the sole property of Wizapply
 *  All rights, title, ownership, or other interests in the software
 *  remain the property of Wizapply.  This software may only be used
 *  in accordance with the corresponding license agreement.
 *  Any unauthorized use, duplication, transmission, distribution,
 *  or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice may not be removed or modified without prior
 *  written consent of Wizapply.
 *
 *  Wizpply reserves the right to modify this software without notice.
 *
 *  Wizapply                                info@wizapply.com
 *  5F, KS Building,                        http://wizapply.com
 *  3-7-10 Ichiokamotomachi, Minato-ku,
 *  Osaka, 552-0002, Japan
 *
***************************************************************************/

/**************************************************************************
 *
 *  WIZROS for PIC18F25K80
 *
 *  Language is 'C' code source
 *  Compiler Tools : XC8 v1.37 PRO
 *  MClocks : 64 MHz (16 DMIPS)
 *
 *  File Name : wizros_utility.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_utility.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
uint8_t wzStringToUint8(const char* str, int8_t len)
{
    uint8_t res = 0;
    int8_t i, j;
    
    j = 0;
    for(i = len-1; i >= 0; i--) {
        if(str[i] >= '0' && str[i] <= '9') {  //0-9
            res |= (str[i] - '0') << j;
        }
        if(str[i] >= 'A' && str[i] <= 'F') {  //A-F 10-15
            res |= (str[i] - 'A' + 10) << j;
        }
        j += 4; //4bit
    }
    
    return res;
}

uint16_t wzStringBEToUint16(const char* str, int8_t len)
{
    uint16_t res = 0;
    
    if(len <= 2) {
        res |= (uint16_t)wzStringToUint8(&str[0], len);
    } else {
        res |= (uint16_t)wzStringToUint8(&str[0], 2) << 8;
        res |= (uint16_t)wzStringToUint8(&str[2], len-2);
    }
    
    return res;
}

uint16_t wzStringLEToUint16(const char* str, int8_t len)
{
    uint16_t res = 0;
    
    if(len <= 2) {
        res |= (uint16_t)wzStringToUint8(&str[0], len);
    } else {
        res |= (uint16_t)wzStringToUint8(&str[0], 2);
        res |= (uint16_t)wzStringToUint8(&str[2], len-2) << 8;
    }
    
    return res;
}

void wzUint8ToString(uint8_t num, char* ref_str3)
{
    if((num & 0x0F) >= 10)
         ref_str3[1] = (num & 0x0F) - 10 + 'A';
    else
         ref_str3[1] = (num & 0x0F) + '0';
    
    num = num >> 4;
    
    if((num & 0x0F) >= 10)
         ref_str3[0] |= ((num & 0x0F) - 10 + 'A');
    else
         ref_str3[0] |= ((num & 0x0F) + '0');
    
    ref_str3[2] = '\0';
}

void wzUint16ToStringBE(uint16_t num, char* ref_str5)
{
    uint8_t numh = num >> 8;
    uint8_t numl = num & 0x00FF;
    wzUint8ToString((uint8_t)numl,&ref_str5[0]);
    wzUint8ToString((uint8_t)numh,&ref_str5[2]);
    
    ref_str5[5] = '\0';
}

void wzUint16ToStringLE(uint16_t num, char* ref_str5)
{
    uint8_t numh = num >> 8;
    uint8_t numl = num & 0x00FF;
    wzUint8ToString((uint8_t)numh,&ref_str5[0]);
    wzUint8ToString((uint8_t)numl,&ref_str5[2]);
    
    ref_str5[5] = '\0';
}

int8_t wzStringToInt8(const char* str, int8_t len)
{
    union {
        int8_t i;
        uint8_t u;
    } res;
    int8_t i, j;
    
    res.i = 0;
    
    j = 0;
    for(i = len-1; i >= 0; i--) {
        if(str[i] >= '0' && str[i] <= '9') {  //0-9
            res.u |= (str[i] - '0') << j;
        }
        if(str[i] >= 'A' && str[i] <= 'F') {  //A-F 10-15
            res.u |= (str[i] - 'A' + 10) << j;
        }
        j += 4; //4bit
    }
    
    return res.i;
}

int16_t wzStringBEToInt16(const char* str, int8_t len)
{
    union {
        int16_t i;
        uint16_t u;
    } res;
    res.i = 0;
    
    if(len <= 2) {
        res.u |= (uint16_t)wzStringToUint8(&str[0], len);
    } else {
        res.u |= (uint16_t)wzStringToUint8(&str[0], 2) << 8;
        res.u |= (uint16_t)wzStringToUint8(&str[2], len-2);
    }
    
    return res.i;
}

int16_t wzStringLEToInt16(const char* str, int8_t len)
{
    union {
        int16_t i;
        uint16_t u;
    } res;
    res.i = 0;
    
    if(len <= 2) {
        res.u |= (uint16_t)wzStringToUint8(&str[0], len);
    } else {
        res.u |= (uint16_t)wzStringToUint8(&str[0], 2);
        res.u |= (uint16_t)wzStringToUint8(&str[2], len-2) << 8;
    }
    
    return res.i;
}

void wzInt8ToString(int8_t numi, char* ref_str3)
{
    union {
        int8_t i;
        uint8_t u;
    } num;
    num.i = numi;
    
    if((num.u & 0x0F) >= 10)
         ref_str3[1] = (num.u & 0x0F) - 10 + 'A';
    else
         ref_str3[1] = (num.u & 0x0F) + '0';
    
    num.u = num.u >> 4;
    
    if((num.u & 0x0F) >= 10)
         ref_str3[0] |= ((num.u & 0x0F) - 10 + 'A');
    else
         ref_str3[0] |= ((num.u & 0x0F) + '0');
    
    ref_str3[2] = '\0';
}

void wzInt16ToStringBE(int16_t numi, char* ref_str5)
{
    union {
        int16_t i;
        uint16_t u;
    } num;
    
    num.i = numi;
    
    uint8_t numh = num.u >> 8;
    uint8_t numl = num.u & 0x00FF;
    wzUint8ToString((uint8_t)numl,&ref_str5[0]);
    wzUint8ToString((uint8_t)numh,&ref_str5[2]);
    
    ref_str5[5] = '\0';
}

void wzInt16ToStringLE(int16_t numi, char* ref_str5)
{
    union {
        int16_t i;
        uint16_t u;
    } num;
    
    num.i = numi;
    
    uint8_t numh = num.u >> 8;
    uint8_t numl = num.u & 0x00FF;
    wzUint8ToString((uint8_t)numh,&ref_str5[0]);
    wzUint8ToString((uint8_t)numl,&ref_str5[2]);
    
    ref_str5[5] = '\0';
}

uint8_t wzCheckDigitByte(char* str, uint8_t strlen)
{
    uint8_t i;
    int16_t d = 0;
    
    for(i=0; i < strlen; i+=2) {
        d += str[i];
    }
    
    d = ~d + 1; //Not minus
    d = d & 0x00FF;
    
    return (uint8_t)d;
}

uint8_t wzCheckDigitByteString(char* str, uint8_t strlen)
{
    uint8_t i;
    int16_t d = 0;
    
    for(i=0; i < strlen; i+=2) {
        d += wzStringToUint8(&str[i],2);
    }
    
    d = ~d + 1; //Not minus
    d = d & 0x00FF;
    
    return (uint8_t)d;
}

void wzClearString(char* str, int8_t len)
{
    int8_t i;
    for(i = 0; i < len; i++) {
        str[i] = '\0';
    }
}

void wzMemSet(char* str, uint8_t set, int8_t len)
{
    int8_t i;
    for(i = 0; i < len; i++) {
        str[i] = set;
    }
}

uint16_t wzStringLength(char* str)
{
    uint16_t num = 0;
    while(str != '\0') {
        str++;
        num++;
    }
    
    return num;
}

char* wzStringCat(char* str1, char* str2)
{
    char *top;
    top=str1;
 
    while(*str1++ != 0x00);    
    str1--;
 
    while((*str1++ = *str2++) != 0x00);
    return top;
}
