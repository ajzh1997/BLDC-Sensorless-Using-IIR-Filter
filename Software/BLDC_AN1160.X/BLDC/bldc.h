/* 
 * File:   bldc.h
 * Author: Duy Ngo
 *
 * Created on November 29, 2019, 1:48 PM
 */
#include "xc.h"
#include "dsp.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdlib.h"

#define FCY  29491200UL
#define FPWM 20000UL
#include "libpic30.h"
/*Push Buttons defines*/
#define S2	!PORTDbits.RD0		//Defines for the Push Buttons status
#define S3	!PORTDbits.RD1
void SYS_INIT(void);
void MCPWM_INIT(void);
void ADC10_INIT(void);
void TMR1_INIT(void);
void TMR2_INIT(void);
