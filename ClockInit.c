#include "ClockInit.h"


void SOSC_init_16MHz(void) {
	SCG->SOSCDIV=0x00000101; 													// SOSCDIV1 & SOSCDIV2 =1: divide by 1

	/* HGO=0: Config xtal osc for low power */
	/* EREFS=1: Input is external XTAL */
	SCG->SOSCCFG=0x00000034; 													// Range=3: High freq (SOSC between 8MHz-40MHz)

	while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK); 									// Ensure SOSCCSR unlocked

	/* SOSCCMRE=0: OSC CLK monitor IRQ if enabled */
	/* SOSCCM=0: OSC CLK monitor disabled */
	/* SOSCERCLKEN=0: Sys OSC 3V ERCLK output clk disabled */
	/* SOSCLPEN=0: Sys OSC disabled in VLP modes */
	/* SOSCSTEN=0: Sys OSC disabled in Stop modes */
	/* SOSCEN=1: Enable oscillator */
	SCG->SOSCCSR=0x00000001; 													// LK=0: SOSCCSR can be written

	while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)); 							// Wait for sys OSC clk valid
}


void SPLL_init_160MHz(void) {
	while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK); 									// Ensure SPLLCSR unlocked
	SCG->SPLLCSR = 0x00000000; 													// SPLLEN=0: SPLL is disabled (default)
	SCG->SPLLDIV = 0x00000302; 													// SPLLDIV1 divide by 2; SPLLDIV2 divide by 4

	// (16MHz / 1) * (20 / 2) = 160MHz
	SCG->SPLLCFG = 0x00040000; 													// PREDIV=0: Divide SOSC_CLK by 0+1=1, multiply by 20

	while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK); 									// Ensure SPLLCSR unlocked

	/* SPLLCMRE=0: SPLL CLK monitor IRQ if enabled */
	/* SPLLCM=0: SPLL CLK monitor disabled */
	/* SPLLSTEN=0: SPLL disabled in Stop modes */
	/* SPLLEN=1: Enable SPLL */
	SCG->SPLLCSR = 0x00000001; 													// LK=0: SPLLCSR can be written

	while(!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK)); 							// Wait for SPLL valid
}


void NormalRUNmode_80MHz(void) { 												// Change to normal RUN mode with 16MHz SOSC, 80 MHz PLL
	SCG->RCCR = 0x06010012; 													// PLL as clock source, DIVCORE=1, DIVBUS=1, DIVSLOW=2

	/* Wait for sys clk src = SPLL */
	while (((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT ) != 6);
}
