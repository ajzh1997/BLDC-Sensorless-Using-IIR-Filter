/* 
 * File:   RTDMUSER.h
 * Author: Duy Ngo
 *
 * Created on December 3, 2019, 11:24 AM
 */
#ifndef RTDMUSER_H
#define RTDMUSER_H

#define YES  1
#define NO 	 0
/************************************** RTDM DEFINITIONS  ***************************************/
#define RTDM_FCY	 	29491200	//This define has to be the system operating freq, this 
									//value is used to calculate the value of the BRG register
#define RTDM_BAUDRATE	115200		//This is the desired baudrate for the UART module to be 
									//used by RTDM
#define RTDM_UART			2		// This is the UART module to be used by RTDM. It has only
									// two possible values: 1 or 2
#define RTDM_UART_PRIORITY	3		//This the UART RX interrupt priority assigned to receive
									// the RTDM messages
#define RTDM_RXBUFFERSIZE	32		// This is the buffer size used by RTDM to handle messaages 
#define RTDM_MAX_XMIT_LEN   0x1000	//This the size in bytes of the max num of bytes allowed in 
									//the RTDM protocol Frame
#define RTDM_POLLING		YES		// This defines the mode that RTDM will be operating in 
									//user's application. If it is YES then the user should place the 
									//RTDM_ProcessMsgs()	function in the main loop. 
									//In order to make sure that the messages are being preoccessed
									// it is recommended that the main loop always polls this 
									//function as fast as possible						
#define RTDM_MIN_CODE_SIZE	YES		//When defined causes the RTDM library to build  without 
									//including a pre-calculated polynomial  table for the CRC algorythim. 
									//This saves 768  bytes of code space. 
/*************************************************************************************************/		
#endif	/* RTDMUSER_H */

