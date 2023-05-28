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
 *  File Name : wizros_gpio.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile taskfunc_t g_intPIN21Task;	//Task pin21
volatile taskfunc_t g_intPIN22Task;	//Task pin22
volatile taskfunc_t g_intPIN23Task;	//Task pin23
volatile taskfunc_t g_intPIN24Task;	//Task pin24

volatile uint8_t g_TRISAConf;
volatile uint8_t g_TRISBConf;
volatile uint8_t g_TRISCConf;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzGPIOPinInOutMode(enum wzPinMode pin, uint8_t mode)
{
    switch(pin) {
        case WIZ_PINMODE_A0: TRISAbits.TRISA0 = mode; g_TRISAConf |= 0b00000001;
            break;
        case WIZ_PINMODE_A1: TRISAbits.TRISA1 = mode; g_TRISAConf |= 0b00000010;
            break;
        case WIZ_PINMODE_A2: TRISAbits.TRISA2 = mode; g_TRISAConf |= 0b00000100;
            break;
        case WIZ_PINMODE_A3: TRISAbits.TRISA3 = mode; g_TRISAConf |= 0b00010000;
            break;
        case WIZ_PINMODE_A5: TRISAbits.TRISA5 = mode; g_TRISAConf |= 0b00100000;
            break;
        case WIZ_PINMODE_B0: TRISBbits.TRISB0 = mode; g_TRISBConf |= 0b00000001;
            break;
        case WIZ_PINMODE_B1: TRISBbits.TRISB1 = mode; g_TRISBConf |= 0b00000010;
            break;
        case WIZ_PINMODE_B2: TRISBbits.TRISB2 = mode; g_TRISBConf |= 0b00000100;
            break;
        case WIZ_PINMODE_B3: TRISBbits.TRISB3 = mode; g_TRISBConf |= 0b00001000;
            break;
        case WIZ_PINMODE_B4: TRISBbits.TRISB4 = mode; g_TRISBConf |= 0b00010000;
            break;
        case WIZ_PINMODE_B5: TRISBbits.TRISB5 = mode; g_TRISBConf |= 0b00100000;
            break;
        case WIZ_PINMODE_B6: TRISBbits.TRISB6 = mode; g_TRISBConf |= 0b01000000;
            break;
        case WIZ_PINMODE_B7: TRISBbits.TRISB7 = mode; g_TRISBConf |= 0b10000000;
            break;
        case WIZ_PINMODE_C0: TRISCbits.TRISC0 = mode; g_TRISCConf |= 0b00000001;
            break;
        case WIZ_PINMODE_C1: TRISCbits.TRISC1 = mode; g_TRISCConf |= 0b00000010;
            break;
        case WIZ_PINMODE_C2: TRISCbits.TRISC2 = mode; g_TRISCConf |= 0b00000100;
            break;
        case WIZ_PINMODE_C3: TRISCbits.TRISC3 = mode; g_TRISCConf |= 0b00001000;
            break;
        case WIZ_PINMODE_C4: TRISCbits.TRISC4 = mode; g_TRISCConf |= 0b00010000;
            break;
        case WIZ_PINMODE_C5: TRISCbits.TRISC5 = mode; g_TRISCConf |= 0b00100000;
            break;
        case WIZ_PINMODE_C6: TRISCbits.TRISC6 = mode; g_TRISCConf |= 0b01000000;
            break;
        case WIZ_PINMODE_C7: TRISCbits.TRISC7 = mode; g_TRISCConf |= 0b10000000;
            break;
        default:
            break;
    }
}

void wzGPIOAnalogMode(enum wzPinAnagloMode pin, bool_t enable)
{
    switch(pin) {
        case WIZ_PINANALOG_A0:
            wzGPIOPinInOutMode(WIZ_PINMODE_A0, (uint8_t)enable); //true = WIZ_PINMODEINOUT_IN
            ANCON0bits.ANSEL0 = enable;
            break;
        case WIZ_PINANALOG_A1:
            wzGPIOPinInOutMode(WIZ_PINMODE_A1, (uint8_t)enable);
            ANCON0bits.ANSEL1 = enable;
            break;
        case WIZ_PINANALOG_A2:
            wzGPIOPinInOutMode(WIZ_PINMODE_A2, (uint8_t)enable);
            ANCON0bits.ANSEL2 = enable;
            break;
        case WIZ_PINANALOG_A3:
            wzGPIOPinInOutMode(WIZ_PINMODE_A3, (uint8_t)enable);
            ANCON0bits.ANSEL3 = enable;
            break;
        case WIZ_PINANALOG_A5:
            wzGPIOPinInOutMode(WIZ_PINMODE_A5, (uint8_t)enable);
            ANCON0bits.ANSEL4 = enable;
            break;
        case WIZ_PINANALOG_B1:
            wzGPIOPinInOutMode(WIZ_PINMODE_B1, (uint8_t)enable);
            ANCON1bits.ANSEL8 = enable;
            break;
        case WIZ_PINANALOG_B4:
            wzGPIOPinInOutMode(WIZ_PINMODE_B4, (uint8_t)enable);
            ANCON1bits.ANSEL9 = enable;
            break;
        case WIZ_PINANALOG_B0:
            wzGPIOPinInOutMode(WIZ_PINMODE_B0, (uint8_t)enable);
            ANCON1bits.ANSEL10 = enable;
            break;
        default:
            break;
    }
}

uint16_t wzGPIOAnalogRead(enum wzPinAnagloMode pin)
{
    uint16_t ad_res;
    
    switch(pin) {
        case WIZ_PINANALOG_A0: ADCON0bits.CHS = 0b00000;
            break;
        case WIZ_PINANALOG_A1: ADCON0bits.CHS = 0b00001;
            break;
        case WIZ_PINANALOG_A2: ADCON0bits.CHS = 0b00010;
            break;
        case WIZ_PINANALOG_A3: ADCON0bits.CHS = 0b00011;
            break;
        case WIZ_PINANALOG_A5: ADCON0bits.CHS = 0b00100;
            break;
        case WIZ_PINANALOG_B1: ADCON0bits.CHS = 0b01000;
            break;
        case WIZ_PINANALOG_B4: ADCON0bits.CHS = 0b01001;
            break;
        case WIZ_PINANALOG_B0: ADCON0bits.CHS = 0b01010;
            break;
        default:
            return 0; //end
    }
    
    ADCON0bits.ADON = 1;    //Start
    wzDelayUSec(30);         //30us wait
    ADCON0bits.GO_DONE = 1;
    while(ADCON0bits.GO_DONE);
   
    ad_res = ADRES;
    if(ad_res > 0x0FFF)
        ad_res = 0;
    
    return ad_res;
}

void wzGPIOPullUpInputMode(enum wzPinPullUpMode pin, bool_t enable)
{
    //PullUp Enable
    switch(pin) {
        case WIZ_PINPULLUP_B0: WPUBbits.WPUB0 = enable;
            break;
        case WIZ_PINPULLUP_B1: WPUBbits.WPUB1 = enable;
            break;
        case WIZ_PINPULLUP_B2: WPUBbits.WPUB2 = enable;
            break;
        case WIZ_PINPULLUP_B3: WPUBbits.WPUB3 = enable;
            break;       
        case WIZ_PINPULLUP_B4: WPUBbits.WPUB4 = enable;
            break;
        case WIZ_PINPULLUP_B5: WPUBbits.WPUB5 = enable;
            break;
        case WIZ_PINPULLUP_B6: WPUBbits.WPUB6 = enable;
            break;
        case WIZ_PINPULLUP_B7: WPUBbits.WPUB7 = enable;
            break;
        default:
            return; //end
    }
    
    if(WPUB==0x00) {  //All Disable
        INTCON2bits.nRBPU = 1;
    } else {
        INTCON2bits.nRBPU = 0; //PullUp Enable
    }
}

//intmode
void wzGPIOIntModePinB0(bool_t enable, bool_t edge_rise)
{
    if(enable) {
        wzGPIOPinInOutMode(WIZ_PINMODE_B0, WIZ_PINMODEINOUT_IN);
        
        INTCON2bits.INTEDG0 = edge_rise;
        INTCONbits.INT0IE = 1;
        INTCONbits.INT0IF = 0;
    } else {
        wzGPIOPinInOutMode(WIZ_PINMODE_B0, WIZ_PINMODEINOUT_OUT);
        
        INTCONbits.INT0IE = 0;
        INTCONbits.INT0IF = 0;
    }
}

void wzGPIOIntModePinB1(bool_t enable, bool_t edge_rise)
{
    if(enable) {
        wzGPIOPinInOutMode(WIZ_PINMODE_B1, WIZ_PINMODEINOUT_IN);
        
        INTCON2bits.INTEDG1 = edge_rise;
        INTCON3bits.INT1IE = 1;
        INTCON3bits.INT1IF = 0;
    } else {
        wzGPIOPinInOutMode(WIZ_PINMODE_B1, WIZ_PINMODEINOUT_OUT);
        
        INTCON3bits.INT1IE = 0;
        INTCON3bits.INT1IF = 0;
    }
}
void wzGPIOIntModePinB2(bool_t enable, bool_t edge_rise)    //Share with CAN
{
    if(enable) {
        wzGPIOPinInOutMode(WIZ_PINMODE_B2, WIZ_PINMODEINOUT_IN);
        
        INTCON2bits.INTEDG2 = edge_rise;
        INTCON3bits.INT2IE = 1;
        INTCON3bits.INT2IF = 0;
    } else {
        wzGPIOPinInOutMode(WIZ_PINMODE_B2, WIZ_PINMODEINOUT_OUT);
        
        INTCON3bits.INT2IE = 0;
        INTCON3bits.INT2IF = 0;
    }
}
void wzGPIOIntModePinB3(bool_t enable, bool_t edge_rise)   //Share with CAN
{
    if(enable) {
        wzGPIOPinInOutMode(WIZ_PINMODE_B3, WIZ_PINMODEINOUT_IN);
        
        INTCON2bits.INTEDG3 = edge_rise;
        INTCON3bits.INT3IE = 1;
        INTCON3bits.INT3IF = 0;
    } else {
        wzGPIOPinInOutMode(WIZ_PINMODE_B3, WIZ_PINMODEINOUT_OUT);
        
        INTCON3bits.INT3IE = 0;
        INTCON3bits.INT3IF = 0;
    }
}

void wzGPIOHandleInt(void)
{
    if (INTCONbits.INT0IF) {
        //Enqueue task
        if(g_intPIN21Task != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intPIN21Task);  //int
    }
    if (INTCON3bits.INT1IF) {
        //Enqueue task
        if(g_intPIN22Task != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intPIN22Task);  //int
    }
    if (INTCON3bits.INT2IF) {
        //Enqueue task
        if(g_intPIN23Task != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intPIN23Task);  //int
    }
    if (INTCON3bits.INT3IF) {
        //Enqueue task
        if(g_intPIN24Task != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intPIN24Task);  //int
    }
}

void wzGPIOIntTaskPinB0(taskfunc_t task)
{
    g_intPIN21Task = task;
}

void wzGPIOIntTaskPinB1(taskfunc_t task)
{
    g_intPIN22Task = task;
}

void wzGPIOIntTaskPinB2(taskfunc_t task)
{
    g_intPIN23Task = task;
}

void wzGPIOIntTaskPinB3(taskfunc_t task)
{
    g_intPIN24Task = task;
}
//Secret function
void wzGPIOInitialize(void)
{
    g_TRISAConf = 0x00;
    g_TRISBConf = 0x00;
    g_TRISCConf = 0x00;
}

void wzGPIOInitializeTRISPin(void)
{
    TRISA &= g_TRISAConf;
    TRISB &= g_TRISBConf;
    TRISC &= g_TRISCConf;
}