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
 *  File Name : wizros_bootloader.c
 *
***************************************************************************/
//#define WIZROS_BOOTLOADER
#ifdef WIZROS_BOOTLOADER
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
#include "wizros_prg.h"
#include "wizros_utility.h"

//-------------------------------------------------------------------------
//  EEPROM INIT DATA
//-------------------------------------------------------------------------
//__EEPROM_DATA(0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00);

/**************************************************************************
 * User Program
***************************************************************************/

//-------------------------------------------------------------------------
//  Struct
//-------------------------------------------------------------------------
struct _hexPacket {
    uint8_t byteNum;
    uint8_t type;
    uint16_t address_l;
    uint8_t* pdata;
};

typedef struct _hexPacket HexPacket;

//-------------------------------------------------------------------------
//  Prototype functions
//-------------------------------------------------------------------------
void taskloop();
void taskloop_downloader();
void end_timmer();

bool_t AnalyticsHexData(uint8_t* str, uint8_t len, HexPacket* packet);
bool_t ProgrammingHexData(HexPacket* packet);

void startDownloader();

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
#define WIZROS_BOOTLOADER_MODE      "WIZROS_BOOTMODE"
#define WIZROS_BOOTLOADER_VERSION   "1.0"
#define WIZROS_BOOT_HEXCODE_MAX     (255)

#define RETURN_STATUS_ERROR         "ERR"
#define RETURN_STATUS_WRITEOK       "WOK"
#define RETURN_STATUS_FINISH        "FNS"

//State -> g_programState
#define PROGRAM_STATE_STANDBY      (0)
#define PROGRAM_STATE_READY        (1)
#define PROGRAM_STATE_DATAIN       (2)
#define PROGRAM_STATE_WRITING      (3)
#define PROGRAM_STATE_FINISH       (4)

#define PROGRAM_FLASH_NULL         (0xFFFFFF)

uint8_t g_keycount;
uint8_t g_programCount;
uint8_t g_programHexCode[WIZROS_BOOT_HEXCODE_MAX];
uint8_t g_programState;

uint16_t g_programHexCodeWrite[32];

uint24_t g_flashRAMPosition;
union {
    uint8_t  buf8 [WIZ_PRG_DATABUFFER8];
    uint16_t buf16[WIZ_PRG_DATABUFFER16];
} g_flashRAMBuffer;
uint16_t g_flashRAMHighAddress;

#define APP_SIGNATURE_ADDRESS       0x002006  //0x2006 and 0x2007 contains the "signature" WORD, indicating successful erase/program/verify operation
#define APP_SIGNATURE_VALUE         0x600D    //leet "GOOD", implying that the erase/program was a success and the bootloader intentionally programmed the APP_SIGNATURE_ADDRESS with this value
//ROM RANGES = default,-0-1FFF

//-------------------------------------------------------------------------
//  wzSetup : This Function is called once of the beginning.
//-------------------------------------------------------------------------
void wzSetup()
{
    //USART
    wzEUSARTInitialize(WIZ_EUSART_BRFLAG_115200);
    
    //TIMER
    wzTMR0Initialize(WIZ_TMR0_FLAG_INTERNALCLK);
    wzTMR0IntTask(end_timmer);
    
    g_keycount = 0;
    g_programState = PROGRAM_STATE_STANDBY;
    
    g_flashRAMPosition = PROGRAM_FLASH_NULL;  //ERROR
    g_flashRAMHighAddress = 0x0000;
    
    //TaskLoop
    wzTaskEnqueue(taskloop);
    //1s end Timer
    wzTMR0StartMSec(1000);
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
        if(data == 0x7F) {  //DEL
            g_keycount++;
            if(g_keycount >= 3) {
                startDownloader();
                return;
            }
        } else {
            g_keycount = 0;
        }
    }
    
    wzTaskEnqueue(taskloop);  //loop
}

void taskloop_downloader()
{
    HexPacket packet;
    
    uint8_t n = wzEUSARTDataAvailable();
    while(n != 0)
    {
        uint8_t data = wzEUSARTReadData();
        
        if(data == 0x00 || data == '\r') {    //NUL & <CR>
            //Dispose
            n = wzEUSARTDataAvailable();
            continue;
        }
        
        switch(g_programState)
        {
            case PROGRAM_STATE_STANDBY:
            case PROGRAM_STATE_READY:
                if(data == ':') { //Start : 
                    g_programState = PROGRAM_STATE_DATAIN;
                    g_programCount = 0;
                } 
                break;
            case PROGRAM_STATE_DATAIN:
                if(data == '\n') { //End <LR>
                    g_programState = PROGRAM_STATE_WRITING;
                    //Analytics -> Write
                    if(AnalyticsHexData(&g_programHexCode[0], g_programCount, &packet))
                    {
                        //Programming
                        if(ProgrammingHexData(&packet)) {
                            g_programState = PROGRAM_STATE_FINISH;
                            wzEUSARTWriteLine(RETURN_STATUS_FINISH);
                        } else {
                            g_programState = PROGRAM_STATE_READY;
                            wzEUSARTWriteLine(RETURN_STATUS_WRITEOK);
                        }
                    } else {
                        g_programState = PROGRAM_STATE_STANDBY;
                        wzEUSARTWriteLine(RETURN_STATUS_ERROR);
                    }
                    break;
                }
                
                //in data
                g_programHexCode[g_programCount] = data;
                g_programCount++;
                if(g_programCount >= WIZROS_BOOT_HEXCODE_MAX) {
                     //Error
                    g_programCount = 0;
                    g_programState = PROGRAM_STATE_STANDBY;
                    wzEUSARTFlushBuffer();  //Flash
                    wzEUSARTWriteLine(RETURN_STATUS_ERROR);
                }
                break;
            case PROGRAM_STATE_WRITING:
            case PROGRAM_STATE_FINISH:
                //NOP
                break;
        }
        
        n = wzEUSARTDataAvailable();
    }
    
    wzTaskEnqueue(taskloop_downloader);  //loop
}

//-------------------------------------------------------------------------
//  End Timmer
//-------------------------------------------------------------------------
void end_timmer()
{
    wzTaskClear();
    
    //Program Check
    if(wzPRGReading1word(APP_SIGNATURE_ADDRESS) != APP_SIGNATURE_VALUE) {
        startDownloader();
        return;
    }
    
    //Don't interrupt
    INTCONbits.PEIE = 0;    //Not priority
    INTCONbits.GIE = 0;
    
    //to User Program
    wzDelayMSec(1);
    
    STKPTR = 0x00;
    BSR = 0x00;
    STATUS = 0x00;
    asm("nop");
    asm("goto 0x2000");
}

//-------------------------------------------------------------------------

bool_t AnalyticsHexData(uint8_t* str, uint8_t len, HexPacket* packet)
{
    //Default
    packet->byteNum = 0;
    packet->type = 0;
    
    //min len
    if(len < 10)
        return bFalse;
    
    //Check Sum
    uint8_t lendata = len - 2;
    uint8_t check = wzCheckDigitByteString(&str[0],lendata);
    if(check != wzStringToUint8(&str[lendata], 2))
        return bFalse;
    
    //Byte Count
    packet->byteNum = wzStringToUint8(&str[0],2);
    packet->type = wzStringToUint8(&str[6],2);
    packet->address_l = wzStringBEToUint16(&str[2],4);
    
    if(packet->byteNum > 0)
        packet->pdata = &str[8];    //String Raw
    else
        packet->pdata = 0;
    
    return bTrue;
}

bool_t ProgrammingHexData(HexPacket* packet)
{
    bool_t finish = bFalse;
    uint24_t u24adress = (uint24_t)packet->address_l | ((uint24_t)g_flashRAMHighAddress << 16);

    switch(packet->type)
    {
        case 0x02:  //Linear Address
        case 0x04:  //Extended Address
        {
            if(packet->byteNum < 4)
                break;
            
            //Reload -> Flush
            if(g_flashRAMPosition != PROGRAM_FLASH_NULL) {
                wzPRGWriting32word(g_flashRAMPosition & WIZ_PRG_FIRSTADDRESS, g_flashRAMBuffer.buf16);
            }
            
            g_flashRAMHighAddress = wzStringBEToUint16(packet->pdata,4);
            
            //Reload
            u24adress = ((uint24_t)g_flashRAMHighAddress << 16);
            g_flashRAMPosition = u24adress & WIZ_PRG_FIRSTADDRESS;
            wzPRGReading32word(g_flashRAMPosition, g_flashRAMBuffer.buf16);
            
            g_flashRAMPosition = PROGRAM_FLASH_NULL;
        }
            break;
        case 0x01:  //End File -> Flush
            if(g_flashRAMPosition != PROGRAM_FLASH_NULL) {
                wzPRGWriting32word(g_flashRAMPosition & WIZ_PRG_FIRSTADDRESS, g_flashRAMBuffer.buf16);
                finish = bTrue;
            }
            
            if(finish) {
                //Write Success Signature
                uint8_t uaddr = APP_SIGNATURE_ADDRESS & WIZ_PRG_FIRSTADDRESS_N;
                g_flashRAMPosition = APP_SIGNATURE_ADDRESS & WIZ_PRG_FIRSTADDRESS;
                wzPRGReading32word(g_flashRAMPosition, g_flashRAMBuffer.buf16);

                g_flashRAMBuffer.buf8[uaddr] = APP_SIGNATURE_VALUE & 0x00FF;
                g_flashRAMBuffer.buf8[uaddr+1] = APP_SIGNATURE_VALUE >> 8;

                if(!wzPRGWriting32word(g_flashRAMPosition, g_flashRAMBuffer.buf16)) {
                    wzEUSARTWriteLine(RETURN_STATUS_ERROR);
                    finish = bFalse;
                }
            }
            break;
        case 0x00: //Write
        {
            for(uint8_t i=0; i < packet->byteNum; i++) {//8bit=1byte
                if(u24adress + i < 0x2000)
                    continue;   //Boot Area
                
                uint24_t pscmp = g_flashRAMPosition & WIZ_PRG_FIRSTADDRESS;
                uint24_t psadress = (u24adress + i) & WIZ_PRG_FIRSTADDRESS;
                if(pscmp != psadress) {
                    //Need Flush
                    //Reload -> Flush
                    if(g_flashRAMPosition != PROGRAM_FLASH_NULL) {
                        wzPRGWriting32word(pscmp, g_flashRAMBuffer.buf16);
                    }
                    
                    //Reload
                    wzPRGReading32word(psadress, g_flashRAMBuffer.buf16);
                }

                g_flashRAMPosition = u24adress + i;
                
                //Write
                uint8_t uaddr = g_flashRAMPosition & WIZ_PRG_FIRSTADDRESS_N;
                g_flashRAMBuffer.buf8[uaddr] = wzStringToUint8(&packet->pdata[i<<1],2);   //2char = 1byte //<<1 = *2
            }
        }
            break;
        default:
            //nothing
            break;
    }
  
    return finish;
}


void startDownloader(void)
{
    wzTMR0IntTask(WIZ_TASK_NULL);   //OFF
    wzEUSARTWriteString(WIZROS_BOOTLOADER_MODE);
    wzEUSARTWriteString("-");
    wzEUSARTWriteLine(WIZROS_BOOTLOADER_VERSION);
    wzTaskEnqueue(taskloop_downloader);  //loop
}

#endif /*WIZROS_BOOTLOADER*/