/* 
 * File:   motor.h
 * Author: Duy Ngo
 *
 * Created on November 29, 2019, 2:07 PM
 */
const unsigned int PWM_STATE[]	=	{0x0000,0x2001,0x2004,0x0204,0x0210,0x0810,0x0801,0x0000};
const unsigned int ADC_MASK[8]	=   {0x0000,0x0002,0x0001,0x0004,0x0002,0x0001,0x0004,0x0000};
const unsigned int ADC_XOR[8]	=   {0x0000,0x0000,0xFFFF,0x0000,0xFFFF,0x0000,0xFFFF,0x0000};
const unsigned char ADC_BEMF_FILTER[64]={
    0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E,
	0x20,0x22,0x24,0x26,0x28,0x2A,0x2C,0x2E,0x01,0x01,0x01,0x36,0x01,0x3A,0x3C,0x3E,
	0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x01,0x01,0x01,0x16,0x01,0x1A,0x1C,0x1E,
	0x01,0x01,0x01,0x26,0x01,0x2A,0x2C,0x2E,0x01,0x01,0x01,0x36,0x01,0x3A,0x3C,0x3E
};
struct 
{
	unsigned RunMotor       : 	1;
	unsigned RotorAlignment :   1;
    unsigned MotorStartup 	:   1;	
	unsigned PushButtonS4   :   1;
	unsigned PushButtonS5   :   1;
	unsigned unused         :	11;     
}Flags;



struct 
{
   	unsigned PhaseAOutput : 	1;
	unsigned PhaseBOutput : 	1;
	unsigned PhaseCOutput : 	1;
	unsigned unused 	  :		13;
}	Comparator;
/********************* Motor Control Varaibles *********************************/		
extern unsigned int  PWMticks;
extern unsigned char CommState;
extern unsigned char ADCCommState;
extern unsigned char adcBackEMFFilter;
extern unsigned int PhaseAdvance;
extern unsigned char BlankingCounter;

extern unsigned int MotorNeutralVoltage;
extern unsigned int MotorPhaseA;
extern unsigned int MotorPhaseB;
extern unsigned int MotorPhaseC;
extern unsigned int ComparatorOutputs;
extern unsigned int CommutationStatus;
extern unsigned int DesiredPWMDutyCycle;
extern unsigned int CurrentPWMDutyCycle;
extern unsigned char RampUpCommState;

extern unsigned int Timer2Value;
extern unsigned int Timer2Average;
extern unsigned int Timer1Value;
extern unsigned int ReferenceSpeed;
#define SPEEDMULT	94016			// Factor used to calculated speed PWM TIMEBASE = FCY/2, TIMER2 TIMEBASE / FCY/64
#define INDEX		1				// Commutation base index
#define DEBOUNCE_DELAY 	20			// Push button debounce delay, expressed in millisecond

/********************* Motor Control Definitions *********************************/
/*START-UP SEQUENCE PARAMETERS*/	// Maximum PWM pulses applied to motor 						
#define MAX_PWM_TICKS 1230 		//for detecting initial rotor position
#define RAM_UP_DELAY  0    	 	// Delay for the ramp up sequence, expressed in millisecond
#define MAX_DUTY_CYCLE 1400  	// 100% duty cycle PDC1=PDC2=PDC3 = 1469
#define MIN_DUTY_CYCLE 40	 	// 2.7% duty cycle PDC1=PDC2=PDC3 = 72
#define PHASE_ADVANCE_DEGREES 25//Phase advance angles to get the best motor performance
#define BLANKING_COUNT 5		//Blanking count expressed in PWM periods used to avoid false zero-crossing detection after commutating motor
#define POLEPAIRS 5			 	// Number of pole pairs of the motor