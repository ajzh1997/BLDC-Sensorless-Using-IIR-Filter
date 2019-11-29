#include "bldc.h"
void SYS_INIT(void)
{
    LATEbits.LATE5 = 0;	    //PWM1H3
	TRISEbits.TRISE5 = 0;
	LATEbits.LATE4 = 0;	    //PWM1L3
	TRISEbits.TRISE4 = 0;
	LATEbits.LATE3 = 0;	    //PWM1H2
	TRISEbits.TRISE3 = 0;
	LATEbits.LATE2 = 0;	    //PWM1L2
	TRISEbits.TRISE2 = 0;	
	LATEbits.LATE1 = 0;	    //PWM1H1
	TRISEbits.TRISE1 = 0;	
	LATEbits.LATE0 = 0;	    //PWM1L1
	TRISEbits.TRISE0 = 0;	 

	/*Push Buttons ports*/
	LATDbits.LATD0 = 0;
	TRISDbits.TRISD0 = 1;
	LATDbits.LATD1 = 0;
	TRISDbits.TRISD1 = 1;
    INTCON1bits.NSTDIS = 0;		    // Enabling nested interrupts
    MCPWM_INIT();
    ADC10_INIT();
    TMR2_INIT();
    TMR1_INIT();
}
void MCPWM_INIT(void)
{
   PTPER = ((FCY/FPWM)/2 - 1);
							//FCY  29491200...FRC w/PLL x16
							//FPWM 20KHz PWM Freq
							// MAX_DUTY_CYCLE = 1469
							// 50% duty cycle = 734
	PTCONbits.PTMOD = 2;	// Center Aligned with X interruption
	PWMCON1 = 0x0700;		// disable PWMs
	OVDCON = 0x0000;		// allow control using OVD
	SEVTCMPbits.SEVTDIR = 0; 	// trgger ADC when PWM counter is in upwards dir
								//....Tad=84.77, Tpwm=67.816
	SEVTCMPbits.SEVTCMP = 0;	// generates a trigger event for the ADC
								// when PWM time base is counting upwards
								// just before reaching the PTPER value
								// causing a sampling in the middle of the 
								// pulse
	FLTACON = 0x0000;		// all channels low, non latched
    PWMCON2 = 0x0000;		// 1:1 postscale values
	IPC9bits.PWMIP = 4;		// PWM Interrupt Priority 4
	IFS2bits.PWMIF=0;		// clear 
	IEC2bits.PWMIE=1;

	PTCONbits.PTEN = 1; 
}
void TMR2_INIT(void)
{							// Tcy = 33.908 nSec

	TMR2 = 0;				// Resetting TIMER
	PR2 = 0xFFFF;			// Setting TIMER periond to the MAX value
	T2CON = 0x0030;			// internal Tcy*64 clock = 2.17uSec
}
void TMR1_INIT(void)
{							// Tcy = 33.908 nSec
	TMR1 = 0;				// Resentting TIMER
	PR1 = 10;				// Intial commucation delay value 43.4 uSeg
	T1CON = 0x0030;			// internal Tcy*64 clock = 2.17uSec

	IPC0bits.T1IP = 5; 		// Set Timer 1 Interrupt Priority Level
	IFS0bits.T1IF = 0; 		// Clear Timer 1 Interrupt Flag
	IEC0bits.T1IE = 1; 		// Enable Timer1 interrupt
	T1CONbits.TON = 1; 		// Enable Timer1
}
void ADC10_INIT(void)
{
	ADPCFG = 0xFFC0;	//Port pin multiplexed with AN0,1,2,3,4,5 in Analog mode

	ADCON1 = 0x006C;    //ADC is off
						//Continue module operation in Idle mode
						//10-bit, 4-channel ADC operation
						//Data Output Format bits Integer (0000 00dd dddd dddd)
						//011 = Motor Control PWM interval ends sampling and starts conversion
						//Samples CH0, CH1, CH2, CH3 simultaneously when CHPS<1:0> = 1x
                        //Sampling begins immediately after last conversion SAMP bit is auto-set.

	ADCHS = 0x0022;     //MUX B Channel 0 negative input is VREF-
						//MUX B Channel 0 positive input is AN6
						//MUX A Channel 0 negative input is VREF-
						//MUX A Channel 0 positive input is AN2

	ADCSSL = 0x0000;	//Skip all ANx channels for input scan

	ADCON3 = 0x0004;	//ADC Clock derived from system clock
						//Autosample time time bits = 0 TAD since PWM is controlling sampling time
						//TAD = 3*TCY, TAD = 101.7 nSec

	ADCON2 = 0x0300;	//ADREF+ = AVDD ADREF- = AVSS
						//Do not scan inputs
						//1x = Converts CH0, CH1, CH2 and CH3
						//A/D is currently filling buffer 0x0-0x7
						//Interrupts at the completion of conversion for each sample/convert sequence
						//Always starts filling buffer from the beginning
						//Always uses channel input selects for Sample A

	ADCON1bits.DONE = 0;
	IPC2bits.ADIP = 5;	
	IFS0bits.ADIF = 0;	
	IEC0bits.ADIE = 1;
	ADCON1bits.ADON=1;
}