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
 *  File Name : wizros_eeprom.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_eeprom.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
uint8_t wzInternalEEPROMRead(uint16_t address)
{
    EEADR = address & 0xFF;
    EEADRH = (address >> 8) & 0xFF;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
	EECON1bits.RD = 1;  // Set read bit
    NOP();
    
	return EEDATA;		// Return read data
}

void wzInternalEEPROMWrite(uint16_t address, uint8_t data)
{
    while(EECON1bits.WR);   //error
    
    EEADR = address & 0xFF;
    EEADRH = (address >> 8) & 0xFF;
    EEDATA = data;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
	EECON1bits.WREN = 1;    //Enable writes
    
    EECON2 = 0x55;  //Flash Self-Program Control Register (not a physical register)
	EECON2 = 0xAA;
    EECON1bits.WR = 1;  // Set WR bit to begin write
    
    while(!PIR4bits.EEIF);		// Wait till write operation complete
    PIR4bits.EEIF = 0;
    
    EECON1bits.WREN = 0;  // Clear programming bit
}
