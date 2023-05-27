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
 *  File Name : wizros_eusart.h
 *
***************************************************************************/

#ifndef _WIZROS_EUSART_H_
#define	_WIZROS_EUSART_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
// Receive buffer size
#define WIZ_EUSART_RECEIVE_BUFSIZE  (96)
#define WIZ_EUSART_SEND_BUFSIZE     (64)

// Baud rate flags
#define WIZ_EUSART_BRFLAG_9600      (0)
#define WIZ_EUSART_BRFLAG_14400     (1)
#define WIZ_EUSART_BRFLAG_19200     (2)
#define WIZ_EUSART_BRFLAG_38400     (3)
#define WIZ_EUSART_BRFLAG_57600     (4)
#define WIZ_EUSART_BRFLAG_115200    (5)
#define WIZ_EUSART_BRFLAG_230400    (6)
#define WIZ_EUSART_BRFLAG_500000    (7)
#define WIZ_EUSART_BRFLAG_1000000   (8)

// Config
#define WIZ_EUSART_CONF_NOPARITY        (0x00)
#define WIZ_EUSART_CONF_EVENPARITY      (0x01)
#define WIZ_EUSART_CONF_ODDPARITY       (0x02)
#define WIZ_EUSART_CONF_SPACEPARITY     (0x03)
#define WIZ_EUSART_CONF_MASKPARITY      (0x04)

#define WIZ_EUSART_CONF_DATA8BIT        (0x00)
#define WIZ_EUSART_CONF_DATA7BIT        (0x10)

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzEUSARTInitialize(uint8_t baud_rate_flag);    //, uint8_t config

void wzEUSARTWriteChar(char ch);
void wzEUSARTWriteString(const char *str);
void wzEUSARTWriteLine(const char *str);
void wzEUSARTWriteInt16(int16_t val, int8_t field_length);
void wzEUSARTWriteUInt16(uint16_t val, int8_t field_length);
void wzEUSARTWriteInt32(int32_t val, int8_t field_length);
void wzEUSARTWriteUInt32(uint32_t val, int8_t field_length);
void wzEUSARTWriteNewLine(void);
void wzEUSARTWriteBytes(uint8_t *data, uint16_t len);
void wzEUSARTWriteEndWait(void);

//Printf stdio.h , heavy program
#define wzUSARTWritePrintf(str, c) printf(str, c)

void wzEUSARTEndOfLineUsingCR(bool_t useCR);
void wzEUSARTBreakSignal(void);

uint8_t wzEUSARTReadData(void);
void wzEUSARTReadBuffer(uint8_t *buff, uint16_t len);

uint8_t wzEUSARTDataAvailable(void);
void wzEUSARTFlushBuffer(void);

void wzEUSARTHandleRxInt(void);
void wzEUSARTIntRxTask(taskfunc_t task);
void wzEUSARTHandleTxInt(void);
void wzEUSARTIntTxTask(taskfunc_t task);

#endif /*_WIZROS_EUSART_H_*/