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
 *  File Name : wizros_mssp.h
 *
***************************************************************************/

#ifndef _WIZROS_MSSP_H_
#define	_WIZROS_MSSP_H_

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros.h"

//-------------------------------------------------------------------------
//  Defines
//-------------------------------------------------------------------------
// Receive buffer size
#define WIZ_MSSP_RECEIVE_BUFSIZE  (96)
#define WIZ_MSSP_SEND_BUFSIZE     (64)

//define
#define WIZ_MSSP_INIT_MASTER        (0)
#define WIZ_MSSP_INIT_SLAVE         (1)
#define WIZ_MSSP_INIT_SLAVE_SPI_SS  (2)

//for wzMSSPInitializeSPI
#define WIZ_MSSP_INIT_SPI_LOW_LH    (0)
#define WIZ_MSSP_INIT_SPI_LOW_HL    (1)
#define WIZ_MSSP_INIT_SPI_HIGH_LH   (2)
#define WIZ_MSSP_INIT_SPI_HIGH_HL   (3)

#define WIZ_MSSP_INIT_SPI_CLK16MHZ 	(0)
#define WIZ_MSSP_INIT_SPI_CLK8MHZ 	(1)
#define WIZ_MSSP_INIT_SPI_CLK4MHZ	(2)
#define WIZ_MSSP_INIT_SPI_CLK1MHZ   (3)

//for wzMSSPInitializeI2C
#define WIZ_MSSP_INIT_I2C_100KHZ    (0)
#define WIZ_MSSP_INIT_I2C_400KHZ    (1)
#define WIZ_MSSP_INIT_I2C_1000KHZ   (2)

//-------------------------------------------------------------------------
//  Struct
//-------------------------------------------------------------------------

//Can packet
typedef struct {
    uint8_t Address;
    uint8_t Data;
} WIZ_SSP_PACKET;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzMSSPInitializeSPI(uint8_t type, uint8_t clock, uint8_t edge); //wzMSSPInitializeSPI or wzMSSPInitializeI2C
void wzMSSPInitializeI2C(uint8_t type, uint8_t mode, uint8_t myaddress); //myaddress is WIZ_MSSP_INIT_SLAVE only
void wzMSSPInitializeI2CM(uint8_t type, uint8_t mode);

//SPI Mode
void wzMSSPTransmitSPI(uint8_t data);
void wzMSSPTransmitBufferSPI(uint8_t* datas, uint16_t len);
void wzMSSPReadTransmitSPI(uint8_t data);
void wzMSSPReadTransmitBufferSPI(uint8_t* datas, uint16_t len);

//I2C Mode
void wzMSSPStartI2C(uint8_t address_rw);
void wzMSSPRepeatStartI2C(uint8_t address_rw);
void wzMSSPStopI2C(void);
void wzMSSPTransmitI2C(uint8_t data);
void wzMSSPTransmitBufferI2C(uint8_t* datas, uint16_t len);
void wzMSSPReadTransmitI2C(uint16_t len);

//Read
uint8_t wzMSSPReadSlaveData(void);
uint8_t wzMSSPReadSlaveDataRefAddr(uint8_t* refAddress);
uint8_t wzMSSPReadSlaveDataAvailable();
void wzMSSPReadSlaveBuffer(uint8_t *buf, uint16_t len, bool_t pooling);
void wzMSSPReadSlaveBufferRefAddr(uint8_t *buf, uint8_t *addrBuf, uint16_t len, bool_t pooling);
void wzMSSPReadSlaveReqReply(uint8_t data);

void wzMSSPSetSlaveMemoryPtI2C(uint8_t* memorymap); //MemoryMode
void wzMSSPSetReadSlaveAddressI2C(uint8_t address);
void wzMSSPResetReadSlaveAddressI2C();
//int
void wzMSSPSlaveHandleInt(void);
void wzMSSPSlaveIntTask(taskfunc_t task);

#endif /*_WIZROS_MSSP_H_*/


