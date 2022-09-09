#include "S32K144.h" 														// Include peripheral declarations S32K144
#include "ADC.h"

#include "stdio.h"



void init_adc(void) {
	SCG->FIRCDIV = SCG_FIRCDIV_FIRCDIV2(4); 								// FIRCDIV2 = 4: FIRCDIV2 divide by 8

	/****  Calibrate ADC0  ****/
	PCC->PCCn[PCC_ADC0_INDEX] &=~ PCC_PCCn_CGC_MASK; 						// Disable clock to change PCS
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(3); 							// PCS = 3: Select FIRCDIV2
	PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK; 						// Enable bus clock in ADC
	ADC0->SC3 = ADC_SC3_CAL_MASK 											// CAL = 1: Start calibration sequence
							  | ADC_SC3_AVGE_MASK 							// AVGE = 1: Enable hardware average
							  | ADC_SC3_AVGS(3); 							// AVGS = 11b: 32 samples averaged

	/****  Calibrate ADC1  ****/
	PCC->PCCn[PCC_ADC1_INDEX] &=~ PCC_PCCn_CGC_MASK;
	PCC->PCCn[PCC_ADC1_INDEX] |= PCC_PCCn_PCS(3);
	PCC->PCCn[PCC_ADC1_INDEX] |= PCC_PCCn_CGC_MASK;
	ADC1->SC3 = ADC_SC3_CAL_MASK
							  | ADC_SC3_AVGE_MASK
							  | ADC_SC3_AVGS(3);
	// Wait for ADC0 to finish
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	// Wait for ADC1 to finish
	while(((ADC1->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);

	// ADC0: Software trigger, single conversion, 10-bit resolution
	 ADC0->SC1[0] = ADC_SC1_ADCH_MASK; 										// ADCH: Module disabled for conversions
	 ADC0->CFG1 = ADC_CFG1_ADIV(0) | ADC_CFG1_MODE(2); 						// ADIV = 0: Divide ratio = 1, mode = 2 -> 10bit mode

	 ADC0->CFG2 = ADC_CFG2_SMPLTS(12); 										// SMPLTS = 12: sample time is 13 ADC clk
	 ADC0->SC2 = ADC_SC2_ADTRG(0); 											// ADTRG = 0: SW trigger
	 ADC0->SC3 = 0x00000000; 												// ADCO = 0: One conversion performed

	// ADC1: software trigger, single conversion, 10-bit resolution

	 ADC1->SC1[0] = ADC_SC1_ADCH_MASK;
	 ADC1->CFG1 = ADC_CFG1_ADIV(0) | ADC_CFG1_MODE(2);

	 ADC1->CFG2 = ADC_CFG2_SMPLTS(12);
	 ADC1->SC2 = ADC_SC2_ADTRG(0);
	 ADC1->SC3 = 0x00000000;

	 check_adc();
}

void check_adc(void) {
	uint16_t temp = 0;

	/****    ADC0    ****/
	// Read Vref HIGH
	ADC0->SC1[0] = ADC_SC1_ADCH(0x1D);
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC0->R[0];

	if ((temp & 0x3FF) == 0x3FF) {											// Check that no bits are stuck LOW
		printf("All ADC bits are HIGH on ADC0\n");
	}

	// Read Vrf LOW
	ADC0->SC1[0] = ADC_SC1_ADCH(0x1E);
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC0->R[0];

	if ((temp & 0x3FF) == 0x000) {											// Check that no bits are stuck HIGH
		printf("All ADC bits are LOW on ADC0\n");
	}

	// Read Bandgap voltage
	ADC0->SC1[0] = ADC_SC1_ADCH(0x1B);
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC0->R[0];

	if ((temp)) {															// Check that no bits are stuck HIGH
		printf("Bandgap Voltage on ADC0: %d\n", temp * 5000 / ((0x1 << 10) - 1));
	}

	/****    ADC1    ****/
	// Read Vref HIGH
	ADC1->SC1[0] = ADC_SC1_ADCH(0x1D);
	while(((ADC1->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC1->R[0];

	if ((temp & 0x3FF) == 0x3FF) {											// Check that no bits are stuck LOW
		printf("All ADC bits are HIGH on ADC1\n");
	}

	// Read Vrf LOW
	ADC1->SC1[0] = ADC_SC1_ADCH(0x1E);
	while(((ADC1->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC1->R[0];

	if ((temp & 0x3FF) == 0x000) {											// Check that no bits are stuck HIGH
		printf("All ADC bits are LOW on ADC1\n");
	}

	// Read Bandgap voltage
	ADC1->SC1[0] = ADC_SC1_ADCH(0x1B);
	while(((ADC1->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	temp = ADC1->R[0];

	if ((temp)) {															// Check that no bits are stuck HIGH
		printf("Bandgap Voltage on ADC1: %d\n", temp * 5000 / ((0x1 << 10) - 1));
	}
}

uint16_t read_adc0(uint8_t ch) {
	uint32_t ADC_RawResult;
	// Initiate new conversion by writing to ADC0_SC1A(ADCH)
	ADC0->SC1[0] = ADC_SC1_ADCH(ch);

	// Wait for latest conversion to complete
	while(((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	ADC_RawResult = ADC0->R[0];											// Read ADC Data Result A (ADC0_RA)
	return (ADC_RawResult * 5000) / ((1 << 10) - 1); 						// Convert to mV (@VREFH = 5V)
}

uint16_t read_adc1(uint8_t ch) {
	uint32_t ADC_RawResult;
	// Initiate new conversion by writing to ADC1_SC1A(ADCH)
	ADC1->SC1[0] = ADC_SC1_ADCH(ch);

	// Wait for latest conversion to complete
	while(((ADC1->SC1[0] & ADC_SC1_COCO_MASK) >> ADC_SC1_COCO_SHIFT) == 0);
	ADC_RawResult = ADC1->R[0];											// Read ADC Data Result A (ADC0_RA)
	return (ADC_RawResult * 5000) / ((1 << 10) - 1); 						// Convert to mV (@VREFH = 5V)
}
