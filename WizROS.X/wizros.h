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
 *  File Name : wizros.h
 *
***************************************************************************/

#pragma warning disable 520
#pragma warning disable 1498

#ifndef _WIZROS_H_	// Don't include more than once (last line = #endif)
#define _WIZROS_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include <xc.h>
#include <stdio.h>

//-------------------------------------------------------------------------
//  Typedefs
//-------------------------------------------------------------------------
// Boolean
typedef enum _bool_t
{
	bFalse = 0,
	bDisable = 0,
	bTrue = 1,
	bEnable = 1
} bool_t;

// 8bit
typedef unsigned char           uint8_t;
typedef signed char             int8_t;
typedef signed char             char8_t;

// 16bit
typedef unsigned short          uint16_t;
typedef signed short            int16_t;

// 32bit
typedef	unsigned long           uint32_t;
typedef	signed long             int32_t;

// float 24bit or 32bit By config
typedef	float                   float_t;

//function pointer
typedef	void (*taskfunc_t)(void);

/**************************************************************************
 * Defines
***************************************************************************/

// WizRos Max task size
#define WIZ_TASK_SIZE     (16)
#define WIZ_TASK_NULL     (0)

// WizROS version string
#define WIZ_VERSION_NAME "WizROS 3.0"

#define _XTAL_FREQ  (64000000)  //64Mhz for delay

// NOTE: To use the macros below, YOU must have previously defined _XTAL_FREQ
#define wzDelayUSec(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define wzDelayMSec(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#define wzDelayWDTUSec(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define wzDelayWDTMSec(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000.0)))

#define wzPRG_GDSGN_PTR (0x2006)

//-------------------------------------------------------------------------
//  Prototype functions
//-------------------------------------------------------------------------
void wzTaskEnqueue(taskfunc_t task);
void wzTaskEnqueueForInt(taskfunc_t task);
taskfunc_t wzTaskDnqueue(void);
bool_t wzTaskEmptyQueue(void);
void wzTaskClear(void);

void wzSleepMode(void);

//-------------------------------------------------------------------------
//  EntryPoint functions
//-------------------------------------------------------------------------
void wzSetup(void);

#endif