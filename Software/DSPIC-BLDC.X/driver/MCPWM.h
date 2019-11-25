/* 
 * File:   MCPWM.h
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 12:41 PM
 */

#ifndef MCPWM_H
#define	MCPWM_H
#include "../system_config/general.h"
#define 	PERIOD	FCY/FPWM
#define		DEAD_TIME	.000002          // in seconds
#define		DT_COUNT	DEAD_TIME*FCY    // in PWM counts

// PWM Fault protection options
// Load FLTACON below with one of thes options
#define CYCLE_BY_CYCLE_PROTECTION 0x0087;		// PWM1L/H, PWM2L/H, PWM3L/H have cycle by cycle current limiting enabled
#define FAULT_CAUSES_PWM_SHUTDOWN 0x0007;       // An over-current fault shuts down PWM1L/H, PWM2L/H, PWM3L/H   
#define NO_FAULT_PROTECTION       0x0000;       // No over-current protection
void MCPWM_Initialize(void);
#endif	/* MCPWM_H */

