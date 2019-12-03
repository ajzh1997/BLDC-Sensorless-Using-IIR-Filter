#include "bldc.h"
#include "stdint.h"
#include "system_config/turning.h"
#include "BEMF_filter.h"
#include "IIR_Filter.h"
#include "snap.h"
#ifdef SNAPSHOT
int SnapBuf1[SNAPSIZE];
int SnapBuf2[SNAPSIZE];
int SnapBuf3[SNAPSIZE];
int SnapBuf4[SNAPSIZE];
#endif
// ----------------- Global Variable Declarations ----------------------
unsigned int pot;		    // stores the potentiometer value
unsigned int HalltoSector[8] = {-1,2,4,3,0,1,5,-1};
unsigned int SectortoState[6] = {STATE0,STATE1,STATE2,STATE3,STATE4,STATE5};
unsigned int HallState;		// Hall effect sensor state
int Sector = 0;				// Sector (0 - 5)
int OldSector;				// Stores old sector

int vpha, vphb, vphc, vbus;  // stores the ADC result for phase A, B, and C and the bus voltage measurement

unsigned int Timer1TimeoutCntr = 0;
unsigned int BlankingCount = 3;
unsigned int phase_advance = 0;
unsigned int SpeedPtr;
unsigned int RunMode = 0;
unsigned int SensorlessStartState = 0;
unsigned int MediumEventCounter = 0;
unsigned int SlowEventCounter = 0;
unsigned int UARTEventCounter = 0;
long accumulator_c;
int signal_average;
unsigned long Speed = 0;
int vpha_filtered_sample;
int vphb_filtered_sample;
int vphc_filtered_sample;
unsigned int NextSectorState;
unsigned int SixtyDegreeTime[6];
unsigned int OneEightyDegreeTime[16];

volatile struct ControlFlags ControlFlags;

unsigned int pos_ptr = 0;
unsigned int TMR4Save;

unsigned int DATA;
unsigned int DATA2;
void CheckZeroCrossing(void);
unsigned int ThirtyDegreeTimeAverage();
/*---------------------------------------------------------------------
  Function Name: ADCInterrupt
  Description:   ADC Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/

void __attribute__((__interrupt__ , no_auto_psv)) _ADCInterrupt( void )   // occurs at a rate of 81.920 kHz
{
	/* reset ADC interrupt flag */
	IFS0bits.ADIF = 0; 
		
	// Read the potentiometer and phase voltages.
	pot = (pot + POTBUF) >> 1;
	if (BlankingCount)				// if the blanking count hasn't expired, feed the Back EMF
	{								// filters the last filtered Back EMF sample (rather than the unfiltered sample.)
		BlankingCount--;
//		vpha = vpha_filtered_sample;
//		vphb = vphb_filtered_sample;
//		vphc = vphc_filtered_sample;

		vpha = (3*vpha_filtered_sample + VPHABUF)>>2;  // This three lines can be used an a alternative to the 
		vphb = (3*vphb_filtered_sample + VPHBBUF)>>2;  //  previous three lines
		vphc = (3*vphc_filtered_sample + VPHCBUF)>>2;
	}
	else
	{
		vpha = VPHABUF;
		vphb = VPHBBUF;
		vphc = VPHCBUF;
	}

	// Get the bus voltage and do a little averaging
	vbus =  (vbus + VBUSBUF) >> 1;
	switch (RunMode)
	{
		case MOTOR_OFF:
			OVDCON = 0;
			break;
		case HALL_SENSOR_MODE:
			// Read the hall sensors.  This is only done If running sensored.
			// Read Hall sensors to get position.
//			do
//			{
//				HallState = (PORTD >> 8) & 0x0007;
//				Sector = HalltoSector[HallState];
//				i++;
//			} while((Sector == -1) && (i<4));
//	
//			if(Sector != OldSector)
//			{
//				Commutate(Sector);
//				TMR2 = 0;
//				IFS0bits.T2IF = 0;
//			}
		case SENSORLESS_START:
		case SENSORLESS_RUNNING	:
			CheckZeroCrossing();							
			break;
		default:
			if (RunMode > (NO_OF_RUN_MODES - 1))  RunMode = 0;
			break;
	}
}

/*---------------------------------------------------------------------
  Function Name: T1Interrupt
  Description:   T1 Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void __attribute__((__interrupt__ , no_auto_psv)) _T1Interrupt( void ) 
{
	IFS0bits.T1IF = 0;
	if (Timer1TimeoutCntr++ >= NumOfTimer1TimeOuts)  // When Timer 1 overflows the algorithm is lost
	{
		IEC0bits.T1IE = 0;		// turn off Timer 1 and stop the motor
		RunMode = MOTOR_OFF;
	}
}

/*---------------------------------------------------------------------
  Function Name: T2Interrupt
  Description:   T2 Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void __attribute__((__interrupt__ , no_auto_psv)) _T2Interrupt( void )  // TMR 2 is never turned on in Hall Mode
{
	TMR2 = 0;			   			// clear TMR2
	IFS0bits.T2IF = 0;

	if (RunMode == SENSORLESS_RUNNING)	
	{
		Sector++;						// Increment Sector (there are 6 total)
		if(Sector > 5) Sector = 0;
		Commutate(Sector);				// Change the PWM output for next sector
	}
}

/*---------------------------------------------------------------------
  Function Name: T3Interrupt
  Description:   T3 Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void __attribute__((__interrupt__ , no_auto_psv)) _T3Interrupt( void )  // TMR 3 is never turned on in Hall Mode
{
	T3CONbits.TON = 0;     			// turn off TMR3
	TMR3 = 0;
	TMR2 = 0;		
	IFS0bits.T2IF = 0;	   		
	IFS0bits.T3IF = 0;
	IEC0bits.T3IE = 0;
	Timer1TimeoutCntr = 0;			// A commutation has occured so the Timer 1 timeout counter can be reset

	if (RunMode == SENSORLESS_RUNNING)	
	{
		Sector = NextSectorState;
		Commutate(Sector);				// Force commutation
	}
}

/*---------------------------------------------------------------------
  Function Name: PWMInterrupt
  Description:   PWM Interrupt Handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/

void __attribute__((__interrupt__ , no_auto_psv)) _PWMInterrupt( void )  // Occurs every 50us or at a rate of 20kHz
{
	IFS2bits.PWMIF = 0;             // Clear the PWM interrupt flag
	if (++SlowEventCounter >= 200)	// Fire Slow event every 10ms
	{
		SlowEventCounter = 0;
		ControlFlags.SlowEventFlag = 1;
	}
	
	if (++MediumEventCounter >= 20)  // Fire Medium event every 1ms
	{
		MediumEventCounter = 0;
		ControlFlags.MediumEventFlag = 1;
	}
	if (RunMode == SENSORLESS_START)
	{
		MediumEventCounter += 9;  // Fire Medium event every 100us in SENSORLESS_START mode
	}

}

/*---------------------------------------------------------------------
  Function Name: CheckZeroCrossing
  Description:   This function filters ADC samples of each motor phase
				  and looks at the filtered phase signals in order to 
				  detect a zero cross event
  Inputs:        None (global variables including vpha, vphb, vphc, and vbus)
  Returns:       None
-----------------------------------------------------------------------*/
void CheckZeroCrossing(void)
{
	static unsigned int ZeroCrossState = 0;
	unsigned int ThirtyDegreeTime;
	unsigned int ThreeSixtyDegreeTime;
	unsigned int phase_delay;
	static int vbus_offset = 0;
	
	
	if (ZeroCrossState < 6)  // If Low Speed Mode
	{
		BEMF_phaseA_Filter.pCoefs = &BEMF_filterCoefs_49152Hz;	
		BlockIIRTransposeFilter( &BEMF_phaseB_Filter, &vphb, &vphb_filtered_sample, 1 );
		BlockIIRTransposeFilter( &BEMF_phaseC_Filter, &vphc, &vphc_filtered_sample, 1 );
	}
	else
		BEMF_phaseA_Filter.pCoefs = &BEMF_filterCoefs_81940Hz;
	BlockIIRTransposeFilter( &BEMF_phaseA_Filter, &vpha, &vpha_filtered_sample, 1 );  // Get a filtered sample

	// tìm thấy điện áp trung tâm của tín hiệu pha ngay cả dưới các tải khác nhau
	signal_average = vbus/2 + vbus_offset;
	accumulator_c += vpha_filtered_sample - signal_average;
	vbus_offset = accumulator_c >> 13;
    // bat data de gui len terminal
    DATA = signal_average;
    DATA2 = vpha_filtered_sample;
	if (ZeroCrossState < 6)
    {
		phase_delay = FILTER_PHASE_DELAY + PROCESSING_DELAY + phase_advance;
    }
	else
    {
		phase_delay = FILTER_PHASE_DELAY + PROCESSING_DELAY_HS + phase_advance;
    }

	switch(ZeroCrossState)
	{
// States 0 - 5 implement the low speed mode of this algorithm.  All three phase voltages are sampled. The sampling frequency
// is 49kHz when running in high speed mode. 
        
//State 0 - 5 thực hiện chế độ tốc độ thấp của thuật toán này. Tất cả ba điện áp pha được lấy mẫu. Tần suất lấy mẫu
//là 49kHz khi chạy ở chế độ tốc độ cao.
		case 0:
			if (vpha_filtered_sample < signal_average)   // tín hiệu đang giảm khi tìm dưới mức điện áp trung tâm
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();			
				if (ThirtyDegreeTime < phase_delay)
                {
					ThirtyDegreeTime = phase_delay;
                }
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 3;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState++;
			}
			break;
		case 1:
			if (vphc_filtered_sample > signal_average)
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();
				if (ThirtyDegreeTime < phase_delay)
					ThirtyDegreeTime = phase_delay;
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 4;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState++;
			}		
			break;
		case 2:
			if (vphb_filtered_sample < signal_average)
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();
				if (ThirtyDegreeTime < phase_delay)
					ThirtyDegreeTime = phase_delay;
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 5;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState++;
			}		
			break;
		case 3:
			if (vpha_filtered_sample > signal_average)
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();
				if (ThirtyDegreeTime < phase_delay)
					ThirtyDegreeTime = phase_delay;
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 0;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState++;
				// do the change over from LowSpeedMode to HighSpeed Mode
                // thay đổi từ Chế độ tốc độ thấp sang Chế độ tốc độ cao
				if (ControlFlags.HighSpeedMode)
				{
					ZeroCrossState = 6;
					TMR2 = 0;
					PR2 = ThirtyDegreeTime*2;
					IFS0bits.T2IF = 0;	  
					IEC0bits.T2IE = 1;
					//ControlFlags.TakeSnapshot = 1;  // take shapshot on crossover
					ADCON1bits.ADON = 0;		// Turn ADC module off before modifying control bits;
					ADCSSL = ADCSSL_HIGH_SPEED;  // only read the pot, vbus and vpha
					ADCON2 = ADCON2_HIGH_SPEED;  // interrupt after three adc reads (adc interrupt frequency changes to 81.94 kHz)
					ADCON1bits.ADON = 1;		//Turn the ADC module back on 
				}
			}		
			break;
		case 4:
			if (vphc_filtered_sample < signal_average)
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();
				if (ThirtyDegreeTime < phase_delay)
                {
					ThirtyDegreeTime = phase_delay;
                }
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 1;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState++;
			}		
			break;
		case 5:
			if (vphb_filtered_sample > signal_average)
			{
				SixtyDegreeTime[ZeroCrossState] = TMR1;
				TMR1 = 0;
				ThirtyDegreeTime = ThirtyDegreeTimeAverage();
				if (ThirtyDegreeTime < phase_delay)
                {
					ThirtyDegreeTime = phase_delay;
                }
				PR3 = ThirtyDegreeTime - phase_delay;
				NextSectorState = 2;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				OneEightyDegreeTime[SpeedPtr&0x000F] = ThirtyDegreeTime*6;
				SpeedPtr++;
				ZeroCrossState = 0;
			}		
			break;
// States 6 - 9 implement the high speed mode of this algorithm.  Only one phase voltage is sampled. The sampling frequency
// is 81kHz when running in high speed mode.  
// State 6 - 9 thực hiện chế độ tốc độ cao của thuật toán này. Chỉ có một điện áp pha được lấy mẫu. Tần suất lấy mẫu
// là 81kHz khi chạy ở chế độ tốc độ cao.            
		case 6:
			// Wait in this state until it's safe to check for the next zero cross event
            //Đợi ở State này cho đến khi an toàn để kiểm tra sự kiện ZC tiếp theo
//			if (vpha_filtered_sample > (signal_average + 15))   
//				ZeroCrossState++;
			if (Sector == 2)
				ZeroCrossState++;
			break;
		case 7:
			if (vpha_filtered_sample < signal_average)   // signal is falling look for when it falls below center voltage
			{
				OneEightyDegreeTime[SpeedPtr&0x000F] = TMR1;
				TMR1 = 0;
				ThreeSixtyDegreeTime = OneEightyDegreeTime[SpeedPtr&0x000F] + OneEightyDegreeTime[(SpeedPtr-1)&0x000F];
				PR2 = ThreeSixtyDegreeTime/6;
				if ((ThreeSixtyDegreeTime>>2) < phase_delay)
                {
					PR3 = 0;
                }
				else
                {
					PR3 = (ThreeSixtyDegreeTime>>2) - phase_delay;
                }
				SpeedPtr++;
				NextSectorState = 4;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				ZeroCrossState++;
			}
			break;	
		case 8:
			// Wait in this state until it's safe to check for the next zero cross event 
//			if (vpha_filtered_sample < (signal_average - 15))
//				ZeroCrossState++;
			if (Sector == 5)
				ZeroCrossState++;
			break;		
		case 9:
			if (vpha_filtered_sample > signal_average)   // signal is falling look for when it falls below center voltage
			{
				OneEightyDegreeTime[SpeedPtr&0x000F] = TMR1;
				TMR1 = 0;
				ThreeSixtyDegreeTime = OneEightyDegreeTime[SpeedPtr&0x000F] + OneEightyDegreeTime[(SpeedPtr-1)&0x000F];
				PR2 = ThreeSixtyDegreeTime/6;
				if ((ThreeSixtyDegreeTime>>2) < phase_delay)
                {
					PR3 = 0;
                }
				else
                {
					PR3 = (ThreeSixtyDegreeTime>>2) - phase_delay;
                }
				SpeedPtr++;
				NextSectorState = 1;
				T3CONbits.TON = 1; 	
				IEC0bits.T3IE = 1;
				ZeroCrossState = 6;
				if (!ControlFlags.HighSpeedMode)
				{
					ZeroCrossState = 4;             // Go back to the appropriate state in low-speed mode
					NextSectorState = 0;            // The next timer 3 interrupt will initiate a commutation to Sector 0
					PR3 = (PR2>>1) -  phase_delay;  // load PR2 with 30 degree time minus the phase delay
					IEC0bits.T2IE = 0;
					ADCON1bits.ADON = 0;		// Turn ADC module off before modifying control bits;
					ADCSSL = ADCSSL_LOW_SPEED;  // read vphb and vphc in addition to the pot, vbus, and vpha
					ADCON2 = ADCON2_LOW_SPEED;  // interrupt every five samples (adc interrupts every 49kHz)
					ADCON1bits.ADON = 1;		// Turn ADC module back on
				}
			}
			break;
		default:
			ZeroCrossState = 0;
			break;
	}
}


/*---------------------------------------------------------------------
  Function Name: Commutate
  Description:   Commutates the motor to the next drive state
  Inputs:        sector
  Returns:       None
-----------------------------------------------------------------------*/
void Commutate(unsigned int sector)
{
	OVDCON = SectortoState[sector];			// Change PWM phase 
	OldSector = sector;						// Keep track of last sector (for error checking elsewhere)
	BlankingCount = blanking_count_initial;
}

/*---------------------------------------------------------------------
  Function Name: ThirtyDegreeTimeAverage()
  Description:   This functions takes the stored sixty degree times for an 
                  entire electrical revolution and commputs the average 30
	              degree time
  Inputs:        SixtyDegreeTime[6] (sixty degree times for an entire electrical revolution
  Returns:       The average 30 degree time
-----------------------------------------------------------------------*/
unsigned int ThirtyDegreeTimeAverage()
{
	unsigned int i;
	unsigned long temp; 
	temp = 0;
	for (i=0; i<6; i++)
	{
		temp += SixtyDegreeTime[i];
	}
	i = __builtin_divud(temp,12);		// same as divide by 6 and then divide by 2 (returning the average 30 degree time
	return i;
}