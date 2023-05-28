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
 *  File Name : wizros_timer1.h
 *
***************************************************************************/

#ifndef _WIZROS_TIMER1_H_
#define	_WIZROS_TIMER1_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
#define WIZ_TMR1_FLAG_INTERNALCLK    (0)
#define WIZ_TMR1_FLAG_EXTERNAL_LH    (1)    //16MHz

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzTMR1Initialize(uint8_t flag);

void wzTMR1StartUSec(uint16_t usec);
void wzTMR1StartDirect(uint16_t tmr1, uint8_t t1ckps);
void wzTMR1Stop(void);

void wzTMR1HandleInt(void);
void wzTMR1IntTask(taskfunc_t task);

#endif /*_WIZROS_TIMER1_H_*/