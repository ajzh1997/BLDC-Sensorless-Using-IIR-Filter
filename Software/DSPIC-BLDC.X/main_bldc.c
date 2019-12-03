/*
 * File:   main_bldc.c
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 12:05 PM
 */

#include "stdlib.h"
#include "stddef.h"
#include "stdbool.h"


#include "xc.h"
#include "bldc.h"
#include "dsp.h"
#include "BEMF_filter.h"
#include "IIR_Filter.h"
#include "snap.h"
#include "RTDM.h"
#define AMOUNT_OF_DATA_TO_BE_PLOTTED 256 //number of sanpshot samples 512 bytes or 256 16-bit words

////////////////////////////////////////////////////////////////////////////////
/************************************ MAIN ***********************************/
////////////////////////////////////////////////////////////////////////////////
unsigned int MyVariable, Frequency, Amplitude;								//Varaible to be recored and its paramters to be modified using the DMCI sliders
unsigned int SnapShotBufferPlottedInDMCI[AMOUNT_OF_DATA_TO_BE_PLOTTED];		//buffer where the data is recorded
unsigned int * PointerToSnapShotBuffer = &SnapShotBufferPlottedInDMCI[0];	//Tail pointer required to plot circular buffers in DMCI
unsigned int * PointerToSnapShotBufferUpperLimit = &SnapShotBufferPlottedInDMCI[0] + AMOUNT_OF_DATA_TO_BE_PLOTTED -1;
																			//Buffer Upper limit



struct 
{
	unsigned TrigggerSnapshot: 	1; //Tirgger variable to start recording the values of MyVariable
    unsigned LED_C13: 	1;		   //Definition of Exp 16 LED D3-D6
    unsigned LED_C14: 	1;
    unsigned BUTTON_RUN: 	1;
    unsigned BUTTON_STOP: 	1;
    unsigned unused :		11;     
}	MyFlags;
void INIT_SYSTEMS(void);    // Khởi tạo hệ thống

int main(void) 
{
    TRISCbits.TRISC13 = 0;
    TRISCbits.TRISC14 = 0;
    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD1 = 1;
    LATCbits.LATC13 = 0;
    LATCbits.LATC14 = 0;
    RTDM_Start();
    INIT_SYSTEMS();
    IIRTransposeFilterInit( &BEMF_phaseA_Filter );
	IIRTransposeFilterInit( &BEMF_phaseB_Filter );
	IIRTransposeFilterInit( &BEMF_phaseC_Filter );
    RunMode = MOTOR_OFF;
    ControlFlags.EnablePotentiometer = 1;
    ControlFlags.SpeedControlEnable = 0;
    ControlFlags.TakeSnapshot = 1;
    while(1)
    {
        LATCbits.LATC13 = MyFlags.LED_C13;
        LATCbits.LATC14 = MyFlags.LED_C14;
        if(MyFlags.BUTTON_RUN && RunMode == MOTOR_OFF)
        {
            RunMode = SENSORLESS_INIT;
            MyFlags.BUTTON_RUN = 0;
        }
        if(MyFlags.BUTTON_STOP && RunMode != MOTOR_OFF)
        {
            RunMode = MOTOR_OFF;
        }
        RTDM_ProcessMsgs();
        if(ControlFlags.MediumEventFlag)
			MediumEvent();				
		if(ControlFlags.SlowEventFlag)
			SlowEvent();
    }
    RTDM_Close();
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
    MCPWM_Initialize();
    Init_ADC();
}