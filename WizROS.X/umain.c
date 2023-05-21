/**************************************************************************
 *
 *              Copyright (c) 2014-2016 by Wizapply.
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
 *  WIZROS v1.0 for PIC18F25K80
 *
 *  Language is 'C' code source
 *  Compiler Tools : XC8 v1.37 PRO
 *  MClocks : 64 MHz (16 DMIPS)
 *
 *  File Name : umain_dev.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"
#include "wizros_eusart.h"
#include "wizros_ecan.h"
#include "wizros_timer0.h"
#include "wizros_gpio.h"
#include "wizros_eeprom.h"

//-------------------------------------------------------------------------
//  EEPROM INIT DATA
//-------------------------------------------------------------------------
//__EEPROM_DATA(0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00);

/**************************************************************************
 * User Program
***************************************************************************/

//-------------------------------------------------------------------------
//  Prototype functions
//-------------------------------------------------------------------------
void taskloop();
void tmr_taskloop();

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
WIZ_CAN_PACKET g_packet;

//-------------------------------------------------------------------------
//  wzSetup : This Function is called once of the beginning.
//-------------------------------------------------------------------------
void wzSetup()
{
    //USART
    wzEUSARTInitialize(WIZ_EUSART_BRFLAG_9600);
    
    //TIMER
    wzTMR0Initialize(WIZ_TMR0_FLAG_INTERNALCLK);
    wzTMR0IntTask(tmr_taskloop);
    
    //CAN
    wzECANInitialize(WIZ_ECAN_NORMAL_MODE,
                     WIZ_ECAN_ALL_VALID_MSG,
                     WIZ_ECAN_BAND_250K);
    wzECANSetFilter(0x0001, 0x000F);
    
    //Alert
    wzGPIOPinInOutMode(WIZ_PINMODE_A0, WIZ_PINMODEINOUT_OUT);
    wzGPIODigitalWrite(WIZ_WRITE_A0, bDisable);
    
    wzDelayMSec(1);

    g_packet.isRemote = bFalse;
    
    wzTaskEnqueue(taskloop);
    wzTMR0StartMSec(200);
}

//-------------------------------------------------------------------------
//  Loop
//-------------------------------------------------------------------------
void taskloop()
{
    uint8_t n = wzEUSARTDataAvailable();
    if(n != 0)
    {
        uint8_t data = wzEUSARTReadData();
        wzEUSARTWriteChar(data);
        wzEUSARTWriteNewLine();
    }
    
    wzTaskEnqueue(taskloop);  //loop
}

//-------------------------------------------------------------------------
//  TimerLoop
//-------------------------------------------------------------------------
void tmr_taskloop()
{
    wzTMR0StartMSec(200);
    
    //Switch
    wzGPIODigitalWriteSwitch(WIZ_WRITE_A0);
}
