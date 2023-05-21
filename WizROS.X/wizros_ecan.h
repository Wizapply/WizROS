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
 *  File Name : wizros_ecan.h
 *
***************************************************************************/

#ifndef _WIZROS_ECAN_H_
#define	_WIZROS_ECAN_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
// Receive buffer size
#define WIZ_ECAN_RECEIVE_BUFSIZE  (32)  //WIZ_CAN_PACKET size
#define WIZ_ECAN_SEND_BUFSIZE     (24)  //WIZ_CAN_PACKET size

//Can Operation
#define WIZ_ECAN_NORMAL_MODE    (0b000)
#define WIZ_ECAN_LOOPBACK_MODE  (0b010)
#define WIZ_ECAN_DISABLE_MODE   (0b001)
#define WIZ_ECAN_LISTEN_ONLY    (0b011)
#define WIZ_ECAN_CONFIG_MODE    (0b100)

//Receive modes
#define WIZ_ECAN_RECEIVE_ALL    (0)     //Receive all messages (including those with errors); filter criteria is ignored
#define WIZ_ECAN_VALID_MSG_EID  (1)     //Receive only valid messages with extended identifier; EXIDEN in RXFnSIDL must be ‘1’
#define WIZ_ECAN_VALID_MSG_SID  (2)     //Receive only valid messages with standard identifier, EXIDEN in RXFnSIDL must be ‘0’
#define WIZ_ECAN_ALL_VALID_MSG  (3)     //Receive all valid messages as per EXIDEN bit in RXFnSIDL register

//BAUD RATE modes
#define WIZ_ECAN_BAND_125K      (0)     //Low Mode
#define WIZ_ECAN_BAND_250K      (1)
#define WIZ_ECAN_BAND_500K      (2)
#define WIZ_ECAN_BAND_1000K     (3)

//Time out wait
#define WIZ_ECAN_TOWAIT         (30)

//-------------------------------------------------------------------------
//  Struct
//-------------------------------------------------------------------------

//Can packet
typedef struct {
    bool_t  isRemote;
    uint8_t D0;
    uint8_t D1;
    uint8_t D2;
    uint8_t D3;
    uint8_t D4;
    uint8_t D5;
    uint8_t D6;
    uint8_t D7;
    
    uint8_t dlc;
    
    //EX
    bool_t isEXID;
    union {
        uint16_t sid; //11bit, In case of EXID, do not use it.
        uint32_t eid; //29bit
    } id;
} WIZ_CAN_PACKET;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzECANInitialize(uint8_t opeMode, uint8_t recvMode, uint8_t baudrate);
void wzECANSetFilterRXB0(uint16_t sid, uint16_t mask);
void wzECANSetFilterRXB1(uint16_t sid, uint16_t mask);
void wzECANSetFilter(uint16_t sid, uint16_t mask);
void wzECANSetFilterRXB0EX(uint32_t eid, uint32_t emask);
void wzECANSetFilterRXB1EX(uint32_t eid, uint32_t emask);
void wzECANSetFilterEX(uint32_t eid, uint32_t emask);

//Write
void wzECANWrite(WIZ_CAN_PACKET* packet);
void wzECANWriteBuffer(WIZ_CAN_PACKET* packets, uint16_t len);
void wzECANWriteAbort();
void wzECANWriteRemote(uint16_t send_sid);
void wzECANWriteRemoteEX(uint32_t send_eid);
void wzECANWriteEndWait();

//Read
bool_t wzECANReadData(WIZ_CAN_PACKET* data);
void wzECANReadBuffer(WIZ_CAN_PACKET* buff, uint16_t len);

uint8_t wzECANDataAvailable(void);
void wzECANFlushBuffer(void);

//Error counter
uint8_t wzECANWriteErrorCount(void);
uint8_t wzECANReadErrorCount(void);

void wzECANHandleRxInt(void); //RXB0 or RXB1
void wzECANIntRxTask(taskfunc_t task);

void wzECANHandleTxInt(void); //TXB0
void wzECANHandleTxErrInt(void); //TXB0 Error
void wzECANIntTxTask(taskfunc_t task);
void wzECANIntTxErrorTask(taskfunc_t task);

#endif /*_WIZROS_ECAN_H_*/