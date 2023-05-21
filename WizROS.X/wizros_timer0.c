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
 *  File Name : wizros_timer0.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_timer0.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile taskfunc_t g_intTMR0Task;	//Task 

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzTMR0Initialize(uint8_t flag)
{
    T0CON = 0x00;
    T0CONbits.T08BIT = 0; //Timer0 is configured as a 16-bit timer/counter
    T0CONbits.PSA = 0;    //Timer0 prescaler is assigned. Timer0 clock input comes from prescaler output
    T0CONbits.T0PS = 0;   //Timer0 Prescaler Select bits
    T0CONbits.TMR0ON = 0;   //OFF
    
    switch(flag) {
        case WIZ_TMR0_FLAG_INTERNALCLK:
        default:
            T0CONbits.T0CS = 0;   //Internal instruction cycle clock (CLKO)
            T0CONbits.T0SE = 0;   //Increment on low-to-high transition on T0CKI pin
            break;
        case WIZ_TMR0_FLAG_EXTERNAL_HL:
            T0CONbits.T0CS = 1;   //Transition on T0CKI pin
            T0CONbits.T0SE = 1;   //Increment on high-to-low transition on T0CKI pin
            wzGPIOPinInOutMode(WIZ_PINMODE_B5, WIZ_PINMODEINOUT_IN);
            break;
        case WIZ_TMR0_FLAG_EXTERNAL_LH:
            T0CONbits.T0CS = 1;   //Transition on T0CKI pin
            T0CONbits.T0SE = 0;   //Increment on low-to-high transition on T0CKI pin
            wzGPIOPinInOutMode(WIZ_PINMODE_B5, WIZ_PINMODEINOUT_IN);
            break;
    }
    
    //Set
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    
    TMR0H = 0x00;
    TMR0L = 0x00;
    
    g_intTMR0Task = WIZ_TASK_NULL;
}

void wzTMR0StartUSec(uint16_t usec)
{
    uint16_t settime;
    //(1 / 64MHz) * 4 = 0.0625us
    // 1us / 0.0625us = 16
    T0CONbits.PSA = 0;  //enable
    T0CONbits.T0PS = 0b011;  //1:16

    settime = 0xFFFF - usec + 1;
    TMR0 = settime;
    T0CONbits.TMR0ON = bEnable;
    INTCONbits.TMR0IF = 0;
}

void wzTMR0StartMSec(uint16_t msec)
{
    uint16_t settime;
    
    if(msec <= 500) {
        //(1 / 64MHz) * 4 = 0.0625us
        // 1000us / 0.0625us = 16000
        // 16000 / 128 = 125;
        T0CONbits.PSA = 0;  //enable
        T0CONbits.T0PS = 0b110;  //1:128

        settime = 0xFFFF - (125*msec) + 1;
    } else {
        //(1 / 64MHz) * 4 = 0.0625us
        // 1000us / 0.0625us = 16000
        // 16000 / 256 = 62.5;
        T0CONbits.PSA = 0;  //enable
        T0CONbits.T0PS = 0b110;  //1:128

        settime = 0xFFFF - (62*msec) - (msec>>1) + 1; //62.5
    }
    TMR0 = settime;
    T0CONbits.TMR0ON = bEnable;
    INTCONbits.TMR0IF = 0;
}

void wzTMR0StartDirect(uint16_t tmr0, bool_t psa, uint8_t t0ps)
{
    T0CONbits.PSA = psa;
    T0CONbits.T0PS = t0ps;
    TMR0 = tmr0;
    
    T0CONbits.TMR0ON = bEnable;
    INTCONbits.TMR0IF = 0;
}

void wzTMR0Stop(void)
{
    T0CONbits.TMR0ON= bDisable;
    INTCONbits.TMR0IF = 0;
}

void wzTMR0HandleInt(void)
{
    //Enqueue task
    if(g_intTMR0Task != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intTMR0Task);  //int
    
    T0CONbits.TMR0ON = bDisable;
}

void wzTMR0IntTask(taskfunc_t task)
{
    g_intTMR0Task = task;
}