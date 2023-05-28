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
 *  File Name : wizros_mssp.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_mssp.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
#define MSSP_INIT_SPI_MASTER     (0)
#define MSSP_INIT_SPI_SLAVE      (1)
#define MSSP_INIT_I2C_MASTER     (2)
#define MSSP_INIT_I2C_SLAVE      (3)

volatile WIZ_SSP_PACKET g_wzSSPBuff[WIZ_MSSP_RECEIVE_BUFSIZE];	//SSP Receive Buffer
volatile uint8_t g_wzSSPQ_front;
volatile uint8_t g_wzSSPQ_rear;
volatile uint8_t g_wzSSPQ_count;

#define MSSP_FRAME_DATA_W           (0x01)  //I2C
#define MSSP_FRAME_DATA_R           (0x02)
#define MSSP_FRAME_DATA_ACK         (0x03)
#define MSSP_FRAME_DATA_NACK        (0x04)
#define MSSP_FRAME_ADDRESS          (0x05)
#define MSSP_FRAME_STARTBIT         (0x06)
#define MSSP_FRAME_STOPBIT          (0x07)
#define MSSP_FRAME_RSENBIT          (0x08)
#define MSSP_SPI_COMMAND_READ       (0x10)  //SPI
#define MSSP_SPI_COMMAND_WRITE      (0x20)

volatile uint8_t g_wzSSPBufStatus;
volatile uint16_t g_wzSSPBufStatusErrorCount;
#define MSSP_BUSSTATUS_READY     (0x00)
#define MSSP_BUSSTATUS_BUSY      (0x01)
#define MSSP_BUSSTATUS_ERROR     (0xFF)

volatile uint16_t g_wzSSP_SlaveAddress;
volatile uint8_t* g_wzSSP_SlaveMemoryMap;

volatile taskfunc_t g_intMSSPTask;	//Task 
volatile uint8_t g_currentType;	//Type



//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzMSSPInitializeSPI(uint8_t type, uint8_t clock, uint8_t edge)
{
    SSPSTAT = 0;
    SSPCON1 = 0;
    SSPCON2 = 0;
    
    //Setup queue
    g_wzSSPQ_front = g_wzSSPQ_rear = g_wzSSPQ_count = 0;
    g_wzSSPBufStatus = MSSP_BUSSTATUS_READY;
    g_wzSSPBufStatusErrorCount = 0;
    
    g_wzSSP_SlaveAddress = 0xFFFF;
    g_wzSSP_SlaveMemoryMap = NULL;
    
    //Master mode or Sleve mode
    if(type == WIZ_MSSP_INIT_MASTER)
    {
        //Select the clock
        switch(clock)
        {
            case WIZ_MSSP_INIT_SPI_CLK16MHZ:
            default:
                SSPCON1bits.SSPM = 0b0000;
                break;
            case WIZ_MSSP_INIT_SPI_CLK8MHZ:
                SSPCON1bits.SSPM = 0b1010;
                break;
            case WIZ_MSSP_INIT_SPI_CLK4MHZ:
                SSPCON1bits.SSPM = 0b0001;
                break;
            case WIZ_MSSP_INIT_SPI_CLK1MHZ:
                SSPCON1bits.SSPM = 0b0010;
                break;
        }
        SSPSTATbits.SMP = 1;    //Input data sampled at the end of data output time
        
        PIE1bits.SSPIE = 0; //default 0
        PIR1bits.SSPIF = 0;
        
        PIE2bits.BCLIE = 0;
        PIR2bits.BCLIF = 0;
        
        //Pin setting
        wzGPIOPinInOutMode(WIZ_PINMODE_C3, WIZ_PINMODEINOUT_OUT);
        wzGPIOPinInOutMode(WIZ_PINMODE_C4, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C5, WIZ_PINMODEINOUT_OUT);
    
        g_currentType = MSSP_INIT_SPI_MASTER;
    }
    else
    {
        if(type == WIZ_MSSP_INIT_SLAVE_SPI_SS) {
            SSPCON1bits.SSPM = 0b0100;
            wzGPIOPinInOutMode(WIZ_PINMODE_C5, WIZ_PINMODEINOUT_IN); //Pin setting : IN
        }else
            SSPCON1bits.SSPM = 0b0101;
        SSPSTATbits.SMP = 0;
        
        //Start SSP interrupt
        PIE1bits.SSPIE = 1;
        PIR1bits.SSPIF = 0;
        
        //Pin setting
        wzGPIOPinInOutMode(WIZ_PINMODE_C3, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C4, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C5, WIZ_PINMODEINOUT_OUT);
        
        g_currentType = MSSP_INIT_SPI_SLAVE;
    }
    
    //Select the clock edge
    switch(edge)
    {
        case WIZ_MSSP_INIT_SPI_LOW_LH:
        default:
            SSPSTATbits.CKE = 0;    //Transmit occurs on transition from Idle to active clock state
            SSPCON1bits.CKP = 1;    //Idle state for clock is a high level
            break;
        case WIZ_MSSP_INIT_SPI_LOW_HL:
            SSPSTATbits.CKE = 0;    //Transmit occurs on transition from Idle to active clock state
            SSPCON1bits.CKP = 0;    //Idle state for clock is a low level
            break;
        case WIZ_MSSP_INIT_SPI_HIGH_LH:
            SSPSTATbits.CKE = 1;    //Transmit occurs on transition from active to Idle clock state
            SSPCON1bits.CKP = 1;    //Idle state for clock is a high level
            break;
        case WIZ_MSSP_INIT_SPI_HIGH_HL:
            SSPSTATbits.CKE = 1;    //Transmit occurs on transition from active to Idle clock state
            SSPCON1bits.CKP = 0;    //Idle state for clock is a low level
            break;
    }

    SSPCON1bits.SSPEN = 1;  //Enable SPI
    
    g_intMSSPTask = WIZ_TASK_NULL;
}

void wzMSSPInitializeI2C(uint8_t type, uint8_t mode, uint8_t myaddress)
{
    SSPSTAT = 0;
    SSPCON1 = 0;
    SSPCON2 = 0;
    
    //Setup queue
    g_wzSSPQ_front = g_wzSSPQ_rear = g_wzSSPQ_count = 0;
    g_wzSSPBufStatus = MSSP_BUSSTATUS_READY;
    g_wzSSPBufStatusErrorCount = 0;
    
    g_wzSSP_SlaveAddress = 0xFFFF;
    g_wzSSP_SlaveMemoryMap = NULL;
    
    //Master mode or Sleve mode
    if(type==WIZ_MSSP_INIT_MASTER)
    {
        SSPCON1bits.SSPM = 0b1000;
        
        PIE1bits.SSPIE = 1; //default 1
        PIR1bits.SSPIF = 0;
        
        PIE2bits.BCLIE = 0;
        PIR2bits.BCLIF = 0;
        
        switch(mode) {
            case WIZ_MSSP_INIT_I2C_100KHZ:
            default:
                SSPADD = 0x9F;
                SSPSTATbits.SMP = 1;    //100Khz
                break;
            case WIZ_MSSP_INIT_I2C_400KHZ:
                SSPADD = 0x27;
                SSPSTATbits.SMP = 0;    //400Khz
                break;
            case WIZ_MSSP_INIT_I2C_1000KHZ:
                SSPADD = 0x0F;
                SSPSTATbits.SMP = 1;    //1000Khz
                break;
        }
        
        //Pin setting
        wzGPIOPinInOutMode(WIZ_PINMODE_C3, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C4, WIZ_PINMODEINOUT_IN);
        
        g_currentType = MSSP_INIT_I2C_MASTER;
    }
    else
    {
        SSPCON1bits.SSPM = 0b0110;
        
        //Start SSP interrupt
        PIE1bits.SSPIE = 1;
        PIR1bits.SSPIF = 0;
        
        SSPADD = (myaddress << 1) & 0xFF;
        
        //Pin setting
        wzGPIOPinInOutMode(WIZ_PINMODE_C3, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C4, WIZ_PINMODEINOUT_IN);
        
        SSPSTATbits.SMP = 0;
        SSPCON2bits.SEN = 1;
        
        g_currentType = MSSP_INIT_I2C_SLAVE;
    }
    
    SSPSTATbits.CKE = 0;    //do not i2c
    SSPCON1bits.SSPEN = 1;  //Enable I2C
    
    SSPCON1bits.CKP = 1;    //Start
    
    g_intMSSPTask = WIZ_TASK_NULL;
}
void wzMSSPInitializeI2CM(uint8_t type, uint8_t mode)
{
    wzMSSPInitializeI2C(type, mode, 0x00);
}

#pragma warning disable 2053
static inline void wzMSSPTransmit_Main(uint8_t data, uint8_t cmd)
{ 
    //status wait
    g_wzSSPBufStatusErrorCount = 0xFFFF;
    while(g_wzSSPBufStatus == MSSP_BUSSTATUS_BUSY)
    {
        --g_wzSSPBufStatusErrorCount;
        if(g_wzSSPBufStatusErrorCount == 0)
            g_wzSSPBufStatus = MSSP_BUSSTATUS_ERROR;
    }
    
    if(g_wzSSPBufStatus == MSSP_BUSSTATUS_ERROR) {
        //ERROR
        return;
    }
    
    PIE1bits.SSPIE = 0;    
    //command
    if(cmd == MSSP_FRAME_STARTBIT) {
        SSPCON2bits.SEN = 1;    // Start Condition Enable bit
        while(SSPCON2bits.SEN == 1);
    } else if(cmd == MSSP_FRAME_RSENBIT) {
        SSPCON2bits.RSEN = 1;   //Recv req
        while(SSPCON2bits.RSEN == 1);
    } else if(cmd == MSSP_FRAME_STOPBIT) {
        SSPCON2bits.PEN = 1;    // Stop Condition Enable bit
        while(SSPCON2bits.PEN);
        g_wzSSPBufStatus = MSSP_BUSSTATUS_BUSY;
    } else if(cmd == MSSP_FRAME_DATA_R) {
        SSPCON2bits.RCEN = 1;   // Recv req
        while(SSPCON2bits.RCEN);
        g_wzSSPBufStatus = MSSP_BUSSTATUS_BUSY;
    } else if(cmd == MSSP_FRAME_DATA_ACK) {
        SSPCON2bits.ACKDT = 0;  //ACK
        SSPCON2bits.ACKEN = 1;
        g_wzSSPBufStatus = MSSP_BUSSTATUS_BUSY;
    } else if(cmd == MSSP_FRAME_DATA_NACK) {
        SSPCON2bits.ACKDT = 1;  //NOACK
        SSPCON2bits.ACKEN = 1;
        g_wzSSPBufStatus = MSSP_BUSSTATUS_BUSY;
    } else {
        do {    //SEND
            SSPCON1bits.WCOL = 0;
            SSPBUF = data;
        } while(SSPCON1bits.WCOL);  //collision

        if(cmd == MSSP_FRAME_ADDRESS)
            g_wzSSP_SlaveAddress = data;
        g_wzSSPBufStatus = MSSP_BUSSTATUS_BUSY;
    }
    
    PIE1bits.SSPIE = 1;
}

//for SPI
void wzMSSPTransmitSPI(uint8_t data)
{
    wzMSSPTransmit_Main(data, MSSP_SPI_COMMAND_WRITE);
}

void wzMSSPTransmitBufferSPI(uint8_t* datas, uint16_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++) {
        wzMSSPTransmit_Main(datas[i], MSSP_SPI_COMMAND_WRITE);
    }
}

void wzMSSPReadTransmitSPI(uint8_t data)
{
    wzMSSPTransmit_Main(data, MSSP_SPI_COMMAND_READ);
}

void wzMSSPReadTransmitBufferSPI(uint8_t* datas, uint16_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++) {
        wzMSSPTransmit_Main(datas[i], MSSP_SPI_COMMAND_READ);
    }
}

//-----------------------------------------------
//for I2C
void wzMSSPStartI2C(uint8_t address_rw)
{
    g_wzSSPBufStatus = MSSP_BUSSTATUS_READY;    //Reset
    
    wzMSSPTransmit_Main(0, MSSP_FRAME_STARTBIT); //Start
    wzMSSPTransmit_Main(address_rw, MSSP_FRAME_ADDRESS); //Address
}

void wzMSSPStopI2C()
{
    wzMSSPTransmit_Main(0, MSSP_FRAME_STOPBIT); //Stop
}

void wzMSSPRepeatStartI2C(uint8_t address_rw)
{
    wzMSSPTransmit_Main(0, MSSP_FRAME_RSENBIT); //Restart
    wzMSSPTransmit_Main(address_rw, MSSP_FRAME_ADDRESS); //Address
}

void wzMSSPTransmitI2C(uint8_t data)
{
    wzMSSPTransmit_Main(data, MSSP_FRAME_DATA_W);
}

void wzMSSPTransmitBufferI2C(uint8_t* datas, uint16_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++) {
        wzMSSPTransmit_Main(datas[i], MSSP_FRAME_DATA_W);
    }
}

void wzMSSPReadTransmitI2C(uint16_t len)
{
    uint8_t i;
    for(i = 0; i < len-1; i++) {
        wzMSSPTransmit_Main(0x00, MSSP_FRAME_DATA_R);
        wzMSSPTransmit_Main(0x00, MSSP_FRAME_DATA_ACK);
    }
    wzMSSPTransmit_Main(0x00, MSSP_FRAME_DATA_R);
    wzMSSPTransmit_Main(0x00, MSSP_FRAME_DATA_NACK);
}
 
uint8_t wzMSSPReadSlaveData(void)
{
    uint8_t data = 0x00;
    
    //Queue is empty
    if(g_wzSSPQ_count == 0) {
        return data;
    }
        
    //next
    PIE1bits.SSPIE = 0;
    
    uint8_t front = g_wzSSPQ_front++;
    --g_wzSSPQ_count;
    if(g_wzSSPQ_front == WIZ_MSSP_RECEIVE_BUFSIZE)
        g_wzSSPQ_front = 0;
   
    data = g_wzSSPBuff[front].Data;

    PIE1bits.SSPIE = 1;
    
    return data;
}

uint8_t wzMSSPReadSlaveDataRefAddr(uint8_t* refAddress)
{
    uint8_t data = 0x00;
    
    //Queue is empty
    if(g_wzSSPQ_count == 0) {
        return data;
    }
        
    //next
    PIE1bits.SSPIE = 0;
    
    uint8_t front = g_wzSSPQ_front++;
    --g_wzSSPQ_count;
    if(g_wzSSPQ_front == WIZ_MSSP_RECEIVE_BUFSIZE)
        g_wzSSPQ_front = 0;
   
    data = g_wzSSPBuff[front].Data;
    *refAddress = g_wzSSPBuff[front].Address;

    PIE1bits.SSPIE = 1;
    
    return data; 
}

uint8_t wzMSSPReadSlaveDataAvailable(void)
{
    return g_wzSSPQ_count;
}

void wzMSSPReadSlaveBuffer(uint8_t *buf, uint16_t len, bool_t pooling)
{
    uint16_t i;
    
    for(i = 0; i < len; i++) {
        if(pooling) while(g_wzSSPQ_count == 0);
        buf[i] = wzMSSPReadSlaveData();
    }
}

void wzMSSPReadSlaveBufferRefAddr(uint8_t *buf, uint8_t *addrBuf, uint16_t len, bool_t pooling)
{
    uint16_t i;
    
    for(i = 0; i < len; i++) {
        if(pooling) while(g_wzSSPQ_count == 0);
        buf[i] = wzMSSPReadSlaveDataRefAddr(&addrBuf[i]);
    }
}

void wzMSSPReadSlaveReqReply(uint8_t data)
{
    if(g_wzSSP_SlaveMemoryMap != NULL)
        return;
    
    if(g_currentType == MSSP_INIT_SPI_SLAVE || g_currentType == MSSP_INIT_I2C_SLAVE)
    {
        do {
            SSPCON1bits.WCOL = 0;
            SSPBUF = data;
        } while(SSPCON1bits.WCOL);  //collision
        
        if (SSPCON2bits.SEN) SSPCON1bits.CKP = 1;
    }
}

void wzMSSPSetSlaveMemoryPtI2C(uint8_t* memorymap)
{
    g_wzSSP_SlaveMemoryMap = memorymap;
}

void wzMSSPSetReadSlaveAddressI2C(uint8_t address)
{
    g_wzSSP_SlaveAddress = (uint16_t)address;
}
void wzMSSPResetReadSlaveAddressI2C(void)
{
    g_wzSSP_SlaveAddress = 0xFFFF;
}

void wzMSSPSlaveHandleInt(void)
{
    uint8_t data = 0x00;
    uint8_t dataStats = SSPSTAT & 0b00111101;
    bool_t bufferOK = bFalse;
    bool_t senderReqOK = bFalse;

    //Slave Read
    switch(g_currentType) {
        case MSSP_INIT_I2C_MASTER:
            // _, _, D/A, P, S, R/W, _, BF
            if(g_wzSSPBufStatus != MSSP_BUSSTATUS_BUSY)
                break;
            if(dataStats & 0b00010000) {  //Stopbit reset
                g_wzSSPBufStatus = MSSP_BUSSTATUS_READY;
                break;
            }
            
            if(dataStats & 0b00000101){  //Read & BF
                data = SSPBUF;
                bufferOK = bTrue;
                if (SSPCON1bits.SSPOV)
                    SSPCON1bits.SSPOV = 0; // Clear receive overflow indicator
            }
            
            if(SSPCON2bits.ACKSTAT){    //NACK
                SSPCON2bits.PEN = 1;
                while(SSPCON2bits.PEN);
                g_wzSSPBufStatus = MSSP_BUSSTATUS_ERROR;
            }
            else 
                g_wzSSPBufStatus = MSSP_BUSSTATUS_READY;
            
            break;
        case MSSP_INIT_I2C_SLAVE:
            // _, _, D/A, P, S, R/W, _, BF
            if(dataStats & 0b00010000)  //Stopbit
                break;
                        
            if(dataStats & 0b00001000) {    //startbit
                if(dataStats & 0b00000100) { //R or W
                    //Reader
                    if(dataStats & 0b00000001) {  //BF
                        //Data 
                        uint8_t addrDummy = SSPBUF;
                        senderReqOK = bTrue;
                        
                        if (SSPCON1bits.SSPOV)
                            SSPCON1bits.SSPOV = 0; // Clear receive overflow indicator
                    } else {
                        senderReqOK = bTrue;
                    }
                }
                else
                {   //Writer
                    if(dataStats & 0b00100000) {  //D or A
                        //Data bit & addressSet
                        data = SSPBUF;
                        if(g_wzSSP_SlaveAddress == 0xFFFF)
                            g_wzSSP_SlaveAddress = (uint16_t)data;
                        else
                            bufferOK = bTrue;
                    } else {   //Slave Address ScanMode
                        uint8_t addrDummy = SSPBUF;
                        g_wzSSP_SlaveAddress = 0xFFFF;  //Reset
                    }
                    
                    if (SSPCON1bits.SSPOV)
                        SSPCON1bits.SSPOV = 0;
                    
                    if (SSPCON2bits.SEN) SSPCON1bits.CKP = 1;
                }
            }
            break;
        case MSSP_INIT_SPI_MASTER:
            if(dataStats & 0b00000101){  //Read & BF
                data = SSPBUF;
                bufferOK = bTrue;
                if (SSPCON1bits.SSPOV)
                    SSPCON1bits.SSPOV = 0;
            }
            break;
        case MSSP_INIT_SPI_SLAVE:
            data = SSPBUF;
            if(g_wzSSP_SlaveAddress == 0xFFFF) {
                g_wzSSP_SlaveAddress = (uint16_t)data;
            } else {
                bufferOK = bTrue;
                senderReqOK = bTrue;
            }
            if (SSPCON1bits.SSPOV)
                SSPCON1bits.SSPOV = 0;
            break;
        default:
            return;
    }

    //Recver
    //Now add it to q
    if(bufferOK) {
        //Queue is not full
        if(g_wzSSP_SlaveMemoryMap == NULL) {
            if(g_wzSSPQ_count < WIZ_MSSP_RECEIVE_BUFSIZE) {
                g_wzSSPBuff[g_wzSSPQ_rear].Address = (uint8_t)(g_wzSSP_SlaveAddress & 0x00FF);
                g_wzSSPBuff[g_wzSSPQ_rear].Data = data;
                ++g_wzSSPQ_rear;

                ++g_wzSSPQ_count;
                if(g_wzSSPQ_rear == WIZ_MSSP_RECEIVE_BUFSIZE)
                    g_wzSSPQ_rear = 0;
            }
        }
        else
            g_wzSSP_SlaveMemoryMap[g_wzSSP_SlaveAddress & 0x00FF] = data;
        
        //Enqueue task
        if(g_intMSSPTask != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intMSSPTask);  //int
    }
    
    //Request Sender
    if(senderReqOK && g_wzSSP_SlaveAddress != 0xFFFF) {
        if(g_wzSSP_SlaveMemoryMap != NULL) {
            uint8_t sendData = g_wzSSP_SlaveMemoryMap[g_wzSSP_SlaveAddress & 0x00FF];
            do {
                SSPCON1bits.WCOL = 0;
                SSPBUF = sendData;
            } while(SSPCON1bits.WCOL);  //collision
            if (SSPCON2bits.SEN) SSPCON1bits.CKP = 1;
        }
    }
    
    if(senderReqOK || bufferOK)
        if(g_wzSSP_SlaveAddress != 0xFFFF)
            ++g_wzSSP_SlaveAddress;
}

void wzMSSPSlaveIntTask(taskfunc_t task)
{
    g_intMSSPTask = task;
}