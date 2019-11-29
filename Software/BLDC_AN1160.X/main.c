#include "BLDC/bldc.h"
#include "BLDC/motor.h"
unsigned int  PWMticks;
unsigned char CommState;
unsigned char ADCCommState;
unsigned char adcBackEMFFilter;
unsigned int PhaseAdvance;
unsigned char BlankingCounter;

unsigned int MotorNeutralVoltage;
unsigned int MotorPhaseA;
unsigned int MotorPhaseB;
unsigned int MotorPhaseC;
unsigned int ComparatorOutputs;
unsigned int CommutationStatus;
unsigned int DesiredPWMDutyCycle;
unsigned int CurrentPWMDutyCycle;
unsigned char RampUpCommState;

unsigned int Timer2Value;
unsigned int Timer2Average;
unsigned int Timer1Value;

unsigned int ReferenceSpeed;

void PreCommutationState(void);
void OpenLoopController(void);
int16_t main()
{
    SYS_INIT();
    Flags.RotorAlignment = 0;   // Off ramup
    Flags.RunMotor = 0;
    while(1)
    {
        while(!S3);
        while(S3)
        {
            __delay_ms(100);            
        }
        T2CONbits.TON = 1;
        PWMCON1 = 0x0777;
        /*ROTOR ALIGNMENT SEQUENCE*/
		Flags.RotorAlignment = 1;		// TURN ON rotor alignment sequence
		Flags.RunMotor = 1;				// Indicating that motor is running
		CurrentPWMDutyCycle = MIN_DUTY_CYCLE;	//Init PWM values
		DesiredPWMDutyCycle = MIN_DUTY_CYCLE;	//Init PWM values
		PWMticks = 0;					//Init Rotor aligment counter
        /************* Rotor alignment sequence and ramp-up sequence ************/
		for(RampUpCommState=1;RampUpCommState<7;RampUpCommState++)
		{
            
			while(++PWMticks<MAX_PWM_TICKS)
            {
				OVDCON=PWM_STATE[RampUpCommState];
            }
			PWMticks = 0;				
		}   
		Flags.RotorAlignment = 0;		// TURN OFF rotor alignment sequence
		PWMticks = MAX_PWM_TICKS+1;		// RAMP UP for breaking the motor IDLE state
		__delay_ms(0);
        while(Flags.RunMotor)			// while motor is running
		{
		/****************** Stop Motor *********************************/			
			if (S3)							// if S3 is pressed
			{
				while (S3)				
                {// wait for key release
					__delay_ms(100);	
                }
				PWMCON1 = 0x0700;			// disable PWM outputs
  			    OVDCON = 0x0000;			// override PWM low.
				Flags.RotorAlignment = 0;	// turn on RAMP UP
				Flags.RunMotor = 0;			// reset run flag
				CurrentPWMDutyCycle = 1;	// Set PWM to the min value				
				//Initi speed measurement variables & timer
				T2CONbits.TON = 0;	// Stop TIMER2
				TMR2 = 0;			//Clear TIMER2 register
				Timer2Value = 0;
				Timer2Average = 0;								
               }
		}//end of motor running loop
    }
    return 0;
}
void __attribute__((__interrupt__,auto_psv)) _ADCInterrupt(void)
{
	ReferenceSpeed = ADCBUF0;	//ADC CH0 holds the POT value
	if(ReferenceSpeed < (MIN_DUTY_CYCLE*2)/3)
		ReferenceSpeed = (MIN_DUTY_CYCLE*2)/3;
	
	MotorPhaseA = ADCBUF1;		//ADC CH1 holds the Phase A value
	MotorPhaseB = ADCBUF2;		//ADC CH2 holds the Phase B value
	MotorPhaseC = ADCBUF3;		//ADC CH3 holds the Phase C value
	//Reconstrucs Voltage at the  Motor Neutral Point
	MotorNeutralVoltage = (MotorPhaseA + MotorPhaseB + MotorPhaseC)/3; 

	/********************* ADC SAMPLING & BMEF signals comparison ****************/
	if(BlankingCounter > BLANKING_COUNT){
		ComparatorOutputs = 0;						// Precondition all comparator bits as zeros
		if(MotorPhaseA > MotorNeutralVoltage)
			ComparatorOutputs += 1;					// Set bit 0 when Phase C is higher than Neutural
		if(MotorPhaseB > MotorNeutralVoltage)
			ComparatorOutputs += 2;					// Set bit 1 when Phase C is higher than Neutural
		if(MotorPhaseC > MotorNeutralVoltage)
			ComparatorOutputs += 4;					// Set bit 2 when Phase C is higher than Neutral
	}

	ADCON1bits.DONE = 0;
	IFS0bits.ADIF = 0;
}
void __attribute__((__interrupt__,auto_psv)) _PWMInterrupt(void)
{
	//Sets the ADC sampling point according to the PWM duty cycle
	if(CurrentPWMDutyCycle>160)
		SEVTCMPbits.SEVTCMP = CurrentPWMDutyCycle>>1;
	else if(CurrentPWMDutyCycle>76)
		SEVTCMPbits.SEVTCMP = CurrentPWMDutyCycle>>3;
	else
		SEVTCMPbits.SEVTCMP = 0;

	//Ramp-up period to detect the rotor position
	if(Flags.RotorAlignment && Flags.RunMotor)
		{
    	PDC1=PDC2=PDC3=CurrentPWMDutyCycle;
		++PWMticks;
		}
	//Check if the Ramp-up value is disabled, if so starts sensorless operation
	if (++PWMticks>MAX_PWM_TICKS)
		PreCommutationState();

	// Masking the BEMF signals according to the SECTOR in order to determine the ACTIVE BEMF signal
	// XOR operator helps to determine the direction of the upcoming zero-crossing slope
	BlankingCounter++;
	if(BlankingCounter > BLANKING_COUNT){
		if((ComparatorOutputs^ADC_XOR[ADCCommState])& ADC_MASK[ADCCommState])
			adcBackEMFFilter|=0x01;
		
		//Majority detection filter	
		adcBackEMFFilter = ADC_BEMF_FILTER[adcBackEMFFilter];
		if (adcBackEMFFilter&0b00000001)
			PreCommutationState();	
	}
	_PWMIF = 0;
}
void __attribute__((__interrupt__,auto_psv)) _T1Interrupt(void)
{

	OVDCON=PWM_STATE[ADCCommState];

	BlankingCounter = 0;

	IFS0bits.T1IF = 0; 		// Clear Timer 1 Interrupt Flag
	T1CONbits.TON = 0;		// Stop TIMER1
	TMR1 = 0;
}
void PreCommutationState(void)
{
	// Calculate the time proportional to the 60 electrical degrees
	T2CONbits.TON = 0;	// Stop TIMER2
	Timer2Average = ((Timer2Average + Timer2Value + TMR2)/3);
	Timer2Value = TMR2;
	TMR2 = 0;
	T2CONbits.TON = 1;	// Start TIMER2
	
	//Calculate the delay in TIMER1 counts proportional to the Phase Adv angle
	PhaseAdvance = ((Timer2Average*PHASE_ADVANCE_DEGREES)/60);
	
	// Calculate the time proportional to the 30 electrical degrees
	// Load the TIMER1 with  the TIMER1 counts porportional to 30 deg	minus the PHASE ADV angle delay
	Timer1Value = (((Timer2Average)>>1)-PhaseAdvance);
	if(Timer1Value>1)
		PR1 = Timer1Value;	
	else
		PR1 = Timer1Value = 1;	
	
	// Start TIMER1	
	T1CONbits.TON = 1;	  	

	//disabling rotor alignment & ramp-up sequence
	PWMticks = 0;
	RampUpCommState = 7;

	//if Motor is runnining in sensorless mode and the PI button is ON on DMIC window 
	//then the PI controller is enabled if the PI button is OFF then motor runs in open loop
	if((!Flags.RotorAlignment) && Flags.RunMotor){

#ifdef CLOSELOOPMODE
		SpeedPILoopController();
#else	
		OpenLoopController();
#endif
	}
		
	// Change The Six-Step Commutation Sector
	adcBackEMFFilter=0;
	if (++ADCCommState>6)
		ADCCommState=1;
}
void OpenLoopController(void)
{
	//PWM duty cycle = pot value *3/2
	DesiredPWMDutyCycle = (ReferenceSpeed*3)/2;
	//Update the duty cycle according to the POT value, a POT follower is implemented here
	if(CurrentPWMDutyCycle != DesiredPWMDutyCycle)
		{
		if(CurrentPWMDutyCycle < DesiredPWMDutyCycle)
			CurrentPWMDutyCycle++;		
		if(CurrentPWMDutyCycle > DesiredPWMDutyCycle)
			CurrentPWMDutyCycle--;
		}

	// Max and Min PWM duty cycle limits
	if (CurrentPWMDutyCycle < MIN_DUTY_CYCLE)
		CurrentPWMDutyCycle = MIN_DUTY_CYCLE;
	if (CurrentPWMDutyCycle > MAX_DUTY_CYCLE)
		CurrentPWMDutyCycle = MAX_DUTY_CYCLE;
	//Assigning new duty cycles to the PWM channels
	PDC1 = CurrentPWMDutyCycle;
	PDC2 = CurrentPWMDutyCycle;
	PDC3 = CurrentPWMDutyCycle;

} 