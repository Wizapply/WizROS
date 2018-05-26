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
 *  File Name : wizros_prg.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_prg.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile uint16_t g_wzPRGBuff[WIZ_PRG_DATABUFFER16];

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
uint16_t wzPRGReading1word(uint24_t flashmem_address)
{
    uint16_t data = 0x00;
    
    TBLPTR = flashmem_address;

    asm("TBLRD*+");     //PIC is Little-endian
    data |= TABLAT;
    asm("TBLRD*+");
    data |= ((uint16_t)TABLAT << 8);
    
    return data;
}

bool_t wzPRGReading32word(uint24_t flashmem_address, uint16_t* data_buffer32word)
{
    uint8_t i;
    uint8_t counter = WIZ_PRG_DATABUFFER16;
    
    TBLPTR = flashmem_address & WIZ_PRG_FIRSTADDRESS;
    
    for(i=0; i < counter; i++) {
        g_wzPRGBuff[i] = 0x00;
        asm("TBLRD*+");
        g_wzPRGBuff[i] |= TABLAT;
        asm("TBLRD*+");
        g_wzPRGBuff[i] |= ((uint16_t)TABLAT << 8);
    }
    
    for(i=0; i < counter; i++) {
        data_buffer32word[i] = g_wzPRGBuff[i];
    }
    
    return bTrue;
}

bool_t wzPRGErasing32word(uint24_t flashmem_address)
{
    //The erase blocks are 32 words or 64 bytes.
    //Word erase in the Flash array is not supported.
    //TBLPTR<21:6> point to the block being erased. The
    //TBLPTR<5:0> bits are ignored.
    TBLPTR = flashmem_address & WIZ_PRG_FIRSTADDRESS;
    
    EECON1 = 0x00;
    EECON1bits.EEPGD = 1;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    EECON1bits.FREE = 1;    //Erase the program memory row addressed by TBLPTR on the next WR command
    
    //Interrput off
    INTCONbits.GIE = 0;
    //Required Sequence : password
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;  //start erase (CPU stall)
    NOP();
    while(EECON1bits.WR) NOP();
    
    //Interrput on
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
    
    return !EECON1bits.WRERR;
}

bool_t wzPRGWriting32word(uint24_t flashmem_address, uint16_t* data_buffer32word)
{
    uint8_t i;
    uint8_t counter = WIZ_PRG_DATABUFFER16;
    
    for(i=0; i < counter; i++)
        g_wzPRGBuff[i] = data_buffer32word[i];
    
    if(!wzPRGErasing32word(flashmem_address))
        return bFalse;
    
    //dummy read decrement
    asm("TBLRD*-");
    
    for(i=0; i < counter; i++) {
        TABLAT = g_wzPRGBuff[i] & 0x00FF;
        asm("TBLWT+*");
        TABLAT = (g_wzPRGBuff[i] >> 8) & 0x00FF;
        asm("TBLWT+*");
    }
    
    EECON1 = 0x00;
    EECON1bits.EEPGD = 1;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    EECON1bits.FREE = 0;    //Write
    
    //Interrput off
    INTCONbits.GIE = 0;
    //Required Sequence : password
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;  //start program (CPU stall)
    NOP();
    while(EECON1bits.WR) NOP(); //write check
    
    //Interrput on
    INTCONbits.GIE = 1;
    EECON1bits.WREN = 0;
    
   return !EECON1bits.WRERR;
}
