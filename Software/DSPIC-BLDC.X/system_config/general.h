/* 
 * File:   general.h
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 12:16 PM
 */

#ifndef GENERAL_H
#define	GENERAL_H
#include "xc.h"
#define XTAL_FRQ 7372800    // Thạch Anh trên Board
#define PLL         16      // Bộ nhân tần x16
#define FCY     ((XTAL_FRQ*PLL)/4)  // Tần số máy 

#define FPWM 20000  // Tần số PWM
// FULL_DUTY the equivalent of 100% duty cycle
//#define	FULL_DUTY (2*FCY/FPWM)
#define FULL_DUTY 2900UL
// Timer 1,2, and 3 prescaler
#define TMR1_PRESCALER 8UL

// Specify the maximum number of degrees of phase advance
#define MAX_PHASE_ADVANCE 30

// Filter phase delay is the delay in the filtered signal compared to the actual signal
// FILTER_PHASE_DELAY is equal to the Group Delay (as a function of filtered signal frequency)
//  multiplied by Fcy divided by the Timer 1 prescaler.
// Assuming a maximum speed of 100000 electrical revs per minute the frequency we are concerned 
//  with is 1666 revs per second.  This is the frequency of the filtered signal this speed
// Refering to the group delay plot in the dsPIC Filter design tool the delay at 1333 Hz
//  is approx 90 us.  At a frequency of 0 the phase delay is 85us.  We can ignore the 5us
//  and just account for the worst case scenerio
// FILTER_PHASE_DELAY = 90us*FCY/TMR1_PRESCALER
#define INVERSE_GROUP_DELAY 11111 // 1/(90us)  
#define FILTER_PHASE_DELAY FCY/TMR1_PRESCALER/INVERSE_GROUP_DELAY

// time through ADC interrupt divided by prescaler
#define PROCESSING_DELAY 410/TMR1_PRESCALER

// time through ADC interrupt divided by prescaler for high speed mode
#define PROCESSING_DELAY_HS 270/TMR1_PRESCALER

// ADC Configuration Values for low and high speed modes
#define ADCON2_LOW_SPEED 0x0410     // 410channel scan for CH0, MUX A only, 5 conversions per interrupt, Vrefs are AVdd and AVss
#define ADCON2_HIGH_SPEED 0x0408    // 408channel scan for CH0, MUX A only, 3 conversions per interrupt, Vrefs are AVdd and AVss
#define ADCSSL_LOW_SPEED 0x01C5     // scan AN7, AN11, AN12, AN13 and AN14
#define ADCSSL_HIGH_SPEED 0x0045    // scan AN7, AN11, AN12

// Define the ADC result buffers for the analog pins
#define POTBUF	ADCBUF0		//AN7
#define VBUSBUF	ADCBUF1		//AN11
#define VPHABUF	ADCBUF2		//AN12
#define VPHBBUF	ADCBUF3		//AN13
#define VPHCBUF ADCBUF4		//AN14

// PID Speed Control Loop enable.  
// Uncomment this define statement if the user desires that the PID Speed Control Loop be enabled
#define PID_SPEED_LOOP
#endif	/* GENERAL_H */

