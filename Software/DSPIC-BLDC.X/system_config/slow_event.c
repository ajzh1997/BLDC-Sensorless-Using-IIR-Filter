#include "../bldc.h"
#include "hardware.h"
#include "stdio.h"
void SlowEvent(void)
{
	volatile struct {
		unsigned Button_S1:1;
		unsigned Button_S2:1;
		unsigned :12;
	} DebounceFlags;

	ControlFlags.SlowEventFlag = 0;

// Check button presses
//	if(BUTTON_S1 && !DebounceFlags.Button_S1)
//	{
//		DebounceFlags.Button_S1= 1;
//		if (RunMode != SENSORLESS_RUNNING)
//		{
//			ResetPowerModule();
//			GetParameters();
//		}
//		ControlFlags.TakeSnapshot = 1;
//		RunMode = HALL_SENSOR_MODE;	
//	}

	if(BUTTON_S1 && !DebounceFlags.Button_S1 && RunMode == MOTOR_OFF) 
	{
		DebounceFlags.Button_S1 = 1;
		RunMode = SENSORLESS_INIT;
        printf("SENSORLESS_INIT\r\n");
	}

//	if(BUTTON_S6 && !DebounceFlags.Button_S6) 
//	{
//		DebounceFlags.Button_S6 = 1;
//		ControlFlags.TakeSnapshot = 1;
//		RunMode = SENSORLESS_RUNNING;
//	}

	if(BUTTON_S2 && !DebounceFlags.Button_S2 && RunMode != MOTOR_OFF)
	{
		DebounceFlags.Button_S2 = 1;
		RunMode = MOTOR_OFF;
        printf("MOTOR OFF\r\n");
		SensorlessStartState = 0;
	}

	if (DebounceFlags.Button_S1)
	{
		if (!BUTTON_S1) DebounceFlags.Button_S1 = 0;
	}

	if (DebounceFlags.Button_S2)
	{
		if (!BUTTON_S2) DebounceFlags.Button_S2 = 0;
	}

//	if (DebounceFlags.Button_S6)
//	{
//		if (!BUTTON_S6) DebounceFlags.Button_S6 = 0;
//	}
//
//	if (DebounceFlags.Button_S7)
//	{
//		if (!BUTTON_S7) DebounceFlags.Button_S7 = 0;
//	}
	
//	switch (RunMode)
//	{
//		case MOTOR_OFF:
//			MOTOR_OFF_LED = 1;
//			SENSORLESS_LED = 0;
//			START_LED = 0;		
//			HALL_MODE_LED = 0;	
//			break;
//		case SENSORLESS_RUNNING:
//			MOTOR_OFF_LED = 0;
//			SENSORLESS_LED = 1;
//			START_LED = 0;		
//			HALL_MODE_LED = 0;	
//			break;
//		case SENSORLESS_START:
//		case SENSORLESS_INIT:
//			MOTOR_OFF_LED = 0;
//			SENSORLESS_LED = 0;
//			START_LED = 1;		
//			HALL_MODE_LED = 0;	
//			break;
//		case HALL_SENSOR_MODE:
//			MOTOR_OFF_LED = 0;
//			SENSORLESS_LED = 0;
//			START_LED = 0;		
//			HALL_MODE_LED = 1;	
//			break;
//	}
//	if (ControlFlags.HighSpeedMode)
//		HIGH_SPEED_LED = 1;
//	else
//		HIGH_SPEED_LED = 0;

}
