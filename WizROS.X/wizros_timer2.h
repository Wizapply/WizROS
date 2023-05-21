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
 *  File Name : wizros_timer2.h
 *
***************************************************************************/

#ifndef _WIZROS_TIMER2_H_
#define	_WIZROS_TIMER2_H_

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
void wzTMR2Initialize(void);

void wzTMR2StartUSec(uint16_t usec);
void wzTMR2StartMSec(uint16_t msec);
void wzTMR2StartDirect(uint8_t tmr2, uint8_t t2ckps);
void wzTMR2Stop(void);

void wzTMR2HandleInt(void);
void wzTMR2IntTask(taskfunc_t task);

#endif /*_WIZROS_TIMER2_H_*/