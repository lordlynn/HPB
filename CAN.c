#include "S32K146.h"
#include "CAN.h"
#include "stdio.h"


int MSG_BUF_SIZE = 18;															// Msg Buffer Size. ( 2 hdr + 16 data= 4 words)
int BRS = 0;																	// BRS enabled or disabled
int FD = 0x1;																	// Use FD or CAN 2.0B. BY DEFAULT THIS IS ALWAYS SET TO CANFD


void CAN0_init(void) {
	int i = 0;
	PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_CGC_MASK; 						// CGC=1: enable clock to FlexCAN0
	CAN0->MCR |= CAN_MCR_MDIS_MASK; 											// MDIS=1: Disable module before selecting clock
	CAN0->CTRL1 |= CAN_CTRL1_CLKSRC_MASK; 										// CLKSRC=1: Clock Source = Peripheral clock (80MHz)
	CAN0->MCR &= ~CAN_MCR_MDIS_MASK; 											// MDIS=0; Enable module config. (Sets FRZ, HALT)

	while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT));		// Good practice: wait for FRZACK=1 on freeze mode entry/exit

	CAN0->CTRL1 = 0;															// Timing variables set in CBT so nothing from example to set here
	CAN0->CTRL2 = (1 << 12);													// Use CANFD standard ISO 11898-1
	// CLK is 16MHz -> TQ = 62.5ns -> 1 / (500Kbits/s  *  62.5ns) = 32
	// So to set bit rate at 500k use 32 tq per bit.
	// synch = 1, prop = 12, seg1 = 12, seg2 = 7
	CAN0->CBT = 0x806139C7;

	CAN0->FDCTRL = 0x000F0000;													// Bit rate switching initially off, 64 data bytes,TDC disabled
	BRS = 0;																	// BRS flag is true when BRS is enabled

	// CLK is 8MHz -> TQ = 62.5ns -> 1 / (2Mbits/s  *  62.5ns) = 8
	// So to set bit rate at 2Mbits/s use 8 tq per bit.
	// synch = 1, prop = 3, seg1 = seg2 = 2
	CAN0->FDCBT = 0x00310C41;

	for(i=0; i < 128; i++ ) { 													// CAN0: clear all 128 RAM words
		CAN0->RAMn[i] = 0;
	}

	for(i=0; i < 16; i++ ) { 													// In FRZ mode, init CAN0 16 msg buf filters
		CAN0->RXIMR[i] = 0xFFFFFFFF;											// Check all ID bits for incoming messages
	}

	CAN0->RXMGMASK = 0x1FFFFFFF; 												// Global acceptance mask: check all ID bits

	// Setup message buffer for to receive messages
	CAN0->RAMn[4*MSG_BUF_SIZE + 0] = 0x04000000; 								// Msg Buf 4, word 0: Enable for reception
	CAN0->RAMn[5*MSG_BUF_SIZE + 0] = 0x04000000;
	/* EDL,BRS,ESI=0: CANFD not used */
	/* CODE=4: MB set to RX inactive */
	/* IDE=0: Standard ID */
	/* SRR, RTR, TIME STAMP = 0: not applicable */

	CAN0->RAMn[4*MSG_BUF_SIZE + 1] = (0x100 | DEVICE_ID) << 18; 				// Msg Buf 4, word 1: Standard ID = 0x01xx
	CAN0->RAMn[5*MSG_BUF_SIZE + 1] = (0x200 | DEVICE_ID) << 18;


	CAN0->MCR = 0x0000081F;														// Set Last Message Buffer limit and enable CAN FD, exits freeze mode
	while ((CAN0->MCR && CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT);			// Good practice: wait for FRZACK to clear (not in freeze mode)
	while ((CAN0->MCR && CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT);			// Good practice: wait for NOTRDY to clear (module ready)

	port_init();
}


void CAN0_BRSen(void) {
	BRS = 0x1;

	CAN0->MCR |= (1 << 30) | (1 << 28);											// Enter freeze mode, FRZ and HALT bits
	while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT));		// Good practice: wait for FRZACK=1 on freeze mode entry/exit

	CAN0->FDCTRL |= 0x80000000;													// Enables bit rate switching, 64-byte message buffer

	CAN0->MCR &=~ (1 << 30) | (1 << 28);										// Exit freeze mode
	while ((CAN0->MCR && CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT);			// Good practice: wait for FRZACK to clear (not in freeze mode)
	while ((CAN0->MCR && CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT);			// Good practice: wait for NOTRDY to clear (module ready)
}

void CAN0_BRSdis(void) {
	BRS = 0x0;

	CAN0->MCR |= (1 << 30) | (1 << 28);										 	// Enter freeze mode, FRZ and HALT bits
	while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT));		// Good practice: wait for FRZACK=1 on freeze mode entry/exit

	CAN0->FDCTRL &=~ 0x80000000;												// Diables bit rate switching, 8 data bytes

	CAN0->MCR &=~ (1 << 30) | (1 << 28);										// Exit freeze mode
	while ((CAN0->MCR && CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT);			// Good practice: wait for FRZACK to clear (not in freeze mode)
	while ((CAN0->MCR && CAN_MCR_NOTRDY_MASK) >> CAN_MCR_NOTRDY_SHIFT);			// Good practice: wait for NOTRDY to clear (module ready)
}


void CAN0_transmit_msg(const unsigned char data[], int len, uint8_t msg_type) { // Assumption: Message buffer CODE is INACTIVE
	CAN0->IFLAG1 |= 0x00000001; 						     					// Clear CAN 0 MB 0 flag without clearing others
	uint8_t i, j, k;
	uint8_t DLC_lengths[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
	uint8_t DLC;
	uint8_t len_diff;

	// Based on length of data array, determines which DLC setting to use
	for (i = 0; i < 16; i++) {
		CAN0->RAMn[0*MSG_BUF_SIZE + 2 + i] = 0x00000000;						// Reset every data word in the message buffer
		if (len <= DLC_lengths[i]) {
			DLC = i;
			len_diff = DLC_lengths[DLC] - len;
			break;
		}
	}

	for (i = 0; i < (len / 4); i++) {											// Each word is 4 bytes. Write each word except the last into the message buffer
		CAN0->RAMn[0*MSG_BUF_SIZE + 2 + i] = (data[i * 4 + 0] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | (data[i * 4 + 3] << 0);
	}

	// Write the last bytes into the final message buffer
	for (j = 0; j < len % 4; j++) {
		CAN0->RAMn[0*MSG_BUF_SIZE + 2 + i] |= (data[i * 4 + j] << (24-j*8));
	}

	// fill empty bytes with '0' to matc DLC size
	// j is the ending index of the data, so when j + k reaches 4 need to move to next word in message buffer
	for (k = 0; k < len_diff; k++) {
		CAN0->RAMn[0*MSG_BUF_SIZE + 2 + i + ((j + k) / 4)] |= '0' << (24 - ((j + k) % 4) * 8);
	}

	CAN0->RAMn[0*MSG_BUF_SIZE + 1] = ((msg_type << 8) | DEVICE_ID) << 18; 		// MB0 word 1: Tx msg with ID = msg_type << 8 | DEVICE ID

	if (FD) {
		CAN0->RAMn[0*MSG_BUF_SIZE + 0] = 0x8C400000 | (DLC << CAN_WMBn_CS_DLC_SHIFT) | (BRS << 30);
		// EDL = 1 - Distinguishes between CAN and CAN FD
		// BRS = 1 or 0 - Enable BRS
		// ESI, RES = 0
		// CODE = 0xC
		// RES = 0
		// SRR = 1 - substitute remote request
		// IDE = 0 - No ID extension
		// RTR = 0
		// DLC = variable, Max of 0xF = 64 data bytes
	}
	else {
		CAN0->RAMn[0*MSG_BUF_SIZE + 0] = 0x0C400000 | (DLC << CAN_WMBn_CS_DLC_SHIFT);
		// EDL, BRS, ESI, RES = 0: CANFD not used
		// CODE = 0xC: Activate msg buf to transmit
		// SRR = 1 Tx frame (not req'd for std ID)
		// IDE = 0: Standard ID
		// RTR = 0: data, not remote tx request frame
		// DLC = 8: 8-bytes
	}

	/* RAM0 of a message buffer used for Tx is structured as follows:
	 * EDL  -  31
	 * BRS  -  30
	 * ESI  -  29
	 * RES  -  28
	 * CODE -  24-27
	 * RES  -  23
	 * SRR  -  22
	 * IDE  -  21
	 * RTR  -  20
	 * DLC  -  16-19
	 * TIME -  0-15
	 */

}


void CAN0_receive_msg(int buff) { 												// Receive msg from ID 0x556 using msg buffer 4
	uint8_t j;
	uint32_t dummy;
	uint8_t DLC_lengths[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

	RxCODE = (CAN0->RAMn[ buff*MSG_BUF_SIZE + 0] & 0x07000000) >> 24; 			// Read CODE field
	RxID = (CAN0->RAMn[ buff*MSG_BUF_SIZE + 1] & CAN_WMBn_ID_ID_MASK) >> CAN_WMBn_ID_ID_SHIFT ;
	RxLENGTH = (CAN0->RAMn[ buff*MSG_BUF_SIZE + 0] & CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;

	RxLENGTH = DLC_lengths[RxLENGTH];

	for (j = 0; j < RxLENGTH; j++) { 											// Read two words of data (8 bytes)
		RxDATA[j] = CAN0->RAMn[ buff*MSG_BUF_SIZE + 2 + (j / 4)] >> (24 - j * 8);
	}

	RxTIMESTAMP = (CAN0->RAMn[ 0*MSG_BUF_SIZE + 0] & 0x000FFFF);
	dummy = CAN0->TIMER; 														// Read TIMER to unlock message buffers
	CAN0->IFLAG1 |= 0x1 << buff;	 											// Clear CAN 0 message buffer flag without clearing others
}


void port_init(void) {
	 PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; 							// Enable clock for PORTE
	 PORTE->PCR[4] |= PORT_PCR_MUX(5); 											// Port E4: MUX = ALT5, CAN0_RX
	 PORTE->PCR[5] |= PORT_PCR_MUX(5); 											// Port E5: MUX = ALT5, CAN0_TX
}

