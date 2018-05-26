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
 *  File Name : wizros_mssp.c
 *
***************************************************************************/

/* ※SPI,I2Cのデータ読み出し要求については対応しておりません。今後対応予定*/

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

volatile uint8_t g_wzSSPBuff[WIZ_MSSP_RECEIVE_BUFSIZE];	//SSP Receive Buffer
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

volatile uint8_t g_wzSSPBUFCommand;
volatile uint16_t g_wzSSPSendBuff[WIZ_MSSP_SEND_BUFSIZE];	//SSP Send Buffer HMB:Command,LMB:Data
volatile uint8_t g_wzSSPSendQ_front;
volatile uint8_t g_wzSSPSendQ_rear;
volatile uint8_t g_wzSSPSendQ_count;

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
    g_wzSSPSendQ_front = g_wzSSPSendQ_rear = g_wzSSPSendQ_count = 0;
    g_wzSSPBUFCommand = 0;
    
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
    g_wzSSPSendQ_front = g_wzSSPSendQ_rear = g_wzSSPSendQ_count = 0;
    g_wzSSPBUFCommand = 0;
    
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
        
        SSPADD = myaddress;
        
        //Pin setting
        wzGPIOPinInOutMode(WIZ_PINMODE_C3, WIZ_PINMODEINOUT_IN);
        wzGPIOPinInOutMode(WIZ_PINMODE_C4, WIZ_PINMODEINOUT_IN);
        
        SSPSTATbits.SMP = 0;
        
        g_currentType = MSSP_INIT_I2C_SLAVE;
    }
    
    SSPSTATbits.CKE = 0;    //do not i2c
    SSPCON1bits.SSPEN = 1;  //Enable I2C
    
    SSPCON1bits.CKP = 1;    //Start
    
    g_intMSSPTask = WIZ_TASK_NULL;
}

static inline void wzMSSPTransmit_Main(uint8_t data, uint8_t cmd)
{
    //first
    if(g_wzSSPBUFCommand == 0) {
        g_wzSSPBUFCommand = cmd;

        if(cmd == MSSP_FRAME_STARTBIT) {
            SSPCON2bits.SEN = 1;    // Start Condition Enable bit
        } else if(cmd == MSSP_FRAME_STOPBIT) {
            SSPCON2bits.PEN = 1;    // Stop Condition Enable bit
        } else if(cmd == MSSP_FRAME_DATA_R) {
            SSPCON2bits.RCEN = 1;   // Recv req
        } else if(cmd == MSSP_FRAME_DATA_ACK) {
            SSPCON2bits.ACKDT = 0;  // ACK
            SSPCON2bits.ACKEN = 1;
        } else if(cmd == MSSP_FRAME_DATA_NACK) {
            SSPCON2bits.ACKDT = 1;  // NOACK
            SSPCON2bits.ACKEN = 1;
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_RSENBIT) {
            SSPCON2bits.RSEN = 1;   //Recv req
        } else {
            SSPBUF = data;
        }
        return;
    }
    

    //Queue is full
    if(g_wzSSPSendQ_count == WIZ_MSSP_SEND_BUFSIZE) {
        return; //fall
    }
    
    PIE1bits.SSPIE = 0;
    
    g_wzSSPSendBuff[g_wzSSPSendQ_rear] = (uint16_t)data;
    g_wzSSPSendBuff[g_wzSSPSendQ_rear] |= (uint16_t)cmd << 8;
    ++g_wzSSPSendQ_rear;
    ++g_wzSSPSendQ_count;
    
    if(g_wzSSPSendQ_rear == WIZ_MSSP_SEND_BUFSIZE)
        g_wzSSPSendQ_rear = 0;

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

//In master mode, it cannot use. 
uint8_t wzMSSPReadSlaveData()
{
    uint8_t data;

    //Queue is empty
    if(g_wzSSPQ_count == 0) {
        return 0;
    }
        
    //next
    PIE1bits.SSPIE = 0;
    
    uint8_t front = g_wzSSPQ_front++;
    --g_wzSSPQ_count;
    if(g_wzSSPQ_front == WIZ_MSSP_RECEIVE_BUFSIZE)
        g_wzSSPQ_front = 0;
   
    data = g_wzSSPBuff[front];

    PIE1bits.SSPIE = 1;
    
    return data;
}

uint8_t wzMSSPReadSlaveDataAvailable()
{
    return g_wzSSPQ_count;
}

//In master mode, it cannot use. 
void wzMSSPReadSlaveBuffer(uint8_t *buf, uint16_t len, bool_t pooling)
{
    uint16_t i;
    
    for(i = 0; i < len; i++) {
        if(pooling) while(g_wzSSPQ_count == 0);
        buf[i] = wzMSSPReadSlaveData();
    }
}

//In master mode, it cannot use. 
void wzMSSPSlaveHandleInt()
{
    uint8_t data;
    bool_t bufferOK = bFalse;

    //Slave Read
    switch(g_currentType) {
        case MSSP_INIT_I2C_MASTER:
            if(g_wzSSPBUFCommand == MSSP_FRAME_DATA_R) { //read
                data = SSPBUF;
                bufferOK = bTrue;
            }
            
            break;
        case MSSP_INIT_I2C_SLAVE:
            if((SSPSTAT & 0b00111000) == 0) { //no address and start pit and stop bit
                data = SSPBUF;
                bufferOK = bTrue;
            }
            break;
        case MSSP_INIT_SPI_MASTER:
            data = SSPBUF;
            if(g_wzSSPBUFCommand == MSSP_SPI_COMMAND_READ) bufferOK = bTrue;
            break;
        case MSSP_INIT_SPI_SLAVE:
            data = SSPBUF;
            bufferOK = bTrue;
            break;
        default:
            return;
    }

    //Now add it to q
    if(bufferOK) {
        //Queue is not full
        if(g_wzSSPQ_count < WIZ_MSSP_RECEIVE_BUFSIZE) { 
            g_wzSSPBuff[g_wzSSPQ_rear++] = data;
            ++g_wzSSPQ_count;
            if(g_wzSSPQ_rear == WIZ_MSSP_RECEIVE_BUFSIZE)
                g_wzSSPQ_rear = 0;
        }
        
        //Enqueue task
        if(g_intMSSPTask != WIZ_TASK_NULL)
            wzTaskEnqueueForInt(g_intMSSPTask);  //int
    }

    //Master Mode
    if(g_currentType == MSSP_INIT_SPI_MASTER || g_currentType == MSSP_INIT_I2C_MASTER)
    {
        //Next Data

        //Queue is empty
        if(g_wzSSPSendQ_count == 0) {
            g_wzSSPBUFCommand = 0;
            return;
        }
        
        //next
        uint8_t front = g_wzSSPSendQ_front++;
        --g_wzSSPSendQ_count;
        if(g_wzSSPSendQ_front == WIZ_MSSP_SEND_BUFSIZE)
            g_wzSSPSendQ_front = 0;
        
        g_wzSSPBUFCommand = (uint8_t)(g_wzSSPSendBuff[front] >> 8);
        if(g_wzSSPBUFCommand == MSSP_FRAME_STARTBIT) {
            SSPCON2bits.SEN = 1;    // Start Condition Enable bit
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_STOPBIT) {
            SSPCON2bits.PEN = 1;    // Stop Condition Enable bit
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_DATA_R) {
            SSPCON2bits.RCEN = 1;   //Recv req
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_DATA_ACK) {
            SSPCON2bits.ACKDT = 0; // ACK
            SSPCON2bits.ACKEN = 1;
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_DATA_NACK) {
            SSPCON2bits.ACKDT = 1; // NOACK
            SSPCON2bits.ACKEN = 1;
        } else if(g_wzSSPBUFCommand == MSSP_FRAME_RSENBIT) {
            SSPCON2bits.RSEN = 1;   //Recv req
        } else {
            SSPBUF = (uint8_t)(g_wzSSPSendBuff[front] & 0x00FF);
        }
    }
    
    if(g_currentType == MSSP_INIT_I2C_SLAVE) SSPCON1bits.CKP = 1;
}

void wzMSSPSlaveIntTask(taskfunc_t task)
{
    g_intMSSPTask = task;
}