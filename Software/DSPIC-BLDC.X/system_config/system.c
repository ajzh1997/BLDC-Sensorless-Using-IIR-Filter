/*
 * File:   system.c
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 11:58 AM
 */


#include "xc.h"

// FOSC
#pragma config FPR = XT_PLL16           // Primary Oscillator Mode (XT w/PLL 16x)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (2.0V)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

void __attribute__((__interrupt__,no_auto_psv)) _OscillatorFail(void);
void __attribute__((__interrupt__,no_auto_psv)) _AddressError(void);
void __attribute__((__interrupt__,no_auto_psv)) _StackError(void);
void __attribute__((__interrupt__,no_auto_psv)) _MathError(void);
void __attribute__((__interrupt__,no_auto_psv)) _AltOscillatorFail(void);
void __attribute__((__interrupt__,no_auto_psv)) _AltAddressError(void);
void __attribute__((__interrupt__,no_auto_psv)) _AltStackError(void);
void __attribute__((__interrupt__,no_auto_psv)) _AltMathError(void);
void __attribute__((__interrupt__,no_auto_psv)) _OscillatorFail(void)
{
        INTCON1bits.OSCFAIL = 0;        //Clear the trap flag
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _AddressError(void)
{
        INTCON1bits.ADDRERR = 0;        //Clear the trap flag
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _StackError(void)
{
        INTCON1bits.STKERR = 0;         //Clear the trap flag
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _MathError(void)
{
        INTCON1bits.MATHERR = 0;        //Clear the trap flag
        while (1);
}

/* Alternate Exception Vector handlers:
   These routines are used if INTCON2bits.ALTIVT = 1.
   All trap service routines in this file simply ensure that device
   continuously executes code within the trap service routine. Users
   may modify the basic framework provided here to suit to the needs
   of their application. */

void __attribute__((__interrupt__,no_auto_psv)) _AltOscillatorFail(void)
{
        INTCON1bits.OSCFAIL = 0;
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _AltAddressError(void)
{
        INTCON1bits.ADDRERR = 0;
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _AltStackError(void)
{
        INTCON1bits.STKERR = 0;
        while (1);
}

void __attribute__((__interrupt__,no_auto_psv)) _AltMathError(void)
{
        INTCON1bits.MATHERR = 0;
        while (1);
}