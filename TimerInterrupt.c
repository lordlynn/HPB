#include "S32K146.h"
#include "TimerInterrupt.h"

/** Pointer to Systick **/
#define SysTick ((SysTick_Type *) (0xE000E010UL))


/** renames the s32_systick register **/
typedef struct {
	/** Systick control register **/
	__IO uint32_t CTRL;

	/** Systick max count value **/
	__IO uint32_t LOAD;

	/** Current Systick counter value **/
	__IO uint32_t VAL;

	/** Systick calibration **/
	__IO uint32_t CALIB;

} SysTick_Type;


/** How many Systick interrupts have happened since startup **/
uint32_t millis = 0;


void start_systick(double delay) {
	SysTick->LOAD = delay * 80000; 												// Set SysTick timer load value. 80Mhz = 80000 cycles per ms
	SysTick->VAL = 0;
	SysTick->CTRL = 0x7; 										    			// Start SysTick timer with Interrupt using core clock
}


/***********************************************************************
 * Called on every systick interrupt. Increments the value of count on
 * 		every interrupt as a means of keeping time since startup.
 *
 * @param void
 * @return void
 **********************************************************************/
void SysTick_Handler(void ) {
	millis++;
}

