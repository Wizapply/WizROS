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
 *  File Name : wizros_prg.h
 *
***************************************************************************/

#ifndef _WIZROS_PROGRAMMER_H_
#define	_WIZROS_PROGRAMMER_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------

#define WIZ_PRG_FIRSTADDRESS     (0xFFFFC0)
#define WIZ_PRG_FIRSTADDRESS_N   (0x00003F)
#define WIZ_PRG_DATABUFFER16     (32)
#define WIZ_PRG_DATABUFFER8      (64)

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
uint16_t wzPRGReading1word(uint24_t flashmem_address);

bool_t wzPRGReading32word(uint24_t flashmem_address, uint16_t* data_buffer32word);
bool_t wzPRGErasing32word(uint24_t flashmem_address);
bool_t wzPRGWriting32word(uint24_t flashmem_address, uint16_t* data_buffer32word);

#endif /*_WIZROS_PROGRAMMER_H_*/