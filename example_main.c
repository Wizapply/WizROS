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
 *  File Name : example_main.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"
#include "wizros_eusart.h"
#include "wizros_pwm.h"
#include "wizros_ecan.h"
#include "wizros_mssp.h"
#include "wizros_timer0.h"
#include "wizros_timer1.h"
#include "wizros_timer2.h"
#include "wizros_gpio.h"
#include "wizros_eeprom.h"
#include "wizros_utility.h"

/**************************************************************************
 * User Program
***************************************************************************/

//-------------------------------------------------------------------------
//  Prototype functions
//-------------------------------------------------------------------------
void taskloop();

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//  wzSetup : This Function is called once of the beginning.
//-------------------------------------------------------------------------
void wzSetup()
{
    wzTaskEnqueue(taskloop);
}

//-------------------------------------------------------------------------
//  Loop
//-------------------------------------------------------------------------
void taskloop()
{
    
    //Loop
    
    wzTaskEnqueue(taskloop);  //loop
}