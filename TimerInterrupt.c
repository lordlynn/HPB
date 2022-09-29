#include "S32K146.h"
#include "TimerInterrupt.h"
//#include "fsl_core_cm4.h"

/** Pointer to Systick **/
#define SysTick ((SysTick_Type *) (0xE000E010UL))

/** Defines the systick registers **/
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
uint32_t count = 0;

void start_systick(double delay) {
//	DISABLE_INTERRUPTS();

	SysTick->LOAD = delay * 48000; 											// set SysTick timer load value. 48Mhz = 48000 cycles per ms

	SysTick->CTRL = 0x7; 										    		// start SysTick timer with Interrupt using core clock

//	ENABLE_INTERRUPTS();

}

/***********************************************************************
 * Called on every systick interrupt. Increments the value of count on
 * 		every interrupt as a means of keeping time since startup.
 *
 * @param void
 * @return void
 **********************************************************************/
void SysTick_Handler(void ) {
	count++;
}
