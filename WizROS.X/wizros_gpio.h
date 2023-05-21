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
 *  File Name : wizros_gpio.h
 *
***************************************************************************/

#ifndef _WIZROS_GPIO_H_
#define	_WIZROS_GPIO_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
#define WIZ_READ_A0     PORTAbits.RA0 //Input Output
#define WIZ_READ_A1     PORTAbits.RA1
#define WIZ_READ_A2     PORTAbits.RA2
#define WIZ_READ_A3     PORTAbits.RA3
#define WIZ_READ_A5     PORTAbits.RA5
#define WIZ_READ_B0     PORTBbits.RB0
#define WIZ_READ_B1     PORTBbits.RB1
#define WIZ_READ_B2     PORTBbits.RB2   //CANTX
#define WIZ_READ_B3     PORTBbits.RB3   //CANRX
#define WIZ_READ_B4     PORTBbits.RB4
#define WIZ_READ_B5     PORTBbits.RB5
#define WIZ_READ_B6     PORTBbits.RB6
#define WIZ_READ_B7     PORTBbits.RB7
#define WIZ_READ_C0     PORTCbits.RC0
#define WIZ_READ_C1     PORTCbits.RC1
#define WIZ_READ_C2     PORTCbits.RC2
#define WIZ_READ_C3     PORTCbits.RC3
#define WIZ_READ_C4     PORTCbits.RC4
#define WIZ_READ_C5     PORTCbits.RC5
#define WIZ_READ_C6     PORTCbits.RC6
#define WIZ_READ_C7     PORTCbits.RC7

#define WIZ_WRITE_A0    LATAbits.LA0  //Output only
#define WIZ_WRITE_A1    LATAbits.LA1
#define WIZ_WRITE_A2    LATAbits.LA2
#define WIZ_WRITE_A3    LATAbits.LA3
#define WIZ_WRITE_A5    LATAbits.LA5
#define WIZ_WRITE_B0    LATBbits.LB0
#define WIZ_WRITE_B1    LATBbits.LB1
#define WIZ_WRITE_B2    LATBbits.LB2    //CANTX
#define WIZ_WRITE_B3    LATBbits.LB3    //CANRX
#define WIZ_WRITE_B4    LATBbits.LB4
#define WIZ_WRITE_B5    LATBbits.LB5
#define WIZ_WRITE_B6    LATBbits.LB6
#define WIZ_WRITE_B7    LATBbits.LB7
#define WIZ_WRITE_C0    LATCbits.LC0
#define WIZ_WRITE_C1    LATCbits.LC1
#define WIZ_WRITE_C2    LATCbits.LC2
#define WIZ_WRITE_C3    LATCbits.LC3
#define WIZ_WRITE_C4    LATCbits.LC4
#define WIZ_WRITE_C5    LATCbits.LC5
#define WIZ_WRITE_C6    LATCbits.LC6
#define WIZ_WRITE_C7    LATCbits.LC7

//PIN MODE
enum wzPinMode {
    WIZ_PINMODE_A0 = 0,
    WIZ_PINMODE_A1,
    WIZ_PINMODE_A2,
    WIZ_PINMODE_A3,
    WIZ_PINMODE_A5,
    WIZ_PINMODE_B0,
    WIZ_PINMODE_B1,
    WIZ_PINMODE_B2, //CANTX
    WIZ_PINMODE_B3, //CANRX
    WIZ_PINMODE_B4,
    WIZ_PINMODE_B5,
    WIZ_PINMODE_B6,
    WIZ_PINMODE_B7,
    WIZ_PINMODE_C0,
    WIZ_PINMODE_C1,
    WIZ_PINMODE_C2,
    WIZ_PINMODE_C3,
    WIZ_PINMODE_C4,
    WIZ_PINMODE_C5,
    WIZ_PINMODE_C6,
    WIZ_PINMODE_C7
};

//MODE
#define WIZ_PINMODEINOUT_OUT      (0)
#define WIZ_PINMODEINOUT_IN       (1)

//ANALOGMODE
enum wzPinAnagloMode {
    WIZ_PINANALOG_A0 = 0,
    WIZ_PINANALOG_A1,
    WIZ_PINANALOG_A2,
    WIZ_PINANALOG_A3,
    WIZ_PINANALOG_A5,
    WIZ_PINANALOG_B1,
    WIZ_PINANALOG_B4,
    WIZ_PINANALOG_B0
};

//PULLUPMODE
enum wzPinPullUpMode {
    WIZ_PINPULLUP_B0 = 0,
    WIZ_PINPULLUP_B1,
    WIZ_PINPULLUP_B2, //CANTX
    WIZ_PINPULLUP_B3, //CANRX
    WIZ_PINPULLUP_B4,
    WIZ_PINPULLUP_B5,
    WIZ_PINPULLUP_B6,
    WIZ_PINPULLUP_B7
};

//-------------------------------------------------------------------------
//  Macro Functions
//-------------------------------------------------------------------------
#define wzGPIODigitalRead(pin)          pin
#define wzGPIODigitalWrite(pin, value)  pin = value
#define wzGPIODigitalWriteSwitch(pin)   pin ^= 1

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzGPIOPinInOutMode(enum wzPinMode pin, uint8_t mode);

void wzGPIOAnalogMode(enum wzPinAnagloMode pin, bool_t enable);
uint16_t wzGPIOAnalogRead(enum wzPinAnagloMode pin);
void wzGPIOPullUpInputMode(enum wzPinPullUpMode, bool_t enable);

//intmode
void wzGPIOIntModePinB0(bool_t enable, bool_t edge_rise);
void wzGPIOIntModePinB1(bool_t enable, bool_t edge_rise);
void wzGPIOIntModePinB2(bool_t enable, bool_t edge_rise);   //Share with CAN
void wzGPIOIntModePinB3(bool_t enable, bool_t edge_rise);   //Share with CAN

void wzGPIOHandleInt(void);
void wzGPIOIntTaskPinB0(taskfunc_t task);
void wzGPIOIntTaskPinB1(taskfunc_t task);
void wzGPIOIntTaskPinB2(taskfunc_t task);   //Share with CAN
void wzGPIOIntTaskPinB3(taskfunc_t task);   //Share with CAN

#endif /*_WIZROS_GPIO_H_*/