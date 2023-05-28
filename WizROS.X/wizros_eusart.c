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
 *  File Name : wizros_eusart.c
 *
***************************************************************************/

//-------------------------------------------------------------------------
//  Includes
//-------------------------------------------------------------------------
#include "wizros_eusart.h"
#include "wizros_gpio.h"

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile uint8_t g_wzURBuff[WIZ_EUSART_RECEIVE_BUFSIZE];	//USART Receive Buffer
volatile uint8_t g_wzUQ_front;
volatile uint8_t g_wzUQ_rear;
volatile uint8_t g_wzUQ_count;

volatile uint8_t g_wzUSendBuff[WIZ_EUSART_SEND_BUFSIZE];	//USART Send Buffer
volatile uint8_t g_wzUSendQ_front;
volatile uint8_t g_wzUSendQ_rear;
volatile uint8_t g_wzUSendQ_count;

volatile bool_t g_wzUSARTConfig;

volatile bool_t g_wzUseCR;
volatile taskfunc_t g_intEUSARTTxTask;	//Task
volatile taskfunc_t g_intEUSARTRxTask; 

//-------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------
void wzEUSARTInitialize(uint8_t baud_rate_flag) //, uint8_t config
{    
    uint8_t config = 0x00;
    //SPBRG
    switch(baud_rate_flag)
    {
    case WIZ_EUSART_BRFLAG_9600:
    default:
		SPBRGH1 = 0x01;
        SPBRG1  = 0x9F;
        break;
    case WIZ_EUSART_BRFLAG_14400:
		SPBRGH1 = 0x01;
        SPBRG1  = 0x14;
        break;
    case WIZ_EUSART_BRFLAG_19200:
		SPBRGH1 = 0x00;
        SPBRG1  = 0xCF;
        break;
    case WIZ_EUSART_BRFLAG_38400:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x67;
        break;
     case WIZ_EUSART_BRFLAG_57600:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x44;
        break;
    case WIZ_EUSART_BRFLAG_115200:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x21;
        break;
    case WIZ_EUSART_BRFLAG_230400:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x10;
        break;
    case WIZ_EUSART_BRFLAG_500000:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x07;
        break;
    case WIZ_EUSART_BRFLAG_1000000:
		SPBRGH1 = 0x00;
        SPBRG1  = 0x03;
        break;
    }
    
    //Config
    g_wzUSARTConfig = config;
    
    BAUDCON1bits.BRG16 = 1;	//16bit
        
    //TXSTA EUSART1
    TXSTA1bits.TXEN = 1; //Transmit enable
    TXSTA1bits.SYNC = 0; //Async mode
    TXSTA1bits.BRGH = 0; //Low speed mode(16bit is not use)
            
    //RCSTA
    RCSTA1bits.SPEN = 1;   //Serial port enabled
    RCSTA1bits.CREN = 1;   //Enable receive
    RCSTA1bits.ADDEN = 0;  //Disable address detection
    
    //config
    if((config & 0x0F)==0) {    //NOPARITY
        RCSTA1bits.RX9 = 0;  //8 bit mode
        TXSTA1bits.TX9 = 0;  //8 bit transmission
    } else {
        if((config & 0xF0)==0) {
            RCSTA1bits.RX9 = 1;  //9 bit mode
            TXSTA1bits.TX9 = 1;  //9 bit transmission
        }else{
            RCSTA1bits.RX9 = 0;  //8 bit mode
            TXSTA1bits.TX9 = 0;  //8 bit transmission
        }
    }

    wzGPIOPinInOutMode(WIZ_PINMODE_C7, WIZ_PINMODEINOUT_IN);    //rx: input  RC7
    wzGPIOPinInOutMode(WIZ_PINMODE_C6, WIZ_PINMODEINOUT_OUT);   //tx: output RC6

    RCREG;	// rx cleanup
    
    //Receive interrupt
    PIE1bits.TX1IE = 0; //default 0
    PIE1bits.RC1IE = 1;
    
    //Setup queue
    g_wzUseCR = bTrue;
    g_wzUQ_front = g_wzUQ_rear = g_wzUQ_count = 0;
    g_wzUSendQ_front = g_wzUSendQ_rear = g_wzUSendQ_count = 0;
    
    g_intEUSARTTxTask = WIZ_TASK_NULL;
    g_intEUSARTRxTask = WIZ_TASK_NULL;
}

void wzEUSARTWriteChar(char ch)
{
    //First
    if(PIE1bits.TX1IE == 0) {
        TXREG1 = ch;
        PIE1bits.TX1IE = 1;
        return;
    }
    
    //Queue is full
    if(g_wzUSendQ_count == WIZ_EUSART_SEND_BUFSIZE) {
        return; //fall
    }
    
    PIE1bits.TX1IE = 0;
    
    //Send buffer enqueue
    g_wzUSendBuff[g_wzUSendQ_rear++] = ch;
    ++g_wzUSendQ_count;
    if(g_wzUSendQ_rear == WIZ_EUSART_SEND_BUFSIZE)
        g_wzUSendQ_rear = 0;

    PIE1bits.TX1IE = 1;
}

//stdio.h printf
void putch(char data)
{
    wzEUSARTWriteChar(data);
}

void wzEUSARTWriteString(const char *str)
{
    while(*str != '\0')
    {
        wzEUSARTWriteChar(*str);
        str++;
    }
}

void wzEUSARTWriteLine(const char *str)
{
    wzEUSARTWriteString(str);
    if(g_wzUseCR) wzEUSARTWriteChar('\r');//CR
    wzEUSARTWriteChar('\n');//LF
}

void wzEUSARTWriteInt16(int16_t val, int8_t field_length)
{
    int8_t str[5] = {0,0,0,0,0};
    int8_t i = 4, j = 0;

    //Handle negative integers
    if(val < 0)
    {
        wzEUSARTWriteChar('-');   //Write Negative sign
        val = val * -1;     //convert to positive
    }

    if(val == 0 && field_length < 1)
    {
        wzEUSARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i] = val % 10;
        val = val / 10;
        i--;
    }

    if(field_length == -1)
        while(str[j] == 0) j++;
    else
        j = 5 - field_length;

    for(i = j; i < 5; i++)
        wzEUSARTWriteChar('0' + (uint8_t)str[i]);
}

void wzEUSARTWriteUInt16(uint16_t val, int8_t field_length)
{
    int8_t str[5] = {0,0,0,0,0};
    int8_t i = 4, j = 0;

    if(val == 0 && field_length < 1)
    {
        wzEUSARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i] = val % 10;
        val = val / 10;
        i--;
    }

    if(field_length == -1)
        while(str[j] == 0) j++;
    else
        j = 5 - field_length;

    for(i = j; i < 5; i++)
        wzEUSARTWriteChar('0' + (uint8_t)str[i]);
}

void wzEUSARTWriteInt32(int32_t val, int8_t field_length)
{
    int8_t str[10] = {0,0,0,0,0,0,0,0,0,0};
    int8_t i = 9, j = 0;

    //Handle negative integers
    if(val < 0)
    {
        wzEUSARTWriteChar('-');   //Write Negative sign
        val = val * -1;     //convert to positive
    }

    if(val == 0 && field_length < 1)
    {
        wzEUSARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i] = val % 10;
        val = val / 10;
        i--;
    }

    if(field_length == -1)
        while(str[j] == 0) j++;
    else
        j = 10 - field_length;

    for(i = j; i < 10; i++)
        wzEUSARTWriteChar('0' + (uint8_t)str[i]);
}

void wzEUSARTWriteUInt32(uint32_t val, int8_t field_length)
{
    int8_t str[10] = {0,0,0,0,0,0,0,0,0,0};
    int8_t i = 9, j = 0;

    if(val == 0 && field_length < 1)
    {
        wzEUSARTWriteChar('0');
        return;
    }
    while(val)
    {
        str[i] = val % 10;
        val = val / 10;
        i--;
    }

    if(field_length == -1)
        while(str[j] == 0) j++;
    else
        j = 10 - field_length;

    for(i = j; i < 10; i++)
        wzEUSARTWriteChar('0' + (uint8_t)str[i]);
}

void wzEUSARTWriteNewLine(void)
{
    if(g_wzUseCR) wzEUSARTWriteChar('\r');//CR
    wzEUSARTWriteChar('\n');//LF
}

void wzEUSARTWriteBytes(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++) {
        wzEUSARTWriteChar(data[i]);
    }
}

void wzEUSARTEndOfLineUsingCR(bool_t useCR)
{
    g_wzUseCR = useCR;
}

void wzEUSARTWriteEndWait(void)
{
    while(PIE1bits.TX1IE);
    while(!TXSTA1bits.TRMT);
}


void wzEUSARTBreakSignal(void)
{
    while(PIE1bits.TX1IE);
    while(!TXSTA1bits.TRMT);
    
    TXSTA1bits.SENDB = 1;
    TXREG1 = 0x55;   //dummy
    
    while(TXSTA1bits.SENDB);
    while(!TXSTA1bits.TRMT);
    
    /*
    //sync byte
    TXREG1 = 0x55; 
    
    while(!PIR1bits.TXIF);
    while(!TXSTA1bits.TRMT);
    */
}

uint8_t wzEUSARTReadData(void)
{
    uint8_t data;

    //Queue is empty
    if(g_wzUQ_count == 0) {
        return 0;
    }
    
    PIE1bits.RC1IE = 0;

    uint8_t front = g_wzUQ_front++;
    --g_wzUQ_count;
    if(g_wzUQ_front == WIZ_EUSART_RECEIVE_BUFSIZE)
        g_wzUQ_front = 0;
   
    data = g_wzURBuff[front];
    
    PIE1bits.RC1IE = 1;

    return data;
}

void wzEUSARTReadBuffer(uint8_t *buff, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
        buff[i] = wzEUSARTReadData();
}

uint8_t wzEUSARTDataAvailable(void)
{
    return g_wzUQ_count;
}

void wzEUSARTFlushBuffer(void)
{
    while(g_wzUQ_count > 0)
        wzEUSARTReadData();
}

//Rx
void wzEUSARTHandleRxInt(void)
{
    //Read the data
    uint8_t data = RCREG; //throw away
    
    //Error check
    if(RCSTA1bits.FERR) {
        //Error
        return;
    }
    if(RCSTA1bits.OERR) {
        //Error stop
        RCSTA1bits.CREN = 0;
    } 
    
    //Queue is full
    if(g_wzUQ_count == WIZ_EUSART_RECEIVE_BUFSIZE) {
        return; //fall
    }
    
    //Recv buffer enqueue
    g_wzURBuff[g_wzUQ_rear++] = data;
    ++g_wzUQ_count;
    
    if(g_wzUQ_rear == WIZ_EUSART_RECEIVE_BUFSIZE)
        g_wzUQ_rear = 0;

    //Enqueue task
    if(g_intEUSARTRxTask != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intEUSARTRxTask);  //int
}

void wzEUSARTIntRxTask(taskfunc_t task)
{
    g_intEUSARTRxTask = task;
}

//Tx
void wzEUSARTHandleTxInt(void)
{
    //Queue is empty
    if(g_wzUSendQ_count == 0) {
        PIE1bits.TX1IE = 0;
        return;
    }

    //next data
    uint8_t front = g_wzUSendQ_front++;
    --g_wzUSendQ_count;
    if(g_wzUSendQ_front == WIZ_EUSART_SEND_BUFSIZE)
        g_wzUSendQ_front = 0;
    
    TXREG1 = g_wzUSendBuff[front];
    
    //Enqueue task
    if(g_intEUSARTTxTask != WIZ_TASK_NULL)
        wzTaskEnqueueForInt(g_intEUSARTTxTask);  //int
}

void wzEUSARTIntTxTask(taskfunc_t task)
{
    g_intEUSARTTxTask = task;
}
