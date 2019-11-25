#include "../system_config/general.h"    
/*---------------------------------------------------------------------
  Function Name: Init_ADC
  Description:   Initialize ADC module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void Init_ADC( void )
{
	/* set port configuration here */
	ADPCFGbits.PCFG2 = 0;          // ensure AN2 is analog  - pot 	
	ADPCFGbits.PCFG0 = 0;         // ensure AN0 is analog	- vbus
 	ADPCFGbits.PCFG6 = 0;         // ensure AN12 is analog - vpha
	ADPCFGbits.PCFG7 = 0;         // ensure AN13 is analog - vphb
 	ADPCFGbits.PCFG8 = 0;         // ensure AN14 is analog - vphc
   
 	/* set channel scanning here, auto sampling and convert, 
 	   with default read-format mode */
	ADCON1 = 0x00E4;
	
	/* channel scan for CH0+, Use MUX A,  
	   SMPI = 5 per interrupt, Vref = AVdd/AVss */
	ADCON2 = ADCON2_LOW_SPEED;		// intializ for low speed mode
	
	/* Set Samples and bit conversion time */
	//                                  |       |
	//                     (sample time)v       v(conversion time)
	/* Interrupt period: 5 channels * 12Tad + 12Tad  (where Tad = Tcy/2 *(ADCS<5:0> + 1))" */
	// 						5 * 24 * 1/29491200 * 1/2 * (9 + 1) = 2.0345e-5
	// Interrupt frequency: ~49 kHz
	ADCON3 = 0x0C09; 
        	
	/* scan specified channels */
	ADCSSL = ADCSSL_LOW_SPEED;  // Scan all 5 channels
	
	/* channel select AN7 */
	ADCHS = 0x0000;
//    ADCHSbits.CH0SA1 = 1;
//    ADCHSbits.CH0NA = 0;
	/* reset ADC interrupt flag */
	IFS0bits.ADIF = 0;           
    
	/* enable ADC interrupts, disable this interrupt if the DMA is enabled */	  
	IEC0bits.ADIE = 1;       

	ADCON1bits.ADON = 1;          	
}