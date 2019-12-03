#include "../bldc.h"
#include "turning.h"
#include "stdio.h"
unsigned int DegreesAdvanced;   // Degrees of Phase advance based on speed
unsigned int ElectricalSpeed;	// Electrical Speed in electrical revolutions per second

void MediumEvent(void)
{
	static int HighLowCntr = 0;
	static unsigned int ramp_timer; 
	static unsigned int ramp_demand = 0;
	static unsigned int ramp_speed;
	static int motor_speed;
	static unsigned long ThreeSixtyDegreeAverage;
	unsigned int i;
    
	ControlFlags.MediumEventFlag = 0;
	{
		PWMCON2bits.UDIS = 1;  			// disable the duty cycle update

		switch (RunMode)
		{
			case MOTOR_OFF:
				PDC1 = 0;
				PDC2 = 0;
				PDC3 = 0;
				break;
			case SENSORLESS_RUNNING:	
				DegreesAdvanced =  __builtin_divud(((Speed-phase_advance_start) * phase_advance_slope),1000);
				if (DegreesAdvanced > MAX_PHASE_ADVANCE)	// Limit Phase advance to MAX_PHASE_ADVANCE
                {
					DegreesAdvanced = MAX_PHASE_ADVANCE;	
                }
				if (DegreesAdvanced <= 0)	// Because DegreesAdvanced is a divisor (in the next few line) alway have at least 1 degree of advance
                {
					phase_advance = 0;
                }
				else if (Speed > phase_advance_start)
                {
					phase_advance = __builtin_divud(((unsigned long)ThreeSixtyDegreeAverage*DegreesAdvanced),360);
                }
				else
                {
					phase_advance = 0;
                }
				if (ElectricalSpeed > (CrossOverERPS + 50))  // If electrical speed per second is over 300 then consider going to high speed mode
                {
                    HighLowCntr++;	
                }
				if (ElectricalSpeed < (CrossOverERPS - 50))	// If electrical speed per second is under 250 then consider going to low speed mode
                {
                    HighLowCntr--;
                }
				if (HighLowCntr > 20)	    // The device only goes into high speed mode when the HighLowCntr reaches a certain magnitude
				{
					ControlFlags.HighSpeedMode = 1;
					HighLowCntr = 0;
				}
				if (HighLowCntr < -20)		// The device only goes into low speed mode when the HighLowCntr reaches a certain negative magnitude
				{
					ControlFlags.HighSpeedMode = 0;
					HighLowCntr = 0;
				}                
			// no "break" continue on into HALL_SENSOR_MODEE
			case HALL_SENSOR_MODE:
				ThreeSixtyDegreeAverage = 0;
				for (i=0; i<16; i++)
				{
					ThreeSixtyDegreeAverage += OneEightyDegreeTime[i];
				}
				ThreeSixtyDegreeAverage = ThreeSixtyDegreeAverage >> 3;  
				// Electrical Speed is equal to the processor frequency divided by 
				//  the Timer prescaler divied by the number of timer counts for 
				//  360 degrees. This is electrical revolutions per second
                // Tốc độ điện bằng tần số bộ xử lý chia cho
                // bộ đếm trước bộ đếm thời gian được chia cho số lượng bộ đếm thời gian cho
                //  360 độ. Đây là vòng quay điện mỗi giây
				ElectricalSpeed = (__builtin_divud(FCY,((unsigned int)ThreeSixtyDegreeAverage)))/TMR1_PRESCALER;
				// Speed in RPM is equivalent to the electrical revolutions per 
				//	second times 60 divided by the number of pole pairs (poles 
				//  divided by 2 - hence the 2 in the numerator of the following 
				//  equation.)
                // Tốc độ tính bằng RPM tương đương với số vòng quay điện trên mỗi
                // lần thứ hai 60 chia cho số cặp cực (cực
                // chia cho 2 - do đó 2 trong tử số của sau
                // phương trình.)
				Speed = (unsigned long)(__builtin_divud(((unsigned long)ElectricalSpeed*120),NoOfMotorPoles));
				if (ControlFlags.SpeedControlEnable)
				{
					PIDStructure.controlReference = RPM_converter_constant * pot;
					PIDStructure.measuredOutput = Speed;
					PID(&PIDStructure);
					if (PIDStructure.controlOutput < 0)
                    {
						PIDStructure.controlOutput = 0;
                    }
					if (PIDStructure.controlOutput > 32439) 
					{
						PDC1 = FULL_DUTY;
						PIDStructure.controlOutput = 32439;
					}
					else
                    {
						PDC1 = __builtin_divud((FULL_DUTY*PIDStructure.controlOutput),32439);
                    }
					PDC2 = PDC1;
					PDC3 = PDC1;
				}
				else if (ControlFlags.EnablePotentiometer)
				{   
                    static uint16_t DesiredPWMDutyCycle = 0;
                    static uint16_t CurrentPWMDutyCycle = 0;
                    CurrentPWMDutyCycle = PDC1;
                    DesiredPWMDutyCycle = (unsigned int) ((unsigned long)pot*FULL_DUTY/0x3FF);
                    if(CurrentPWMDutyCycle != DesiredPWMDutyCycle)
                    {
                        if(CurrentPWMDutyCycle < DesiredPWMDutyCycle)
                        {
                            CurrentPWMDutyCycle++;		
                        }
                        if(CurrentPWMDutyCycle > DesiredPWMDutyCycle)
                        {
                            CurrentPWMDutyCycle--;
                        }
                    }
                    if (CurrentPWMDutyCycle < 0)
                    {
                        CurrentPWMDutyCycle = 0;
                    }
                    if (CurrentPWMDutyCycle > FULL_DUTY)
                    {
                        CurrentPWMDutyCycle = FULL_DUTY;
                    }
					//PDC1 = (unsigned int) ((unsigned long)pot*FULL_DUTY/0x3FF);
                    PDC1 = CurrentPWMDutyCycle;
                    PDC2 = CurrentPWMDutyCycle;
                    PDC3 = CurrentPWMDutyCycle;
				}
				break;
			case SENSORLESS_INIT:
				GetParameters();
				signal_average = (vbus >> 1);
				// once the parameters are set up, initiate the sensorless start
                // một khi các tham số được thiết lập, bắt đầu khởi động không cảm biến
				RunMode = SENSORLESS_START;
				HighLowCntr = 0;
				ControlFlags.HighSpeedMode = 0;
				break;
			case SENSORLESS_START:
				switch (SensorlessStartState)
				{
					case LOCK1:
						PDC1 = lock1_demand;
						PDC2 = PDC1;
						PDC3 = PDC1;
						if (--lock1_duration == 0)
						{				
							Sector++;						// Increment Sector (there are 6 total)
							if(Sector > 5) Sector = 0;
								Commutate(Sector);			// Change the PWM output for next sector
							SensorlessStartState++;
						}
						break;
					case LOCK2:
						PDC1 = lock2_demand;
						PDC2 = PDC1;
						PDC3 = PDC1;
						if (--lock2_duration == 0)
						{				
							Sector++;						// Increment Sector (there are 6 total)
							if(Sector > 5) Sector = 0;
								Commutate(Sector);			// Change the PWM output for next sector
							SensorlessStartState++;
						}
						break;
					case RAMP_INIT:
						ramp_timer = 0;	
						ramp_demand = ramp_start_demand;
						motor_speed = ramp_start_speed;   // countdown timer to determine when it's time to switch to the next sector 
						ramp_speed = ramp_start_speed;  // current ramp speed in PWM interrupts per sector
						SensorlessStartState++;	
						break;
					case RAMP:  // sau khi quá thời gian ram, khởi động chế độ  ssl
						if (ramp_timer++ >= ramp_duration)
						{
							//SensorlessStartState = 0;
							RunMode = SENSORLESS_RUNNING;
							T3CONbits.TON = 0; 	
							IEC0bits.T3IE = 0;
							IFS0bits.T1IF = 0;  
							IEC0bits.T1IE = 1;	
                            ramp_timer = ramp_duration;
						}
						else
						{
							// check for demand change
							if (ramp_timer%ramp_demand_rate == 0) // lực tăng theo thời gian
                            {
								ramp_demand++;
							}
                            PDC1 = (unsigned int) ramp_demand;
							PDC2 = PDC1;
							PDC3 = PDC1;
							// check for speed change
							if (ramp_timer%ramp_speed_rate == 0)
							{
								motor_speed--;
								ramp_speed--;
							}
							if (motor_speed-- <= 0)
							{
								motor_speed = ramp_speed;
								Sector++;						// Increment Sector (there are 6 total)
								if(Sector > 5) Sector = 0;
								Commutate(Sector);			// Change the PWM output for next sector
							}
						}
						break;
					default:
						SensorlessStartState = 0;
						break;			
				}
				break;
			default:
				if (RunMode > (NO_OF_RUN_MODES - 1))
				RunMode = 0;
			break;
		}
		PWMCON2bits.UDIS = 0;  			// enable the duty cycle update

	}
}