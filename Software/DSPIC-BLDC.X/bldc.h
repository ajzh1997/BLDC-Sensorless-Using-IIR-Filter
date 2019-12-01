#include "system_config/general.h"
#include "system_config/hardware.h"
#include "system_config/turning.h"
#include "system_config/medium_event.h"
#include "system_config/slow_event.h"
#include "driver/uart.h"
#include "driver/MCPWM.h"
#include "driver/adc.h"
#include "dsp.h"


//#define SNAPSHOT
// Function declarations
void Commutate(unsigned int sector);

// General definitions
#define NO_OF_RUN_MODES 5
	
// algorithm state definitions (Loaded in RunMode)
#define MOTOR_OFF 0
#define SENSORLESS_INIT 1
#define SENSORLESS_START 2
#define SENSORLESS_RUNNING 3
#define HALL_SENSOR_MODE 4

// sensorless start mode state definitions (loaded into SensorlessStartState)
#define LOCK1 0
#define LOCK2 1
#define RAMP_INIT 2
#define RAMP 3

// This bit structure provides mode flags
struct ControlFlags{
        unsigned	RunMotor:1; 	
        unsigned	HighSpeedMode:1;
        unsigned 	TakeSnapshot:1;
		unsigned 	MediumEventFlag:1;
		unsigned	SlowEventFlag:1;
		unsigned    SpeedControlEnable:1;
		unsigned	EnablePotentiometer:1;
        unsigned    UARTEventFlag:1;
        unsigned   	:8;
}; 

extern volatile struct ControlFlags ControlFlags;

extern unsigned int ZeroCrossPeriodFalling[8];
extern unsigned int ZeroCrossPeriodRising[8];
extern unsigned int RunMode;
extern unsigned int SensorlessStartState;
extern unsigned int phase_advance;
extern int signal_average;
extern unsigned int pot;
extern int vbus;
extern int Sector;
extern unsigned long Speed;
extern unsigned int OneEightyDegreeTime[16];
// These are the override control states for soft chopping with
// synchronous rectification.  BEMF zero cross will be referenced 
// to 0V DC.

#define		SS_STATE0 	0x0304		// A complementary, B bottom
#define		SS_STATE1 	0x0310		// A complementary, C bottom		 
#define		SS_STATE2 	0x0C10		// B complementary, C bottom
#define		SS_STATE3 	0x0C01		// B complementary, A bottom
#define		SS_STATE4 	0x3001		// C complementary, A bottom
#define  	SS_STATE5   0x3004		// C complementary, B bottom

// These are the override control states for hard chopping.
// BEMF zero crossing events will be referenced to Vbus/2.
#define		STATE0 	0x0600		// A top, B bottom, C BEMF
#define		STATE1 	0x1200		// A top, C bottom, B BEMF		 
#define		STATE2 	0x1800		// B top, C bottom, A BEMF
#define		STATE3 	0x0900		// B top, A bottom, C BEMF
#define		STATE4 	0x2100		// C top, A bottom, B BEMF
#define  	STATE5  0x2400		// C top, B bottom, A BEMF

//#define		STATE0 	0x2001		// A top, B bottom, C BEMF
//#define		STATE1 	0x2004		// A top, C bottom, B BEMF		 
//#define		STATE2 	0x0204		// B top, C bottom, A BEMF
//#define		STATE3 	0x0210		// B top, A bottom, C BEMF
//#define		STATE4 	0x0810		// C top, A bottom, B BEMF
//#define  	STATE5  0x0801		// C top, B bottom, A BEMF
// Sector 0 -- looking for FALLING edge Phase C
// Sector 1 -- looking for RISING edge Phase B
// Sector 2 -- looking for FALLING edge Phase A		
// Sector 3 -- looking for RISING edge Phase C
// Sector 4 -- looking for FALLING edge Phase B
// Sector 5 -- looking for RISING edge Phase A
