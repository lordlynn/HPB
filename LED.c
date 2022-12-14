#include "S32K146.h"
#include "LED.h"

// Core clock 80Mhz / 32 = 2.5MHz clk with 32-bit count register
#define PERIOD  25000 - 1														// clk = 2.5MHz so, 25000 cycles = 100Hz
#define DC_INIT 12500															// Initially set to 50% DC


void PWM_init() {
	/****  Enable clock on ports  *****/
	PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; 							// PORTA
	PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK; 							// PORTB
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; 							// PORTC
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; 							// PORTD
	PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; 							// PORTE


	/****  Select and enable core clk for FTMn modules  *****/
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;			// FTM0
	PCC->PCCn[PCC_FTM1_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;			// FTM1
	PCC->PCCn[PCC_FTM2_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;			// FTM2
	PCC->PCCn[PCC_FTM3_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;			// FTM3
	PCC->PCCn[PCC_FTM4_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;			// FTM4


	/****  Select output pins from PORTA  ****/
//	PORTA->PCR[11] = PORT_PCR_MUX(2); 											// FTM1, Channel5	** DISABLES BKL C2 **
	PORTA->PCR[17] = PORT_PCR_MUX(2); 											// FTM0, Channel6

	/****  Select output pins from PORTB  ****/
	PORTB->PCR[17] = PORT_PCR_MUX(2); 											// FTM0, Channel5

	/****  Select output pins from PORTC  ****/
	PORTC->PCR[10] = PORT_PCR_MUX(2); 											// FTM3, Channel4

	PORTC->PCR[13] = PORT_PCR_MUX(2); 											// FTM3, Channel7

	/****  Select output pins from PORTD  ****/
	PORTD->PCR[0] = PORT_PCR_MUX(2); 											// FTM0, Channel2
	PORTD->PCR[1] = PORT_PCR_MUX(2); 											// FTM0, Channel3
	PORTD->PCR[5] = PORT_PCR_MUX(2); 											// FTM2, Channel3
	PORTD->PCR[10] = PORT_PCR_MUX(2); 											// FTM2, Channel0
	PORTD->PCR[11] = PORT_PCR_MUX(2); 											// FTM2, Channel1
	PORTD->PCR[12] = PORT_PCR_MUX(2); 											// FTM2, Channel2
	PORTD->PCR[13] = PORT_PCR_MUX(2); 											// FTM2, Channel4
	PORTD->PCR[14] = PORT_PCR_MUX(2); 											// FTM2, Channel5 ** DUPLICATE OUTPUT CH

	/****  Select output pins from PORTD  ****/
	PORTE->PCR[9] = PORT_PCR_MUX(2); 											// FTM0, Channel7
//	PORTE->PCR[11] = PORT_PCR_MUX(4); 											// FTM2, Channel5, Mode 4 not typ. ** DUPLICATE OUTPUT CH, SEE PTD14 // COMMENTING OUT THIS LINE DISABLES C1
	PORTE->PCR[13] = PORT_PCR_MUX(2); 											// FTM4, CH5

	/****  Enable registers updating from write buffers  ****/
	FTM0->MODE = FTM_MODE_FTMEN_MASK;
	FTM1->MODE = FTM_MODE_FTMEN_MASK;
	FTM2->MODE = FTM_MODE_FTMEN_MASK;
	FTM3->MODE = FTM_MODE_FTMEN_MASK;
	FTM4->MODE = FTM_MODE_FTMEN_MASK;

	/****  Set period (1kHz PWM frequency @48MHz system clock)  ****/
	FTM0->MOD = FTM_MOD_MOD(PERIOD);
	FTM1->MOD = FTM_MOD_MOD(PERIOD);
	FTM2->MOD = FTM_MOD_MOD(PERIOD);
	FTM3->MOD = FTM_MOD_MOD(PERIOD);
	FTM4->MOD = FTM_MOD_MOD(PERIOD);

	/****  Set CNTIN in initialization stage  ****/
	FTM0->CNTIN = FTM_CNTIN_INIT(0);
	FTM1->CNTIN = FTM_CNTIN_INIT(0);
	FTM2->CNTIN = FTM_CNTIN_INIT(0);
	FTM3->CNTIN = FTM_CNTIN_INIT(0);
	FTM4->CNTIN = FTM_CNTIN_INIT(0);

	/****  Enable high-true pulses of PWM signals  ****/
	FTM0->CONTROLS[2].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0->CONTROLS[3].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0->CONTROLS[6].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0->CONTROLS[7].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;

	FTM1->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;

	FTM2->CONTROLS[0].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM2->CONTROLS[1].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM2->CONTROLS[2].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM2->CONTROLS[3].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM2->CONTROLS[4].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM2->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;

	FTM3->CONTROLS[4].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM3->CONTROLS[7].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;

	FTM4->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;

	/****  Set initial channel duty cycle  ****/
	/* Set LEDs to max intensity */
	FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(DC_INIT);								// Ind P1
	FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(DC_INIT);								// Ind P2
	FTM0->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);								// Ind R1
	FTM0->CONTROLS[6].CnV = FTM_CnV_VAL(DC_INIT);								// Ind R2
	FTM0->CONTROLS[7].CnV = FTM_CnV_VAL(DC_INIT);								// Ind N1

//	FTM1->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl C2	** Disables BKL C2 **

	FTM2->CONTROLS[0].CnV = FTM_CnV_VAL(DC_INIT);								// Ind D4
	FTM2->CONTROLS[1].CnV = FTM_CnV_VAL(DC_INIT);								// Ind D3
	FTM2->CONTROLS[2].CnV = FTM_CnV_VAL(DC_INIT);								// Ind D2
	FTM2->CONTROLS[3].CnV = FTM_CnV_VAL(DC_INIT);								// Ind D1
	FTM2->CONTROLS[4].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl Park
	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl C1 && Ind N2 // DUPLICATE MODULE

	FTM3->CONTROLS[4].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl Drive
	FTM3->CONTROLS[7].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl Neutral

	FTM4->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);								// Bkl Reverse

	/****  Reset FTMn counters  ****/
	FTM0->CNT = 0;
	FTM1->CNT = 0;
	FTM2->CNT = 0;
	FTM3->CNT = 0;
	FTM4->CNT = 0;

	/****  Core clock selected (80MHz) and divided by 32   ****/
	/* On startup, don't enable any LEDS */
	FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(5);

	FTM1->SC = FTM_SC_CLKS(1) | FTM_SC_PS(5);

	FTM2->SC = FTM_SC_CLKS(1) | FTM_SC_PS(5);

	FTM3->SC = FTM_SC_CLKS(1) | FTM_SC_PS(5);

	FTM4->SC = FTM_SC_CLKS(1) | FTM_SC_PS(5);
}


uint8_t LED_controls(unsigned char data[], uint32_t id) {
	uint8_t msg_type = (id >> 26) & 0x7;

	if (msg_type == 1) {														// If the LED command is for On/Off
		if (data[0] == 0) {														// FTM0
			if (data[1] < 2 || data[1] > 7)										// If the LED command is for an invalid ch return false
				return 0;

			if (data[2] == 0)
				FTM0->SC &=~ (FTM_SC_PWMEN0_MASK << data[1]);
			else if (data[2] == 1)
				FTM0->SC |= FTM_SC_PWMEN0_MASK << data[1];
		}
		else if (data[0] == 1) {												// FTM1
			if (data[1] != 5)
				return 0;

			if (data[2] == 0)
				FTM1->SC &=~ (FTM_SC_PWMEN0_MASK << data[1]);
			else if (data[2] == 1)
				FTM1->SC |= FTM_SC_PWMEN0_MASK << data[1];
		}
		else if (data[0] == 2) {												// FTM2
			if (data[1] < 0 || data[1] > 5)
				return 0;

			if (data[2] == 0)
				FTM2->SC &=~ (FTM_SC_PWMEN0_MASK << data[1]);
			else if (data[2] == 1)
				FTM2->SC |= FTM_SC_PWMEN0_MASK << data[1];
		}
		else if (data[0] == 3) {												// FTM3
			if (data[1] != 7)
				return 0;

			if (data[2] == 0)
				FTM3->SC &=~ (FTM_SC_PWMEN0_MASK << data[1]);
			else if (data[2] == 1)
				FTM3->SC |= FTM_SC_PWMEN0_MASK << data[1];
		}
		else if (data[0] == 4) {												// FTM4
			if (data[1] != 5)
				return 0;

			if (data[2] == 0)
				FTM4->SC &=~ (FTM_SC_PWMEN0_MASK << data[1]);
			else if (data[2] == 1)
				FTM4->SC |= FTM_SC_PWMEN0_MASK << data[1];
		}
	}
	else if (msg_type == 2) {													// If LED command is for setting DC
		if (data[0] == 0) {														// FTM0
			if (data[1] < 2 || data[1] > 7)										// If the LED command is for an invalid ch return false
				return 0;

			if (data[2] > 100)
				data[2] = 100;

			FTM0->SC &=~ FTM_SC_CLKS(1);
			FTM0->CONTROLS[data[1]].CnV = FTM_CnV_VAL(((float)data[2] / 100.0) * PERIOD);
			FTM0->SC |= FTM_SC_CLKS(1);
		}
		else if (data[0] == 1) {												// FTM1
			if (data[1] != 5)
				return 0;

			if (data[2] > 100)
				data[2] = 100;

			FTM1->SC &=~ FTM_SC_CLKS(1);
			FTM1->CONTROLS[data[1]].CnV = FTM_CnV_VAL(((float)data[2] / 100.0) * PERIOD);
			FTM1->SC |= FTM_SC_CLKS(1);
		}
		else if (data[0] == 2) {												// FTM2
			if (data[1] < 0 || data[1] > 5)
				return 0;

			if (data[2] > 100)
				data[2] = 100;

			FTM2->SC &=~ FTM_SC_CLKS(1);
			FTM2->CONTROLS[data[1]].CnV = FTM_CnV_VAL(((float)data[2] / 100.0) * PERIOD);
			FTM2->SC |= FTM_SC_CLKS(1);
		}
		else if (data[0] == 3) {												// FTM3
			if (data[1] != 7)
				return 0;

			if (data[2] > 100)
				data[2] = 100;

			FTM3->SC &=~ FTM_SC_CLKS(1);
			FTM3->CONTROLS[data[1]].CnV = FTM_CnV_VAL(((float)data[2] / 100.0) * PERIOD);
			FTM3->SC |= FTM_SC_CLKS(1);
		}
		else if (data[0] == 4) {												// FTM4
			if (data[1] != 5)
				return 0;

			if (data[2] > 100)
				data[2] = 100;

			FTM4->SC &=~ FTM_SC_CLKS(1);
			FTM4->CONTROLS[data[1]].CnV = FTM_CnV_VAL(((float)data[2] / 100.0) * PERIOD);
			FTM4->SC |= FTM_SC_CLKS(1);
		}
	}
	return 1;
}


void enable_LED_park(void) {
	FTM0->SC |= FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK;
}
void disable_LED_park(void) {
	FTM0->SC &=~ (FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK);
}


void enable_LED_reverse(void) {
	FTM0->SC |= FTM_SC_PWMEN5_MASK | FTM_SC_PWMEN6_MASK;
}
void disable_LED_reverse(void) {
	FTM0->SC &=~ (FTM_SC_PWMEN5_MASK | FTM_SC_PWMEN6_MASK);
}


void enable_LED_neutral(void) {
	FTM0->SC |= FTM_SC_PWMEN7_MASK;
	FTM2->SC |= FTM_SC_PWMEN5_MASK;
}
void disable_LED_neutral(void) {
	FTM0->SC &=~ FTM_SC_PWMEN7_MASK;
	FTM2->SC &=~ FTM_SC_PWMEN5_MASK;
}


void enable_LED_drive(void) {
	FTM2->SC |= FTM_SC_PWMEN0_MASK | FTM_SC_PWMEN1_MASK
			  | FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK;
}
void disable_LED_drive(void) {
	FTM2->SC &=~ (FTM_SC_PWMEN0_MASK | FTM_SC_PWMEN1_MASK
			  	| FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK);
}


void enable_bkl(void) {
//	FTM1->SC |= FTM_SC_PWMEN5_MASK;												// DISABLES C2
	FTM2->SC |= FTM_SC_PWMEN4_MASK; // | FTM_SC_PWMEN5_MASK;					// DISABLES C1
	FTM3->SC |= FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN7_MASK;
	FTM4->SC |= FTM_SC_PWMEN5_MASK;
}


void disable_bkl(void) {
//	FTM1->SC &=~ FTM_SC_PWMEN5_MASK;											// DISABLES C2
	FTM2->SC &=~ (FTM_SC_PWMEN4_MASK);// | FTM_SC_PWMEN5_MASK);					// DISABLES C1
	FTM3->SC &=~ (FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN7_MASK);
	FTM4->SC &=~ FTM_SC_PWMEN5_MASK;
}


/* These functions should be called when calibratin the LEDs */
// Park indicators
void set_p1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_p2(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM0->SC |= FTM_SC_CLKS(1);
}


// Reverse indicators
void set_r1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[5].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_r2(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[6].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM0->SC |= FTM_SC_CLKS(1);
}


// Neutral indicators
void set_n1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[7].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_n2(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}


// Drive indicators
void set_d1(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[3].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d2(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[2].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d3(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[1].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d4(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[0].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}


// Backlights
void set_bkl_p(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[4].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM2->SC |= FTM_SC_CLKS(1);
}


void set_bkl_r(uint8_t brightness) {
	FTM4->SC &=~ FTM_SC_CLKS(1);

	FTM4->CONTROLS[5].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM4->SC |= FTM_SC_CLKS(1);
}


void set_bkl_n(uint8_t brightness) {
	FTM3->SC &=~ FTM_SC_CLKS(1);

	FTM3->CONTROLS[7].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM3->SC |= FTM_SC_CLKS(1);
}


void set_bkl_d(uint8_t brightness) {
	FTM3->SC &=~ FTM_SC_CLKS(1);

	FTM3->CONTROLS[4].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);

	FTM3->SC |= FTM_SC_CLKS(1);
}


void set_bkl_c1(uint8_t brightness) {
	// C1 IS DISABLED UNTIL NEW FTM CH CAN BE SELECTED FOR IT
//	FTM2->SC &=~ FTM_SC_CLKS(1);
//
//	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);
//
//	FTM2->SC |= FTM_SC_CLKS(1);
}


void set_bkl_c2(uint8_t brightness) {
	// C2 IS DISABLED UNTIL C1 is working or chrome ring is updated to be translucent
//	FTM1->SC &=~ FTM_SC_CLKS(1);
//
//	FTM1->CONTROLS[5].CnV = FTM_CnV_VAL(((float)brightness / 100.0) * PERIOD);
//
//	FTM1->SC |= FTM_SC_CLKS(1);
}

