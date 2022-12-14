/***********************************************************************
 * This initializes CAN0 module and contains functions for sending and
 * 		receiving messages.
 *
 * Author:  Zac Lynn
 * Version: Phase2.1
 * See: 	https://www.nxp.com/docs/en/application-note/AN5413.pdf
 **********************************************************************/
#ifndef CAN_H_
#define CAN_H_

#define DEVICE_ID 1 															// Can be between 0 and 255. Used to identify the device when multiple shifters are connected to a single CAN system

uint32_t RxCODE; 																// Received message buffer code
uint32_t RxID; 																	// Received message ID
uint32_t RxLENGTH; 																// Received message number of data bytes
unsigned char RxDATA[64]; 														// Received message data, max size is 64 bytes
uint32_t RxTIMESTAMP; 															// Received message time


/***********************************************************************
 * This function initializes CAN0 in CANFD mode with BRS initially off.
 * 		Message buffers are set to 64 data-bytes, however when using
 * 		the transmit function, the DLC is set depending on the length of
 * 		the provided data. Keeping the message buffers at 64-bytes avoids
 * 		accidental overwrites of future message buffers.
 *
 * @param void
 * @return void
 **********************************************************************/
void CAN0_init(void);


/***********************************************************************
 * This function enables bit rate switching for CAN0.
 *
 * @param void
 * @return void
 **********************************************************************/
void CAN0_BRSen(void);


/***********************************************************************
 * This function disables bit rate switching for CAN0.
 *
 * @param void
 * @return void
 **********************************************************************/
void CAN0_BRSdis(void);


/***********************************************************************
 * Given a byte array, the length of data in bytes, and the message type
 * 		this function transmists a message over CAN0 with ascii '0's
 * 		added if data length is not a standard DLC length.
 *
 * @param data Byte array to send over CAN
 * @param len Length of byte array
 * @param msg_type Which type of diagnostics message is being sent
 * 			 		     0 - initialization msg
 * 			 		     3 - good button state change reporting
 * 			 		     4 - bad button state change reporting
 *
 * @return void
 **********************************************************************/
void CAN0_transmit_msg(const unsigned char data[], int len, uint8_t msg_type);


/***********************************************************************
 * Given a message buffer, this function reads the message from that
 * 		buffer into RxDATA global array. RxID, RxLENGTH, and RxTIMESTAMP
 * 		are also set.
 *
 * @param buff Which message buffer to read from
 *
 * @return void
 **********************************************************************/
void CAN0_receive_msg(int buff);


/***********************************************************************
 * Initializes CAN0 pins
 *
 * @param void
 * @return void
 **********************************************************************/
void port_init(void);


#endif /* CAN_H_ */

