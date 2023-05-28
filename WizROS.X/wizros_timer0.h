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
 *  File Name : wizros_timer0.h
 *
***************************************************************************/

#ifndef _WIZROS_TIMER0_H_
#define	_WIZROS_TIMER0_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
#define WIZ_TMR0_FLAG_INTERNALCLK    (0)
#define WIZ_TMR0_FLAG_EXTERNAL_HL    (1)    //16MHz
#define WIZ_TMR0_FLAG_EXTERNAL_LH    (2)

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzTMR0Initialize(uint8_t flag);

void wzTMR0StartUSec(uint16_t usec);
void wzTMR0StartMSec(uint16_t msec);
void wzTMR0StartDirect(uint16_t tmr0, bool_t psa, uint8_t t0ps);
void wzTMR0Stop(void);

void wzTMR0HandleInt(void);
void wzTMR0IntTask(taskfunc_t task);

#endif /*_WIZROS_TIMER0_H_*/