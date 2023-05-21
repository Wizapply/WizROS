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
 *  File Name : wizros_ecan.c
 *
***************************************************************************/


//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_ecan.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile WIZ_CAN_PACKET g_wzCANBuff[WIZ_ECAN_RECEIVE_BUFSIZE];	//USART Receive Buffer
volatile uint8_t g_wzCANQ_front;
volatile uint8_t g_wzCANQ_rear;
volatile uint8_t g_wzCANQ_count;

volatile WIZ_CAN_PACKET g_wzCANSendBuff[WIZ_ECAN_SEND_BUFSIZE];	//USART Send Buffer
volatile uint8_t g_wzCANSendQ_front;
volatile uint8_t g_wzCANSendQ_rear;
volatile uint8_t g_wzCANSendQ_count;

volatile taskfunc_t g_intECANTxTask;	//Task
volatile taskfunc_t g_intECANTxErrorTask;
volatile taskfunc_t g_intECANRxTask;

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzECANInitialize(uint8_t opeMode, uint8_t recvMode, uint8_t baudrate)
{
    //Pin setting
    wzGPIOPinInOutMode(WIZ_PINMODE_B3, WIZ_PINMODEINOUT_IN);   //RX = In
    wzGPIOPinInOutMode(WIZ_PINMODE_B2, WIZ_PINMODEINOUT_OUT);  //TX = Out
    
    TXB0CON = 0x00;
    
    //Clear queue
    g_wzCANQ_front = g_wzCANQ_rear = g_wzCANQ_count = 0;
    g_wzCANSendQ_front = g_wzCANSendQ_rear = g_wzCANSendQ_count = 0;

    //Ensure ECAN is in config mode
    CANCONbits.REQOP = WIZ_ECAN_CONFIG_MODE;
    while(CANSTATbits.OPMODE != WIZ_ECAN_CONFIG_MODE); //config mode Wait

    //Set module mode
    ECANCONbits.MDSEL = 0b00;   // Legacy mode (Mode 0, default)
    ECANCONbits.FIFOWM = 0;     // Will cause FIFO interrupt when four receive buffers remain
    ECANCONbits.EWIN = 0;       // Unimplemented: Read as ‘0’

    //Set receive mode rxbuffer RXB0CON & RXB1CON
    RXB0CON = 0x00;
    RXB1CON = 0x00;
    
    //Set Filter
    RXFCON0 = 0;
    RXFCON1 = 0;
    
    //Receive Buffer 0 overflow will write to Receive Buffer 1
    RXB0CONbits.RB0DBEN = 1;
    
    switch(recvMode){
        case WIZ_ECAN_RECEIVE_ALL:
        default:
            RXB0CON |= 0b01100000;
            RXB1CON |= 0b01100000;
            break;
        case WIZ_ECAN_VALID_MSG_EID:
            RXB0CON |= 0b01000000;
            RXB1CON |= 0b01000000;
            break;
        case WIZ_ECAN_VALID_MSG_SID:
            RXB0CON |= 0b00100000;
            RXB1CON |= 0b00100000;
            break;
        case WIZ_ECAN_ALL_VALID_MSG:
            RXB0CON |= 0b00000000;
            RXB1CON |= 0b00000000;
            break;
    }
    
    //Enable Drive High bit
    CIOCON = 0x00;
    CIOCONbits.ENDRHI  = 1; //CANTX pin will drive VDD when recessive
    CIOCONbits.CLKSEL = 1; // Use the oscillator as the source of the CAN system clock

    //Clear
    BRGCON1 = 0x00;
    BRGCON2 = 0x00;
    BRGCON3 = 0x00;
    
    BRGCON2bits.SEG2PHTS = 1;   // Freely programmable
    BRGCON2bits.SAM = 1;        // Bus line is sampled three times prior to the sample point
    BRGCON3bits.WAKDIS = 1;     // Disable CAN bus activity wake-up feature
    BRGCON3bits.WAKFIL = 0;     // CAN bus line filter is not used for wake-up
    
    //Switch baud rate : 64MHz / 4 : Tq=8
    switch(baudrate){
        case WIZ_ECAN_BAND_125K:
        default:
            BRGCON1bits.BRP = 7;
            break;
        case WIZ_ECAN_BAND_250K:
            BRGCON1bits.BRP = 3;
            break;
        case WIZ_ECAN_BAND_500K:
            BRGCON1bits.BRP = 1;
            break;
        case WIZ_ECAN_BAND_1000K:
            BRGCON1bits.BRP = 0;
            break;
    }
    BRGCON2bits.PRSEG  = 0b010; // Phase Segment 2 time = 3 x TQ
    BRGCON2bits.SEG1PH = 0b001;  // Phase Segment 1 time = 2 x TQ
    BRGCON3bits.SEG2PH = 0b001;  // Phase Segment 2 time = 2 x TQ
    BRGCON1bits.SJW    = 0b00;   // Synchronization jump width time = 1 x TQ
    
    // Set filter & mask default
    wzECANSetFilterRXB0(0, 0);   // EVEN addresses
    wzECANSetFilterRXB1(0, 0);   // ODD addresses
    
    g_intECANRxTask = WIZ_TASK_NULL;
    g_intECANTxTask = WIZ_TASK_NULL;
    g_intECANTxErrorTask = WIZ_TASK_NULL;
    
    //Start interupt CAN Buffeur
    IPR5bits.RXB0IP = 0; // Low priority
    PIE5bits.RXB0IE = 1; // Interrupt enabled
    PIR5bits.RXB0IF = 0; // Clear CAN Receive Buffer 0 Interrupt Flag

    IPR5bits.RXB1IP = 0; // Low priority
    PIE5bits.RXB1IE = 1; // Interrupt enabled
    PIR5bits.RXB1IF = 0; // Clear CAN Receive Buffer 1 Interrupt Flag
    
    PIE5bits.ERRIE = 1;  // Error Interrupt Enable
    
    // Switch CAN module to normal mode    
    CANCONbits.REQOP = opeMode;           // Request normal mode
    while (CANSTATbits.OPMODE != opeMode);// Wait until CAN modul enters to normal mode
}

void wzECANSetFilterRXB0(uint16_t sid, uint16_t mask)
{
    RXF0SIDH = (sid >> 3) & 0xFF;    // filter 0, mask 0 -> RXB0
    RXF0SIDL = (sid << 5) & 0xFF;
    RXF1SIDH = (sid >> 3) & 0xFF;    // filter 1, mask 0 -> RXB0
    RXF1SIDL = (sid << 5) & 0xFF;
    
    RXM0SIDH = (mask >> 3) & 0xFF;
    RXM0SIDL = (mask << 5) & 0xFF;
    RXM0SIDLbits.EXIDEN = 0;
    RXFCON0bits.RXF0EN = 1; // filter 0 enable
    RXFCON0bits.RXF1EN = 1; // filter 1 enable
}

void wzECANSetFilterRXB1(uint16_t sid, uint16_t mask)
{
    RXF2SIDH = (sid >> 3) & 0xFF;    // filter 2, mask 1 -> RXB1
    RXF2SIDL = (sid << 5) & 0xFF;
    RXF3SIDH = (sid >> 3) & 0xFF;    // filter 3, mask 1 -> RXB1
    RXF3SIDL = (sid << 5) & 0xFF;
    RXF4SIDH = (sid >> 3) & 0xFF;    // filter 4, mask 1 -> RXB1
    RXF4SIDL = (sid << 5) & 0xFF;
    RXF5SIDH = (sid >> 3) & 0xFF;    // filter 5, mask 1 -> RXB1
    RXF5SIDL = (sid << 5) & 0xFF;
    
    RXM1SIDH = (mask >> 3) & 0xFF;
    RXM1SIDL = (mask << 5) & 0xFF;
    RXM1SIDLbits.EXIDEN = 0;
    RXFCON0bits.RXF2EN = 1; // filter 2 enable
    RXFCON0bits.RXF3EN = 1; // filter 3 enable
    RXFCON0bits.RXF4EN = 1; // filter 4 enable
    RXFCON0bits.RXF5EN = 1; // filter 5 enable
}

void wzECANSetFilterRXB0EX(uint32_t eid, uint32_t emask)
{
    RXF0SIDH = (eid >> 21) & 0xFF;    // filter 0, mask 0 -> RXB0
    RXF0SIDL = (eid >> 13) & 0xE0;
    RXF0SIDL |= ((eid >> 16) & 0x03) | 0x08;    //ex
    RXF0EIDH = (eid >> 8) & 0xFF;
    RXF0EIDL = eid & 0xFF;
    
    RXF1SIDH = (eid >> 21) & 0xFF;    // filter 1, mask 0 -> RXB0
    RXF1SIDL = (eid >> 13) & 0xE0;
    RXF1SIDL |= ((eid >> 16) & 0x03) | 0x08;   //ex
    RXF1EIDH = (eid >> 8) & 0xFF;
    RXF1EIDL = eid & 0xFF;
    
    RXM0SIDH = (emask >> 21) & 0xFF;    //mask
    RXM0SIDL = (emask >> 13) & 0xE0;
    RXM0SIDL |= ((emask >> 16) & 0x03) | 0x08;   //ex
    RXM0EIDH = (emask >> 8) & 0xFF;
    RXM0EIDL = emask & 0xFF;
    
    RXFCON0bits.RXF0EN = 1; // filter 0 enable
    RXFCON0bits.RXF1EN = 1; // filter 1 enable
}

void wzECANSetFilterRXB1EX(uint32_t eid, uint32_t emask)
{
    RXF2SIDH = (eid >> 21) & 0xFF;    // filter 2, mask 1 -> RXB1
    RXF2SIDL = (eid >> 13) & 0xE0;
    RXF2SIDL |= ((eid >> 16) & 0x03) | 0x08;   //ex
    RXF2EIDH = (eid >> 8) & 0xFF;
    RXF2EIDL = eid & 0xFF;
    RXF3SIDH = (eid >> 21) & 0xFF;    // filter 3, mask 1 -> RXB1
    RXF3SIDL = (eid >> 13) & 0xE0;
    RXF3SIDL |= ((eid >> 16) & 0x03) | 0x08;   //ex
    RXF3EIDH = (eid >> 8) & 0xFF;
    RXF3EIDL = eid & 0xFF;
    RXF4SIDH = (eid >> 21) & 0xFF;    // filter 4, mask 1 -> RXB1
    RXF4SIDL = (eid >> 13) & 0xE0;
    RXF4SIDL |= ((eid >> 16) & 0x03) | 0x08;   //ex
    RXF4EIDH = (eid >> 8) & 0xFF;
    RXF4EIDL = eid & 0xFF;
    RXF5SIDH = (eid >> 21) & 0xFF;    // filter 5, mask 1 -> RXB1
    RXF5SIDL = (eid >> 13) & 0xE0;
    RXF5SIDL |= ((eid >> 16) & 0x03) | 0x08;   //ex
    RXF5EIDH = (eid >> 8) & 0xFF;
    RXF5EIDL = eid & 0xFF;
   
    RXM1SIDH = (emask >> 21) & 0xFF;    //mask
    RXM1SIDL = (emask >> 13) & 0xE0;
    RXM1SIDL |= ((emask >> 16) & 0x03) | 0x08;    //ex en
    RXM1EIDH = (emask >> 8) & 0xFF;
    RXM1EIDL = emask & 0xFF;
    
    RXFCON0bits.RXF2EN = 1; // filter 2 enable
    RXFCON0bits.RXF3EN = 1; // filter 3 enable
    RXFCON0bits.RXF4EN = 1; // filter 4 enable
    RXFCON0bits.RXF5EN = 1; // filter 5 enable
}

void wzECANSetFilter(uint16_t sid, uint16_t mask)
{
    wzECANSetFilterRXB0(sid, mask);   // EVEN addresses
    wzECANSetFilterRXB1(sid, mask);   // ODD addresses
}

void wzECANSetFilterEX(uint32_t eid, uint32_t emask)
{
    wzECANSetFilterRXB0EX(eid, emask);   // EVEN addresses
    wzECANSetFilterRXB1EX(eid, emask);   // ODD addresses
}

void wzECANSetMode(uint8_t opeMode)
{
    // Switch CAN module to normal mode    
    CANCONbits.REQOP = opeMode;           // Request mode
    while (CANSTATbits.OPMODE != opeMode);// Wait until CAN modul enters to normal mode
}

//Write 
void wzECANWrite(WIZ_CAN_PACKET* packet)
{
    //First
    if(PIE5bits.TXB0IE == 0) {

        PIE5bits.RXB0IE = 0; // Read Interrupt disable
        PIE5bits.RXB1IE = 0;

        CANCON &= 0b11110001;
        CANCON |= 0b00001000;   //Transmit Buffer 0

        //Data Set
        TXB0DLC = packet->dlc;
        TXB0SIDH = (packet->id.sid >> 3) & 0x07;
        TXB0SIDL = (packet->id.sid << 5) & 0xE0;

        if(packet->isEXID) {
            //EX format
            uint32_t eid = packet->id.eid;

            TXB0SIDH = (eid >> 21) & 0xFF;
            TXB0SIDL = (eid >> 13) & 0xE0;
            TXB0SIDL |= (eid >> 16) & 0x03;
            TXB0EIDH = (eid >> 8) & 0xFF;
            TXB0EIDL = eid & 0xFF;

            TXB0SIDLbits.EXIDE = 1;
            TXB0SIDLbits.SRR = 1;
        }

        if(packet->isRemote) {
            TXB0DLC = 0x00;
            TXB0DLCbits.TXRTR = 1;
        }
        else
        {
            TXB0D0 = packet->D0;   //data
            TXB0D1 = packet->D1;
            TXB0D2 = packet->D2;
            TXB0D3 = packet->D3;
            TXB0D4 = packet->D4;
            TXB0D5 = packet->D5;
            TXB0D6 = packet->D6;
            TXB0D7 = packet->D7;   
        }

        PIE5bits.TXB0IE = 1;

        PIE5bits.RXB0IE = 1; // Read Interrupt enable
        PIE5bits.RXB1IE = 1;

        //Transmission
        TXB0CONbits.TXREQ = 1;

        return;
    }
    
    PIE5bits.TXB0IE = 0;
    
    //Send buffer enqueue
    g_wzCANSendBuff[g_wzCANSendQ_rear++] = *packet; //copy
    ++g_wzCANSendQ_count;
    if(g_wzCANSendQ_rear == WIZ_ECAN_SEND_BUFSIZE)
        g_wzCANSendQ_rear = 0;
    
    PIE5bits.TXB0IE = 1;
}

void wzECANWriteBuffer(WIZ_CAN_PACKET* packets, uint16_t len)
{
    uint16_t i;
    
    for(i = 0 ; i < len; ++i) {
        wzECANWrite(&packets[i]);
    }
}

void wzECANWriteAbort()
{
    //Abort
    PIE5bits.TXB0IE = 0;
    PIR5bits.TXB0IF = 0;
    CANCONbits.ABAT = 1;
    g_wzCANSendQ_front = g_wzCANSendQ_rear = g_wzCANSendQ_count = 0;
    
}

//Remote
void wzECANWriteRemote(uint16_t send_sid)
{
    WIZ_CAN_PACKET packet;
    packet.isRemote = bTrue;
    packet.dlc = 0;
    packet.id.sid = send_sid;
    
    wzECANWrite(&packet);
}

//Remote
void wzECANWriteRemoteEx(uint32_t send_eid)
{
    WIZ_CAN_PACKET packet;
    packet.isRemote = bTrue;
    packet.isEXID = 1;
    packet.dlc = 0;
    packet.id.eid = send_eid;
    
    wzECANWrite(&packet);
}

void wzECANWriteEndWait()
{
    while(PIE5bits.TXB0IE);
    while(TXB0CONbits.TXREQ);
}

//Read
bool_t wzECANReadData(WIZ_CAN_PACKET* data)
{
    //Queue is empty
    if(g_wzCANQ_count == 0) {
        return bFalse;
    }
    
    PIE5bits.RXB0IE = 0; // Interrupt disable
    PIE5bits.RXB1IE = 0;

    uint8_t front = g_wzCANQ_front++;
    --g_wzCANQ_count;
    if(g_wzCANQ_front == WIZ_ECAN_RECEIVE_BUFSIZE)
        g_wzCANQ_front = 0;
   
    if(data != 0)
        (*data) = g_wzCANBuff[front];
    
    PIE5bits.RXB0IE = 1; // Interrupt enabled
    PIE5bits.RXB1IE = 1;
    
    return bTrue;
}

void wzECANReadBuffer(WIZ_CAN_PACKET* buff, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++) {
        if(wzECANReadData(&buff[i]) == bFalse)
            return;
    }
}

uint8_t wzECANDataAvailable(void)
{
    return g_wzCANQ_count;
}

void wzECANFlushBuffer(void)
{
    while(g_wzCANQ_count > 0)
        wzECANReadData(0);
}

//CAN Error counter
uint8_t wzECANWriteErrorCount(void)
{
    // This register contains a value which is derived from the rate at which errors occur. When the error
    // count overflows, the bus-off state occurs. When the bus has 256 occurrences of 11 consecutive
    // recessive bits, the counter value is cleared.
    return TXERRCNT;
}
uint8_t wzECANReadErrorCount(void)
{
    // This register contains the receive error value as defined by the CAN specifications. When
    // RXERRCNT > 127, the module will go into an error-passive state. RXERRCNT does not have the
    // ability to put the module in “bus-off” state.
    return RXERRCNT;
}

//interrupt functions
void wzECANHandleRxInt(void) //RXB0 or RXB1
{
    WIZ_CAN_PACKET dataPacket;

    //double buffer
    if(RXB1CONbits.RXFUL) {
        dataPacket.id.sid = RXB1SIDH;
        dataPacket.id.sid = dataPacket.id.sid << 3;
        dataPacket.id.sid |= RXB1SIDL >> 5;
        dataPacket.dlc = RXB1DLC & 0x0F;
        dataPacket.isEXID = RXB1SIDLbits.EXID;

        if(dataPacket.isEXID) {
            //EX Format
            dataPacket.id.eid = (uint32_t)(RXB1SIDL >> 5);
            dataPacket.id.eid = dataPacket.id.eid << 27;
            dataPacket.id.eid |= (uint32_t)RXB1SIDH << 19;
            
            dataPacket.id.eid |= (uint32_t)(RXB1SIDL & 0x03) << 16;
            dataPacket.id.eid |= (uint32_t)RXB1EIDH << 8;
            dataPacket.id.eid |= RXB1EIDL;
        }

        //Receive Buffer 1 has overflowed
        if(COMSTATbits.RXB1OVFL) {
            COMSTATbits.RXB1OVFL = 0;
        }

        //Set Data
        if(RXB1CONbits.RXB1RTRR0) {
            //Remote frame
            dataPacket.isRemote = bTrue;
        } else {
            //Data frame
            dataPacket.isRemote = bFalse;
            dataPacket.D0 = RXB1D0;
            dataPacket.D1 = RXB1D1;
            dataPacket.D2 = RXB1D2;
            dataPacket.D3 = RXB1D3;
            dataPacket.D4 = RXB1D4;
            dataPacket.D5 = RXB1D5;
            dataPacket.D6 = RXB1D6;
            dataPacket.D7 = RXB1D7;
        }

        RXB1CONbits.RXFUL = 0;
    } else
    if(RXB0CONbits.RXFUL) {
        dataPacket.id.sid = RXB0SIDH;
        dataPacket.id.sid = dataPacket.id.sid << 3;
        dataPacket.id.sid |= RXB0SIDL >> 5;
        dataPacket.dlc = RXB0DLC & 0x0F;
        dataPacket.isEXID = RXB0SIDLbits.EXID;

        if(dataPacket.isEXID) {
            //EX Format
            dataPacket.id.eid = (uint32_t)(RXB0SIDL >> 5);
            dataPacket.id.eid = dataPacket.id.eid << 27;
            dataPacket.id.eid |= (uint32_t)RXB0SIDH << 19;
            
            dataPacket.id.eid |= (uint32_t)(RXB0SIDL & 0x03) << 16;
            dataPacket.id.eid |= (uint32_t)RXB0EIDH << 8;
            dataPacket.id.eid |= RXB0EIDL;
        }

        //Receive Buffer 0 has overflowed
        if(COMSTATbits.RXB0OVFL) {
            COMSTATbits.RXB0OVFL = 0;
        }

        //Set Data
        if(RXB0CONbits.RXB0RTRR0) {
            //Remote frame
            dataPacket.isRemote = bTrue;
        } else {
            //Data frame
            dataPacket.isRemote = bFalse;
            dataPacket.D0 = RXB0D0;
            dataPacket.D1 = RXB0D1;
            dataPacket.D2 = RXB0D2;
            dataPacket.D3 = RXB0D3;
            dataPacket.D4 = RXB0D4;
            dataPacket.D5 = RXB0D5;
            dataPacket.D6 = RXB0D6;
            dataPacket.D7 = RXB0D7;
        }

        RXB0CONbits.RXFUL = 0;
    }
    else
        return;
    
    //Queue is full
    if(g_wzCANQ_count == WIZ_ECAN_RECEIVE_BUFSIZE) {
        return; //fall
    }
    
    //Recv buffer enqueue
    g_wzCANBuff[g_wzCANQ_rear++] = dataPacket;
    ++g_wzCANQ_count;
    
    if(g_wzCANQ_rear == WIZ_ECAN_RECEIVE_BUFSIZE)
        g_wzCANQ_rear = 0;
    
    //Enqueue task
    if(g_intECANRxTask != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intECANRxTask);  //int
}

void wzECANIntRxTask(taskfunc_t task)
{
    g_intECANRxTask = task;
}

void wzECANHandleTxInt(void) //TXB0
{
    if(CANCONbits.ABAT) {
        TXB0CONbits.TXREQ = 0;
        PIE5bits.TXB0IE = 0;
        return;
    }

    //Queue is empty
    if(g_wzCANSendQ_count == 0) {
        PIE5bits.TXB0IE = 0;
        return;
    }
    
    //Next data
    uint8_t front = g_wzCANSendQ_front++;
    --g_wzCANSendQ_count;
    if(g_wzCANSendQ_front == WIZ_ECAN_SEND_BUFSIZE)
        g_wzCANSendQ_front = 0;
    
    WIZ_CAN_PACKET* packet = (WIZ_CAN_PACKET*)&g_wzCANSendBuff[front];
    
    //Data Set
    TXB0DLC = packet->dlc;
    TXB0SIDH = (packet->id.sid >> 3) & 0xFF;
    TXB0SIDL = (packet->id.sid << 5) & 0xFF;

    if(packet->isEXID) {
        //EX format
        uint32_t eid = packet->id.eid;

        TXB0SIDH = (eid >> 21) & 0xFF;
        TXB0SIDL = (eid >> 13) & 0xE0;
        TXB0SIDL |= (eid >> 16) & 0x03;
        TXB0EIDH = (eid >> 8) & 0xFF;
        TXB0EIDL = eid & 0xFF;

        TXB0SIDLbits.EXIDE = 1;
        TXB0SIDLbits.SRR = 1;
    }

    if(packet->isRemote) {
        TXB0DLC = 0x00;
        TXB0DLCbits.TXRTR = 1;
    }
    else
    {
        TXB0D0 = packet->D0;   //data
        TXB0D1 = packet->D1;
        TXB0D2 = packet->D2;
        TXB0D3 = packet->D3;
        TXB0D4 = packet->D4;
        TXB0D5 = packet->D5;
        TXB0D6 = packet->D6;
        TXB0D7 = packet->D7;   
    }

    PIR5bits.TXB0IF = 0;
    PIE5bits.TXB0IE = 1;

    //Transmission
    TXB0CONbits.TXREQ = 1;
    
    if(g_intECANTxTask != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intECANTxTask);  //int
}

void wzECANHandleTxErrInt(void) //TXB0 Error
{   
    if(TXB0CONbits.TXERR) // Error
    {
        TXB0CONbits.TXREQ = 0;
        if(TXB0CONbits.TXB0LARB) {
            //Wait
            wzDelayUSec(WIZ_ECAN_TOWAIT);
            //Retransmission
            TXB0CONbits.TXREQ = 1;
        } else {
            //Error
            g_wzCANSendQ_front = g_wzCANSendQ_rear = g_wzCANSendQ_count = 0;
            PIE5bits.TXB0IE = 0;    //end
            //Enqueue task
            if(g_intECANRxTask != WIZ_TASK_NULL)
                wzTaskEnqueueForInt(g_intECANTxErrorTask);  //error int   
        }
        
        //Abort
        if(TXB0CONbits.TXABT) { //Cancelled
            g_wzCANSendQ_front = g_wzCANSendQ_rear = g_wzCANSendQ_count = 0;
            PIE5bits.TXB0IE = 0;    //end
        }
    }
}

void wzECANIntTxTask(taskfunc_t task)
{
    g_intECANTxTask = task;
}

void wzECANIntTxErrorTask(taskfunc_t task)
{
    g_intECANTxErrorTask = task;
}
