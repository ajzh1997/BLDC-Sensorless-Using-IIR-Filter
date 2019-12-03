#include "../bldc.h"
#include "hardware.h"
void SlowEvent(void)
{
	volatile struct {
		unsigned Button_S1:1;
		unsigned Button_S2:1;
		unsigned :12;
	} DebounceFlags;
	ControlFlags.SlowEventFlag = 0;
	if(BUTTON_S1 && !DebounceFlags.Button_S1 && RunMode == MOTOR_OFF) 
	{
		DebounceFlags.Button_S1 = 1;
		RunMode = SENSORLESS_INIT;
	}
	if(BUTTON_S2 && !DebounceFlags.Button_S2 && RunMode != MOTOR_OFF)
	{
		DebounceFlags.Button_S2 = 1;
		RunMode = MOTOR_OFF;
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
}
