/*
 * File:   main_bldc.c
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 12:05 PM
 */

#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"
#include "stdbool.h"


#include "xc.h"
#include "bldc.h"
#include "dsp.h"
#include "BEMF_filter.h"
#include "IIR_Filter.h"
#include "snap.h"
void INIT_SYSTEMS(void);    // Khởi tạo hệ thống

int main(void) 
{
    INIT_SYSTEMS();
    printf("INIT SYSTEMS\r\n");
    IIRTransposeFilterInit( &BEMF_phaseA_Filter );
	IIRTransposeFilterInit( &BEMF_phaseB_Filter );
	IIRTransposeFilterInit( &BEMF_phaseC_Filter );
    RunMode = MOTOR_OFF;
    ControlFlags.SpeedControlEnable = 1;
    while(1)
    {
        if(ControlFlags.MediumEventFlag)
			MediumEvent();				
		if(ControlFlags.SlowEventFlag)
			SlowEvent();
        if(ControlFlags.UARTEventFlag)
        {
            ControlFlags.UARTEventFlag = 0;
        }
    }
    return 0;
}
void INIT_SYSTEMS(void)
{
    LATE = 0x0000;
    TRISE = 0xFFC0;
  	LATC = 0;
  	LATD = 0;
    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD1 = 1;
    // Setup Timer1 for 1:8 prescale.  Timer1 is used to measure
 	// the period between zero crossings.  Enable Timer 1.
 	T1CON = 0x8010;
 	
 	// Timer2 and Timer 3 will be the commutation timers.  They
	//  have the same prescaler (1:8) as Timer 1.  Turn on Timer 2
	//  but not Timer 3  
 	T2CON = 0x0010;			// 1:8 prescaler
	TMR2 = 0;				// clear Timer 2
	IFS0bits.T2IF = 0;		// Disable the Timer 2 interrupt
	IEC0bits.T2IE = 0;
	T2CONbits.TON = 1; 		// Turn on Timer 2

	T3CON = 0x0010;			// 1:8 prescaler
	TMR3 = 0;				// clear Timer 3
	IFS0bits.T3IF = 0;		// Disable the Timer 3 interrupt
	IEC0bits.T3IE = 0;

	T4CON = 0x8010;    			// turn on timer 4
    UART1_Initialize();
    MCPWM_Initialize();
    Init_ADC();
}