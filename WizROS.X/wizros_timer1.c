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
 *  File Name : wizros_timer1.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_timer1.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile taskfunc_t g_intTMR1Task;	//Task 

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzTMR1Initialize(uint8_t flag)
{
    T1CON = 0x00;
    T1CONbits.T1CKPS = 0;       //Timer1 Input Clock Prescale Select bits
    T1CONbits.RD16 = 1;         //Enables register read/write of Timer1 in one 16-bit operation
    T1CONbits.TMR1ON = 0;       //OFF
    T1CONbits.nT1SYNC = 1;      //This bit is ignored. No Sync.
    
    switch(flag) {
        case WIZ_TMR1_FLAG_INTERNALCLK:
        default:
            T1CONbits.SOSCEN = 0;       //SOSC disabled for Timer1
            T1CONbits.TMR1CS = 0b00;    //Timer1 clock source is instruction clock (FOSC/4)
            break;
        case WIZ_TMR1_FLAG_EXTERNAL_LH:
            T1CONbits.SOSCEN = 1;       //SOSC disabled for Timer1
            T1CONbits.TMR1CS = 0b10;    //External clock is from the T1CKI pin (on the rising edge).
            wzGPIOPinInOutMode(WIZ_PINMODE_A5, WIZ_PINMODEINOUT_IN);
            break;
    }
    
    //Set
    PIE1bits.TMR1IE = 1;
    PIR1bits.TMR1IF = 0;
   
    TMR1H = 0x00;
    TMR1L = 0x00;
    
    g_intTMR1Task = WIZ_TASK_NULL;
}

void wzTMR1StartUSec(uint16_t usec)
{
    uint16_t settime;
    //(1 / 64MHz) * 4 = 0.0625us
    // 0.5us / 0.0625us = 8
    T1CONbits.T1CKPS = 0b11;  //1:8

    settime = 0xFFFF - (usec<<1) + 1;
    TMR1 = settime;
    T1CONbits.TMR1ON = bEnable;
    PIR1bits.TMR1IF = 0;
}

void wzTMR1StartDirect(uint16_t tmr1, uint8_t t1ckps)
{
    T1CONbits.T1CKPS = t1ckps;
    TMR1 = tmr1;
    
    T1CONbits.TMR1ON = bEnable;
    PIR1bits.TMR1IF = 0;
}

void wzTMR1Stop(void)
{
    T1CONbits.TMR1ON = bDisable;
    PIR1bits.TMR1IF = 0;
}

void wzTMR1HandleInt(void)
{
    //Enqueue task
    if(g_intTMR1Task != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intTMR1Task);  //int
    
    T1CONbits.TMR1ON = bDisable;
}

void wzTMR1IntTask(taskfunc_t task)
{
    g_intTMR1Task = task;
}