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
 *  File Name : wizros_timer2.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_timer2.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile taskfunc_t g_intTMR2Task;	//Task 
volatile uint16_t g_secTMR2Count;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzTMR2Initialize(void)
{
    T2CON = 0x00;
    T2CONbits.T2CKPS = 0;           //Timer1 Input Clock Prescale Select bits
    T2CONbits.TMR2ON = 0;           //OFF
    T2CONbits.T2OUTPS = 0b0000;     //This bit is ignored. No Sync.
    
    //Set
    PIE1bits.TMR2IE = 1;
    PIR1bits.TMR2IF = 0;
    
    TMR2 = 0x00;
    PR2 = 0xFF;
    
    g_secTMR2Count = 0;
    
    g_intTMR2Task = WIZ_TASK_NULL;
}

void wzTMR2StartUSec(uint16_t usec)
{
    uint8_t settime;
    //(1 / 64MHz) * 4 = 0.0625us
    // 1us / 0.0625us = 16
    T2CONbits.T2OUTPS = 0b1111; //1:16
    
    if(usec > (256*4)) {   //1:16
        T2CONbits.T2CKPS = 0b10;
        settime = (uint8_t)(usec / 16) - 1;
    }
    else
    if(usec > 256) {     //1:4
        T2CONbits.T2CKPS = 0b01;
        settime = (uint8_t)(usec / 4) - 1;
    }
    else
    {                           //1:1
        T2CONbits.T2CKPS = 0b00;
        settime = (uint8_t)(usec) - 1;
    }
   
    g_secTMR2Count = 0;
    
    TMR2 = 0xFF - settime + 1;
    T2CONbits.TMR2ON = bEnable;
    PIR1bits.TMR2IF = 0;
}

void wzTMR2StartMSec(uint16_t msec)
{
    uint32_t calc_msec = msec;
    calc_msec *= 1000;
    calc_msec /= 4096;
    
    //(1 / 64MHz) * 4 = 0.0625us
    // 1us / 0.0625us = 16
    T2CONbits.T2OUTPS = 0b1111; //1:16
    
    //1:16
    T2CONbits.T2CKPS = 0b10;//1:16
    TMR2 = 0x00;    //256
    
    //16*16*256*0.0625us = 4096us
    g_secTMR2Count = (uint16_t)(calc_msec & 0x0000FFFF);
    
    T2CONbits.TMR2ON = bEnable;
    PIR1bits.TMR2IF = 0;
}

void wzTMR2StartDirect(uint8_t tmr2, uint8_t t2ckps)
{
    T2CONbits.T2CKPS = t2ckps;
    TMR2 = tmr2;
    
    T2CONbits.TMR2ON = bEnable;
    PIR1bits.TMR2IF = 0;
}

void wzTMR2Stop(void)
{
    T2CONbits.TMR2ON = bDisable;
    PIR1bits.TMR2IF = 0;
}

void wzTMR2HandleInt(void)
{
    //Enqueue task
    if(g_secTMR2Count == 0) {
        if(g_intTMR2Task != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intTMR2Task);  //int

        T2CONbits.TMR2ON = bDisable;
    } else {
        --g_secTMR2Count;
    }
}

void wzTMR2IntTask(taskfunc_t task)
{
    g_intTMR2Task = task;
}