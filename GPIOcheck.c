#include "S32K144.h"
#include <stdio.h>

int main(void) {
	WDOG->CNT = 0xD928C520; 												// Unlock watchdog
	WDOG->TOVAL = 0x0000FFFF; 												// Maximum timeout value
	WDOG->CS = 0x00002100; 													// Disable watchdog


	int i = 0;
	PCC->PCCn[PCC_PORTA_INDEX] |= 0x40000000;								// Write Clock Gate Control high to allow updating registers
	PCC->PCCn[PCC_PORTB_INDEX] |= 0x40000000;
	PCC->PCCn[PCC_PORTC_INDEX] |= 0x40000000;
	PCC->PCCn[PCC_PORTD_INDEX] |= 0x40000000;
	PCC->PCCn[PCC_PORTE_INDEX] |= 0x40000000;

	for (i = 0; i <= 17; i++) {
		PORTA->PCR[i] = 0x00000100;												// Sets pin as GPIO
	}
	for (i = 0; i <= 17; i++) {
		PORTB->PCR[i] = 0x00000100;												// Sets pin as GPIO
	}
	for (i = 0; i <= 17; i++) {
		PORTC->PCR[i] = 0x00000100;												// Sets pin as GPIO
	}
	for (i = 0; i <= 17; i++) {
		PORTD->PCR[i] = 0x00000100;												// Sets pin as GPIO
	}

	for (i = 0; i <= 17; i++) {
		PORTE->PCR[i] = 0x00000100;												// Sets pin as GPIO
	}


	// Set Port Data Direction Register for inputs
	PTA->PDDR |= 0x3FFFF;
	PTB->PDDR |= 0x3FFFF;
	PTC->PDDR |= 0x3FFFF;
	PTD->PDDR |= 0x3FFFF;
	PTE->PDDR |= 0x3FFFF;

	// Set outputs as high
	PTA->PDOR |= 0x3FFFF;
	PTB->PDOR |= 0x3FFFF;
	PTC->PDOR |= 0x3FFFF;
	PTD->PDOR |= 0x3FFFF;
	PTE->PDOR |= 0x3FFFF;

	// CHeck that all pins can drive high
	if ((PTA->PDIR & 0x3FFFF) != 0x3FFFF) {
		printf("PORTA Failure to drive high: %x\n", PTA->PDIR);
	}
	if ((PTB->PDIR & 0x3FFFF) != 0x3FFFF) {
		printf("PORTB Failure to drive high: %x\n", PTB->PDIR);
	}
	if ((PTC->PDIR & 0x3FFFF) != 0x3FFFF) {
		printf("PORTC Failure to drive high: %x\n", PTC->PDIR);
	}
	if ((PTD->PDIR & 0x3FFFF) != 0x3FFFF) {
		printf("PORTD Failure to drive high: %x\n", PTD->PDIR);
	}
	if ((PTE->PDIR & 0x3FFFF) != 0x3FFFF) {
		printf("PORTE Failure to drive high: %x\n", PTE->PDIR);
	}



	// Set outputs as low
	PTA->PDOR &=~ 0x3FFFF;
	PTB->PDOR &=~ 0x3FFFF;
	PTC->PDOR &=~ 0x3FFFF;
	PTD->PDOR &=~ 0x3FFFF;
	PTE->PDOR &=~ 0x3FFFF;

	// Check that all pins can drive low
	if ((PTA->PDIR & 0x3FFFF) != 0x0) {
		printf("PORTA Failure to drive low: %x\n", PTA->PDIR);
	}
	if ((PTB->PDIR & 0x3FFFF) != 0x0) {
		printf("PORTB Failure to drive low: %x\n", PTB->PDIR);
	}
	if ((PTC->PDIR & 0x3FFFF) != 0x0) {
		printf("PORTC Failure to drive low: %x\n", PTC->PDIR);
	}
	if ((PTD->PDIR & 0x3FFFF) != 0x0) {
		printf("PORTD Failure to drive low: %x\n", PTD->PDIR);
	}
	if ((PTE->PDIR & 0x3FFFF) != 0x0) {
		printf("PORTE Failure to drive low: %x\n", PTE->PDIR);
	}

	printf("Finished\n");

	return 0;

}
