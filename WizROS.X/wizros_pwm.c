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
 *  File Name : wizros_pwm.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_pwm.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile uint32_t g_freq_tmp_save;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzPWMInitializeB4(uint32_t freq) //4,000,000Hz - 4000Hz
{
    uint32_t freq_tmp;

    CCP1CON = 0b00001100;   //PWM mode / single mode
    PSTR1CON = 0b00010001;  //P1A port

    wzGPIOPinInOutMode(WIZ_PINMODE_B4, WIZ_PINMODEINOUT_OUT); 

    ECCP1AS = 0x00;
    ECCP1DEL = 0x00;

    //Setting
    // FOSC/4 = 16MHz
    T4CON = 0x00;
    T4CONbits.T4OUTPS = 0b0000; //1:1 Postscale, PWM is not use.

    freq_tmp = (uint32_t)16000000 / freq;  //1:1
    if(freq_tmp > (256*4)) {   //1:16
        T4CONbits.T4CKPS = 0b10;
        PR4 = (uint8_t)(freq_tmp / 16) - 1;
    }
    else
    if(freq_tmp > (256)) {     //1:4
        T4CONbits.T4CKPS = 0b01;
        PR4 = (uint8_t)(freq_tmp / 4) - 1;
    }
    else
    {                           //1:1
        T4CONbits.T4CKPS = 0b00;
        PR4 = (uint8_t)(freq_tmp) - 1;
    }

    g_freq_tmp_save = freq;

    CCP1CONbits.DC1B = 0;
    CCPR1H = 0;
    CCPR1L = PR4>>1; //kari yaku50% PR4+1 = 256?

    T4CONbits.TMR4ON = 1; //Start

    PIR4bits.TMR4IF = 0;
    PIE4bits.TMR4IE = 0;    //No interrupt
}

void wzPWMSetDuty(uint32_t duty_freq)
{
    uint32_t freq_tmp;
    freq_tmp = (duty_freq << 8) / g_freq_tmp_save;
    freq_tmp = ((uint32_t)(PR4) * (freq_tmp)) >> 6;

    CCPR1H = 0;
    CCP1CONbits.DC1B = (uint8_t)(freq_tmp & 0x00000003);
    CCPR1L = (uint8_t)(freq_tmp>>2);
}

void wzPWMStart()
{
    TMR4 = 0;
    PIR4bits.TMR4IF = 0;
    T4CONbits.TMR4ON = 1; //Start
}

void wzPWMStop()
{
    TMR4 = 0;
    PIR4bits.TMR4IF = 0;
    T4CONbits.TMR4ON = 0; //Stop
}
