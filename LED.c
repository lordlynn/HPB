// https://www.nxp.com/docs/en/application-note/AN5303.pdf?_gl=1*7nlult*_ga*NDE0NzY1ODM5LjE2NjE5NTIzNjg.*_ga_WM5LE0KMSH*MTY2MjAzOTQ5OC41LjAuMTY2MjAzOTQ5OC4wLjAuMA..
// Features and Operation Modes of FlexTimer Module on S32K

#include "S32K146.h"
//#include "fsl_core_cm4.h"
#include "LED.h"

#define PERIOD (30000 - 1)													// clk = 48MHz/16 = 3Mhz so, 3333 cycles = 100Hz
#define DC_INIT 15000

void PWM_init() {
	/****  Enable clock on ports  *****/
	PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; 						// PORTA
	PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK; 						// PORTB
	PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; 						// PORTC
	PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK; 						// PORTD
	PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; 						// PORTE


	/****  Select and enable core clk for FTMn modules  *****/
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;		// FTM0
	PCC->PCCn[PCC_FTM1_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;		// FTM1
	PCC->PCCn[PCC_FTM2_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;		// FTM2
	PCC->PCCn[PCC_FTM3_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;		// FTM3
	PCC->PCCn[PCC_FTM4_INDEX] |= PCC_PCCn_PCS(6) | PCC_PCCn_CGC_MASK;		// FTM4  REQUIRES S32k146 which has FTM module


	/****  Select output pins from PORTA  ****/
	PORTA->PCR[11] = PORT_PCR_MUX(2); 										// FTM1, Channel5
	PORTA->PCR[17] = PORT_PCR_MUX(2); 										// FTM0, Channel6

	/****  Select output pins from PORTB  ****/
	PORTB->PCR[17] = PORT_PCR_MUX(2); 										// FTM0, Channel5

	/****  Select output pins from PORTC  ****/
	PORTC->PCR[10] = PORT_PCR_MUX(2); 										// FTM3, Channel4      enable pull down reistor

	PORTC->PCR[13] = PORT_PCR_MUX(2); 										// FTM3, Channel7

	/****  Select output pins from PORTD  ****/
	PORTD->PCR[0] = PORT_PCR_MUX(2); 										// FTM0, Channel2
	PORTD->PCR[1] = PORT_PCR_MUX(2); 										// FTM0, Channel3
	PORTD->PCR[5] = PORT_PCR_MUX(2); 										// FTM2, Channel3
	PORTD->PCR[10] = PORT_PCR_MUX(2); 										// FTM2, Channel0
	PORTD->PCR[11] = PORT_PCR_MUX(2); 										// FTM2, Channel1
	PORTD->PCR[12] = PORT_PCR_MUX(2); 										// FTM2, Channel2
	PORTD->PCR[13] = PORT_PCR_MUX(2); 										// FTM2, Channel4
	PORTD->PCR[14] = PORT_PCR_MUX(2); 										// FTM2, Channel5 ** DUPLICATE OUTPUT CH

	/****  Select output pins from PORTD  ****/
	PORTE->PCR[9] = PORT_PCR_MUX(2); 										// FTM0, Channel7
//	PORTE->PCR[11] = PORT_PCR_MUX(4); 										// FTM2, Channel5, Mode 4 not typ. ** DUPLICATE OUTPUT CH, SEE PTD14 // DISABLES C1
	PORTE->PCR[13] = PORT_PCR_MUX(2); 										// FTM4, CH5  ** FTM4 DOES NOT EXIST ON THE DEV BOARD

	/****  Enable registers updating from write buffers  ****/
	FTM0->MODE = FTM_MODE_FTMEN_MASK;
	FTM1->MODE = FTM_MODE_FTMEN_MASK;
	FTM2->MODE = FTM_MODE_FTMEN_MASK;
	FTM3->MODE = FTM_MODE_FTMEN_MASK;
	FTM4->MODE = FTM_MODE_FTMEN_MASK;										// NO FTM4 MODULE


	/****  Set period (1kHz PWM frequency @48MHz system clock)  ****/
	FTM0->MOD = FTM_MOD_MOD(PERIOD);
	FTM1->MOD = FTM_MOD_MOD(PERIOD);
	FTM2->MOD = FTM_MOD_MOD(PERIOD);
	FTM3->MOD = FTM_MOD_MOD(PERIOD);
	FTM4->MOD = FTM_MOD_MOD(PERIOD);										// NO FTM4 MODULE

	/****  Set CNTIN in initialization stage  ****/
	FTM0->CNTIN = FTM_CNTIN_INIT(0);
	FTM1->CNTIN = FTM_CNTIN_INIT(0);
	FTM2->CNTIN = FTM_CNTIN_INIT(0);
	FTM3->CNTIN = FTM_CNTIN_INIT(0);
	FTM4->CNTIN = FTM_CNTIN_INIT(0);										// NO FTM4 MODULE

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

	FTM4->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;		// NO FTM4 MODULE


	/****  Set initial channel duty cycle  ****/
	/* Set LEDs to max intensity */
	FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(DC_INIT);							// Ind P1
	FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(DC_INIT);							// Ind P2
	FTM0->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);							// Ind R1
	FTM0->CONTROLS[6].CnV = FTM_CnV_VAL(DC_INIT);							// Ind R2
	FTM0->CONTROLS[7].CnV = FTM_CnV_VAL(DC_INIT);							// Ind N1

	FTM1->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl C2

	FTM2->CONTROLS[0].CnV = FTM_CnV_VAL(DC_INIT);							// Ind D4
	FTM2->CONTROLS[1].CnV = FTM_CnV_VAL(DC_INIT);							// Ind D3
	FTM2->CONTROLS[2].CnV = FTM_CnV_VAL(DC_INIT);							// Ind D2
	FTM2->CONTROLS[3].CnV = FTM_CnV_VAL(DC_INIT);							// Ind D1
	FTM2->CONTROLS[4].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl Park
	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl C1 && Ind N2 // DUPLICATE MODULE

	FTM3->CONTROLS[4].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl Drive
	FTM3->CONTROLS[7].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl Neutral

	FTM4->CONTROLS[5].CnV = FTM_CnV_VAL(DC_INIT);							// Bkl Reverse // NO FTM4 MODULE


	/****  Reset FTMn counters  ****/
	FTM0->CNT = 0;
	FTM1->CNT = 0;
	FTM2->CNT = 0;
	FTM3->CNT = 0;
	FTM4->CNT = 0;															// NO FTM4 MODULE

	/****  Clock selection (core clk) divdied by 16 and enable PWM generation  ****/
	/* On startup, only enable the backlights and park indicator */
	FTM0->SC = FTM_SC_CLKS(1) | FTM_SC_PS(4); // | FTM_SC_PWMEN2_MASK | FTM_SC_PWMEN3_MASK; // Enables park indicators

	FTM1->SC = FTM_SC_CLKS(1) | FTM_SC_PS(4); // | FTM_SC_PWMEN5_MASK;

	FTM2->SC = FTM_SC_CLKS(1) | FTM_SC_PS(4); // | FTM_SC_PWMEN4_MASK; // | FTM_SC_PWMEN5_MASK; // DISABLES C1

	FTM3->SC = FTM_SC_CLKS(1) | FTM_SC_PS(4); // | FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN7_MASK;

	FTM4->SC = FTM_SC_CLKS(1) | FTM_SC_PS(4); // | FTM_SC_PWMEN5_MASK;							// NO FTM4 MODULE
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
	FTM1->SC |= FTM_SC_PWMEN5_MASK;
	FTM2->SC |= FTM_SC_PWMEN4_MASK; // | FTM_SC_PWMEN5_MASK;		// DISABLES C1
	FTM3->SC |= FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN7_MASK;
	FTM4->SC |= FTM_SC_PWMEN5_MASK; 											// NO FTM4 MODULE ON DEV BOARD
}

void disable_bkl(void) {
	FTM1->SC &=~ FTM_SC_PWMEN5_MASK;
	FTM2->SC &=~ (FTM_SC_PWMEN4_MASK);// | FTM_SC_PWMEN5_MASK);		// DISABLES C1
	FTM3->SC &=~ (FTM_SC_PWMEN4_MASK | FTM_SC_PWMEN7_MASK);
	FTM4->SC &=~ FTM_SC_PWMEN5_MASK; 											// NO FTM4 MODULE ON DEV BOARD
}


/* These functions should be called when calibratin the LEDs */
// Park indicators
void set_p1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[2].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_p2(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[3].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM0->SC |= FTM_SC_CLKS(1);
}

// Reverse indicators
void set_r1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[5].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_r2(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[6].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM0->SC |= FTM_SC_CLKS(1);
}

// Neutral indicators
void set_n1(uint8_t brightness) {
	FTM0->SC &=~ FTM_SC_CLKS(1);

	FTM0->CONTROLS[7].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM0->SC |= FTM_SC_CLKS(1);
}
void set_n2(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}

// Drive indicators
void set_d1(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[3].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d2(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[2].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d3(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[1].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_d4(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[0].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}

// Backlights
void set_bkl_p(uint8_t brightness) {
	FTM2->SC &=~ FTM_SC_CLKS(1);

	FTM2->CONTROLS[4].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_bkl_r(uint8_t brightness) {
	// **** NO MODULE FTM4 ON THIS DEV BOARD ****
	FTM4->SC &=~ FTM_SC_CLKS(1);

	FTM4->CONTROLS[5].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM4->SC |= FTM_SC_CLKS(1);
}
void set_bkl_n(uint8_t brightness) {
	FTM3->SC &=~ FTM_SC_CLKS(1);

	FTM3->CONTROLS[7].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM3->SC |= FTM_SC_CLKS(1);
}
void set_bkl_d(uint8_t brightness) {
	FTM3->SC &=~ FTM_SC_CLKS(1);

	FTM3->CONTROLS[4].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM3->SC |= FTM_SC_CLKS(1);
}

void set_bkl_c1(uint8_t brightness) {
	// C1 IS DISABLED UNTIL NEW FTM CH CAN BE SELECTED FOR IT
//	FTM2->SC &=~ FTM_SC_CLKS(1);
//
//	FTM2->CONTROLS[5].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));
//
//	FTM2->SC |= FTM_SC_CLKS(1);
}
void set_bkl_c2(uint8_t brightness) {
	FTM1->SC &=~ FTM_SC_CLKS(1);

	FTM1->CONTROLS[5].CnV = FTM_CnV_VAL(PERIOD * ((float)brightness / 100.0));

	FTM1->SC |= FTM_SC_CLKS(1);
}

