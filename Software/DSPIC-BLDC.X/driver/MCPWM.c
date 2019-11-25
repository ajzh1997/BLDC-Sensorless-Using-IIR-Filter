#include "MCPWM.h"
void MCPWM_Initialize(void)
{
	OVDCON = 0;
	
	PTPER = PERIOD;		// set PWM period
	PWMCON1 = 0x0777;	// Enable PWM channels

	DTCON1 = DT_COUNT;	// 2 us deadtime
	
	FLTACON = NO_FAULT_PROTECTION;	// PWM1L/H, PWM2L/H, PWM3L/H have no current limiting enabled -- it's taken care of in hardware
	
	PDC1 = 0;			// Set all PWM duty cycles initially to zero
	PDC2 = 0;			
	PDC3 = 0;
	
	SEVTCMP = 0;		// no special event trigger
	
	PTCON = 0x8000;		// 
	
	IFS2bits.PWMIF = 0;	// enable the PWM interrupt
	IEC2bits.PWMIE = 1;
}
