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
 *  WIZROS v2.0 for PIC18F25K80
 *
 *  Language is 'C' code source
 *  Compiler Tools : XC8 v1.37 PRO
 *  MClocks : 64 MHz (16 DMIPS)
 *
 *  File Name : wizros_utility.h
 *
***************************************************************************/

#ifndef _WIZROS_UTILITY_H_
#define	_WIZROS_UTILITY_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
uint8_t wzStringToUint8(const uint8_t* str, int8_t len);
uint16_t wzStringLEToUint16(const uint8_t* str, int8_t len);    //little
uint16_t wzStringBEToUint16(const uint8_t* str, int8_t len);    //big
void wzUint8ToString(uint8_t num, uint8_t* ref_str3);
void wzUint16ToStringLE(uint16_t num, uint8_t* ref_str3);
void wzUint16ToStringBE(uint16_t num, uint8_t* ref_str3);

int8_t wzStringToInt8(const uint8_t* str, int8_t len);
int16_t wzStringLEToInt16(const uint8_t* str, int8_t len);    //little
int16_t wzStringBEToInt16(const uint8_t* str, int8_t len);    //big
void wzInt8ToString(int8_t numi, uint8_t* ref_str3);
void wzInt16ToStringLE(int16_t numi, uint8_t* ref_str5);
void wzInt16ToStringBE(int16_t numi, uint8_t* ref_str5);

uint8_t wzCheckDigitByte(uint8_t* str, uint8_t strlen);
uint8_t wzCheckDigitByteString(uint8_t* str, uint8_t strlen);

void wzClearString(uint8_t* str, int8_t len);
void wzMemSet(uint8_t* str, uint8_t set, int8_t len);

uint16_t wzStringLength(uint8_t* str);

uint8_t* wzStringCat(uint8_t* str1, uint8_t* str2);

#endif /*_WIZROS_UTILITY_H_*/