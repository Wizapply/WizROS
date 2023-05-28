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
 *  File Name : wizros.c
 *
***************************************************************************/

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

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1L
#pragma config RETEN = OFF		// VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH	// LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = DIG	// SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)
#pragma config XINST = OFF		// Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC = HS2       // Oscillator (HS oscillator (High power, 16 MHz - 25 MHz))
#pragma config PLLCFG = ON      // PLL x4 Enable bit (Enabled)
#pragma config FCMEN = OFF		// Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF		// Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = ON		// Power Up Timer (Enable)
#pragma config BOREN = SBORDIS	// Brown Out Detect (Enabled in hardware, SBOREN disabled)
#pragma config BORV = 0			// Brown-out Reset Voltage bits (3.0V)
#pragma config BORPWR = MEDIUM	// BORMV Power level (BORMV is set to a medium power level)

// CONFIG2H
#pragma config WDTEN = ON       // Watchdog Timer (WDT controlled by SWDTEN bit setting)
#pragma config WDTPS = 65536    // Watchdog Postscaler (1:65536) 4 min

// CONFIG3H
#pragma config CANMX = PORTB	// ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK = MSK7	// MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE = ON		// Master Clear Enable (MCLR Enabled, RE3 Disabled)

// CONFIG4L
#pragma config STVREN = ON		// Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB1K		// Boot Block Size (1K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF		// Code Protect 00800-01FFF (Disabled)
#pragma config CP1 = OFF		// Code Protect 02000-03FFF (Disabled)
#pragma config CP2 = OFF		// Code Protect 04000-05FFF (Disabled)
#pragma config CP3 = OFF		// Code Protect 06000-07FFF (Disabled)

// CONFIG5H
#pragma config CPB = OFF		// Code Protect Boot (Disabled)
#pragma config CPD = OFF		// Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF		// Table Write Protect 00800-03FFF (Disabled)
#pragma config WRT1 = OFF		// Table Write Protect 04000-07FFF (Disabled)
#pragma config WRT2 = OFF		// Table Write Protect 08000-0BFFF (Disabled)
#pragma config WRT3 = OFF		// Table Write Protect 0C000-0FFFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF		// Config. Write Protect (Disabled)
#pragma config WRTB = OFF		// Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF		// Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 = OFF		// Table Read Protect 00800-03FFF (Disabled)
#pragma config EBTR1 = OFF		// Table Read Protect 04000-07FFF (Disabled)
#pragma config EBTR2 = OFF		// Table Read Protect 08000-0BFFF (Disabled)
#pragma config EBTR3 = OFF		// Table Read Protect 0C000-0FFFF (Disabled)

// CONFIG7H
#pragma config EBTRB = OFF		// Table Read Protect Boot (Disabled)

//-------------------------------------------------------------------------
//  Prototype functions
//-------------------------------------------------------------------------
//init system
void Pic18f25k80_init(void);
void wzSetup(void);
void inp_sys(void);

//GPIO
void wzGPIOInitialize(void);
void wzGPIOInitializeTRISPin(void);

//-------------------------------------------------------------------------
//  Global Varriables
//-------------------------------------------------------------------------
volatile taskfunc_t g_pTask[WIZ_TASK_SIZE];	//Task array
volatile int8_t g_pTaskStart;	//Task front
volatile int8_t g_pTaskEnd;     //Task rear

//interruptSystem
volatile taskfunc_t g_pInterruptPointer __at(0x45);

//-------------------------------------------------------------------------
//  Interrupt functions
//-------------------------------------------------------------------------
void __interrupt() inp(void) //high_priority
{
    g_pInterruptPointer();
}

void inp_sys(void)
{
    //TX
    if(PIE1bits.TX1IE && PIR1bits.TX1IF) {
        wzEUSARTHandleTxInt();
    }
    if(PIE5bits.TXB0IE && PIR5bits.TXB0IF) {
        CANCON &= 0b11110001;
        CANCON |= 0b00001000;   //Transmit Buffer 0
        PIR5bits.TXB0IF = 0;
        wzECANHandleTxInt();
    }
    if(PIE5bits.ERRIE && PIR5bits.ERRIF) {
        CANCON &= 0b11110001;
        CANCON |= 0b00001000;   //Transmit Buffer 0
        PIR5bits.ERRIF = 0;
        wzECANHandleTxErrInt();
    }
    
    if (PIE1bits.SSPIE && PIR1bits.SSPIF) {
        wzMSSPSlaveHandleInt();
        PIR1bits.SSPIF = 0;
    }
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        wzTMR0HandleInt();
        INTCONbits.TMR0IF = 0;
    }
    if (PIE1bits.TMR1IE && PIR1bits.TMR1IF) { 
        wzTMR1HandleInt();
        PIR1bits.TMR1IF = 0;
    }
    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF) {
        wzTMR2HandleInt();
        PIR1bits.TMR2IF = 0;
    }
    if (INTCONbits.INT0IE && INTCONbits.INT0IF) {
        wzGPIOHandleInt();
        INTCONbits.INT0IF = 0;
    }
    if (INTCON3bits.INT1IE && INTCON3bits.INT1IF) {
        wzGPIOHandleInt();
        INTCON3bits.INT1IF = 0;
    }
    if (INTCON3bits.INT2IE && INTCON3bits.INT2IF) {
        wzGPIOHandleInt();
        INTCON3bits.INT2IF = 0;
    }
    if (INTCON3bits.INT3IE && INTCON3bits.INT3IF) {
        wzGPIOHandleInt();
        INTCON3bits.INT3IF = 0;
    }
    if (PIE1bits.RC1IE && PIR1bits.RC1IF) {
        wzEUSARTHandleRxInt();
    }
    if (PIE5bits.RXB0IE && PIR5bits.RXB0IF) {
        CANCON &= 0b11110001;//Receive Buffer 0
        PIR5bits.RXB0IF = 0;
        wzECANHandleRxInt();
    }

    if (PIE5bits.RXB1IE && PIR5bits.RXB1IF) {
        CANCON &= 0b11110001;
        CANCON |= 0b00001010;   //Receive Buffer 1
        PIR5bits.RXB1IF = 0;
        wzECANHandleRxInt();
    }
}

//-------------------------------------------------------------------------
//  Entry point (main)
//-------------------------------------------------------------------------
void main(void)
{
	//Initialize
    g_pTaskStart = g_pTaskEnd = -1;
 
    g_pInterruptPointer = inp_sys;   //interrupt change

    wzDelayMSec(2);    //2ms wait
	Pic18f25k80_init();
    wzDelayMSec(5);     //5ms wait
    
    CLRWDT(); //Clear WDT
	wzSetup();
    
    wzGPIOInitializeTRISPin();
    
	while(1) {
        taskfunc_t tf = WIZ_TASK_NULL;
        CLRWDT(); //Clear WDT
		//loop
		while((tf = wzTaskDnqueue()) != WIZ_TASK_NULL) {
            CLRWDT(); //Clear WDT
            tf();   //running
        }
	}
}

void Pic18f25k80_init(void)
{
	INTCON = 0x00;  //init config
	INTCON2 = 0xFF;
	INTCON3 = 0xC0;
    
    RCON = 0x7C;
    
	PORTA = 0x00;	// PORT clear
	PORTB = 0x00;
	PORTC = 0x00;
	TRISA = 0xFF;	// setting input or output, default is input
	TRISB = 0xFF;
	TRISC = 0xFF;
    
    wzGPIOInitialize();

    ANCON0 = 0x00;  // digital mode
    ANCON1 = 0x00;
    
    ADCON0 = 0x00;
    ADCON1 = 0x00;
    ADCON2 = 0x00;
    
    TMR0H = 0x00;
    TMR0L = 0x00;
    T0CON = 0x00;
    
    TMR1H = 0x00;
    TMR1L = 0x00;
    T1CON = 0x00;
    T2CON = 0x00;
    T3CON = 0x00;
    T4CON = 0x00;
    
    SSPSTAT = 0x00;
    SSPCON1 = 0x00;
    SSPCON2 = 0x00;
    
    ECANCON = 0x10; //Receive Buffer 0
    CANCON = 0x80;  //Off
    CIOCON = 0x00;
    TXB0CON = 0x00;
    
    TXSTA1 = 0x00;
    RCSTA1 = 0x00;
    
    PIE1 = 0x00;
    PIE2 = 0x00;
    PIE3 = 0x00;
    PIE4 = 0x00;
    PIE5 = 0x00;
    
    ODCON = 0x00;
    SLRCON = 0x00;
    
    //Analog default
    ADCON2bits.ADFM = 1;
    ADCON2bits.ADCS = 0b110;// FOSC/64
    ADCON1bits.VCFG0 = 0;   // Vref+ = AVdd
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.VNCFG = 0;   // Vref- = AVss
    ADCON1bits.CHSN = 0;
    
    ADCON2bits.ACQT = 0b101;//12 TAD
    ADRES = 0;
    
    //Weak Pull-Up disable
    WPUB = 0;
    
    //CPP
    CCPTMRS = 0x01; //ECCP1 is based off of TMR3/TMR4
    
    //CTMU
    CTMUICON = 0x00;
    CTMUCONH = 0x00;
    CTMUCONL = 0x00;
    
    //WDT
    WDTCON = 0x00;
    WDTCONbits.SWDTEN = 1; //On
    
    //Start interrupt
    INTCONbits.PEIE = 1;    //Not priority
    INTCONbits.GIE = 1;
}

//-------------------------------------------------------------------------
//  Task system
//-------------------------------------------------------------------------
void wzTaskEnqueueForInt(taskfunc_t task)
{
    //Queue
    //Now add it to q
    if(((g_pTaskEnd == WIZ_TASK_SIZE-1) && g_pTaskStart == 0)
            || ((g_pTaskEnd+1) == g_pTaskStart))
    {
        g_pTaskStart++;  //Full
        if(g_pTaskStart == WIZ_TASK_SIZE)
            g_pTaskStart = 0;
    }
    if(g_pTaskEnd == WIZ_TASK_SIZE-1)
        g_pTaskEnd = 0;
    else
        g_pTaskEnd++;

    g_pTask[g_pTaskEnd] = task;
    if(g_pTaskStart == -1) g_pTaskStart = 0;
}

void wzTaskEnqueue(taskfunc_t task)
{
    INTCONbits.GIE = 0;
    //Queue
    //Now add it to q
    if(((g_pTaskEnd == WIZ_TASK_SIZE-1) && g_pTaskStart == 0)
            || ((g_pTaskEnd+1) == g_pTaskStart))
    {
        g_pTaskStart++;  //Full
        if(g_pTaskStart == WIZ_TASK_SIZE)
            g_pTaskStart = 0;
    }
    if(g_pTaskEnd == WIZ_TASK_SIZE-1)
        g_pTaskEnd = 0;
    else
        g_pTaskEnd++;

    g_pTask[g_pTaskEnd] = task;
    if(g_pTaskStart == -1) g_pTaskStart = 0;
    
    INTCONbits.GIE = 1;
}

taskfunc_t wzTaskDnqueue(void)
{
    taskfunc_t f;
    
    //Check if q is empty
    if(g_pTaskStart == -1)
        return WIZ_TASK_NULL;
    
    INTCONbits.GIE = 0;
        
    f = g_pTask[g_pTaskStart];
    
    if(g_pTaskStart == g_pTaskEnd)
    {
        //If single data is left
        //So empty q
        g_pTaskStart = g_pTaskEnd = -1;
    }
    else
    {
        g_pTaskStart++;

        if(g_pTaskStart == WIZ_TASK_SIZE)
            g_pTaskStart = 0;
    }
    
    INTCONbits.GIE = 1;
    
    return f;
}

bool_t wzTaskEmptyQueue(void)
{
    if(g_pTaskStart == g_pTaskEnd)
        return bFalse;
    else
        return bTrue;
}

void wzTaskClear(void)
{
    INTCONbits.GIE = 0;
    
    g_pTaskStart = g_pTaskEnd = -1;
    
    INTCONbits.GIE = 1;
}

void wzSleepMode(void)
{
    //Clock Set
    OSCCONbits.IDLEN = 0;   //Sleep
    OSCCONbits.SCS = 0;
    
    //for Errata
    if(CIOCONbits.ENDRHI)
       wzGPIOPinInOutMode(WIZ_PINMODE_B2, WIZ_PINMODEINOUT_IN);
    
    NOP();
    Sleep();
    NOP();
    NOP();
    NOP();
    
    if(CIOCONbits.ENDRHI)
       wzGPIOPinInOutMode(WIZ_PINMODE_B2, WIZ_PINMODEINOUT_OUT);
}