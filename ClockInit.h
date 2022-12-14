/***********************************************************************
 * This code initializes an external oscillator and sets the system
 * 		clock speeds.
 *
 * Author:  Zac Lynn
 * Version: Phase2.1
 * See:     https://www.nxp.com/docs/en/application-note/AN5413.pdf
 **********************************************************************/
#include "S32k146.h"


/***********************************************************************
 * This function initializes the external 16MHz oscillator.
 *
 * @param void
 * @return void
 **********************************************************************/
void SOSC_init_16MHz(void);


/***********************************************************************
 * This function initializes the SPLL to 160MHz
 *
 * @param void
 * @return void
 **********************************************************************/
void SPLL_init_160MHz(void);


/***********************************************************************
 * This function initializes the core clk to 80MHz
 *
 * @param void
 * @return void
 **********************************************************************/
void NormalRUNmode_80MHz(void);
