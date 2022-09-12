// ADC Code adapted from https://www.nxp.com/docs/en/application-note/AN12217.pdf
#include "S32K144.h" 														// Include peripheral declarations S32K144
#include <stdio.h>															// DEBUG ONLY. INCLUDED FOR PRINTF
#include "TimerInterrupt.h"
#include "ADC.h"
#include "LED.h"

/** Number of consecutive samples required to debounce **/
#define DL 5
/** High end of initial voltage range **/
#define IH 4200
/** Low end of initial voltage range **/
#define IL 3500
/** High end of final voltage range **/
#define FH 2700
/** Low end of final voltage range **/
#define FL 2000

/** Keeps track of the state of individual contacts as well as the overall debounced state for a button **/
enum state {
	/** When the analog contact voltage is less than the pressed range **/
	low,

	/** When the analog contact voltage is within the pressed range **/
	press,

	/** When the analog contact voltage is between the pressed and open ranges **/
	transition,

	/** When the analog contact voltage is within the open range **/
	open,

	/** When the analog contact voltage is greater than the open range **/
	high,

	/** Initial state. If this state persists something is wrong **/
	none
};

/** Holds variables used to track analog contact debouncing **/
struct analog_contact {
	/** Tracks invalid high voltages **/
	uint8_t invalid_high;

	/** Tracks how many samples have been read for 16/20 sample req **/
	uint8_t high_count;

	/** Tracks invalid low voltages **/
	uint8_t invalid_low;

	/** Tracks how many samples have been read for 16/20 sample req **/
	uint8_t low_count;

	/** Tracks final voltages (closed) **/
	uint8_t press;

	/** Tracks initial voltages (open) **/
	uint8_t unpress;

	/** Tracks transitional voltages **/
	uint16_t transition;

	/** Tracks how many samples have been read for 2000/2500 sample requirement **/
	uint16_t transition_count;

	/** Debounced state of contact **/
	enum state state;

	/** Value of count variable at last state change. Used for contact timing **/
	uint32_t time;

	/** Which ADC module to use (0 or 1) **/
	uint8_t adc;

	/** ADC CH to read **/
	uint8_t ch;
};

/** Holds variables used to track digital contact debouncing **/
struct digital_contact {
	/** Tracks if the digital contact was closed **/
	uint8_t press;

	/** Tracks if the digital contact was open **/
	uint8_t unpress;

	/** Debounced state of contact **/
	enum state state;

	/** Value of count variable at last state change **/
	uint32_t time;
};

/** Stores two analog contacts and one digital contact to represent a
 * 		button (P, R, N, D), and the final debounced state  **/
struct btn {
	/** Analog contact 1 **/
	struct analog_contact A1;

	/** Analog contact 2 **/
	struct analog_contact A2;

	/** Digital contact 1 **/
	struct digital_contact D1;

	/** Final debounced button state **/
	enum state state;

	/** Stores the last state **/
	enum state last;
};

void WDOG_disable(void);													// Disables WDT. USED FOR EARLY DEV ONLY. ADD WDT LATER
void init_digital_pins(void);												// Initialize PTC0,14,16 and PTB0 as GPIO inputs with pull up resistors
void init_btn(struct btn *b, uint8_t ch1, uint8_t adc1,						// Initializes the nested structs
		      uint8_t ch2, uint8_t adc2);

void debounce_btn(struct btn *b, uint32_t digital_condition);				// Debounces a button with 2 analog and 1 digital contacts
void debounce_analog_contact(struct analog_contact *a);						// Helper function for debounce_btn

int main(void) {
	WDOG_disable(); 														// Disable Watchdog

	// Creates structures used for debouncing for each button
	struct btn park_btn;
	struct btn reverse_btn;
	struct btn neutral_btn;
	struct btn drive_btn;

	// Initializes the structure's variables to their starting values
	init_btn(&park_btn,     9, 0, 8, 1);									// ADC0-CH9 and ADC1-CH8
	init_btn(&reverse_btn, 15, 0, 2, 1);									// ADC0-CH15 and ADC1-CH2
	init_btn(&neutral_btn, 13, 0, 3, 1);									// ADC0-CH13 and ADC1-CH3
	init_btn(&drive_btn,    5, 0, 6, 1);									// ADC0-CH5 and ADC1-CH1

	start_systick(1.0);														// Start systick with interrupt every 1ms. variable "count" increments on every interrupt
	PWM_init();																// Initializes LED pins
	init_adc();																// Initializes analog switch pins
	init_digital_pins();													// Initializes digital swithc pins

	int bkl_off = 0;

	while (1) {
		if (count % 5 == 0) {

			debounce_btn(&park_btn,    (PTC->PDIR & (0x1 << 0)));
			debounce_btn(&reverse_btn, (PTC->PDIR & (0x1 << 16)));
			debounce_btn(&neutral_btn, (PTC->PDIR & (0x1 << 14)));
			debounce_btn(&drive_btn,   (PTB->PDIR & (0x1 << 0)));


			// Check that two buttons are not pushed at the same time
			if ((park_btn.state == press && reverse_btn.state == press)    ||
				(park_btn.state == press && neutral_btn.state == press)    ||
				(park_btn.state == press && drive_btn.state == press)      ||
				(reverse_btn.state == press && neutral_btn.state == press) ||
				(reverse_btn.state == press && drive_btn.state == press)   ||
				(neutral_btn.state == press && drive_btn.state == press)) {

				continue;
			}


			if (park_btn.state == press && park_btn.state != park_btn.last) {
				enable_LED_park();
				disable_LED_reverse();
				disable_LED_neutral();
				disable_LED_drive();
				printf("PARK\n");
				bkl_off += 1;

				if (bkl_off >= 10) {
					bkl_off = 0;

					if ((FTM1->SC & FTM_SC_PWMEN5_MASK) == FTM_SC_PWMEN5_MASK) {	// If bkl pwm ch is enabled, disable bkl
						disable_bkl();
					}
					else {
						enable_bkl();
					}
				}
			}
			else if (park_btn.state == open && park_btn.state != park_btn.last) {
				bkl_off += 1;
			}


			if (reverse_btn.state == press && reverse_btn.state != reverse_btn.last) {
				disable_LED_park();
				enable_LED_reverse();
				disable_LED_neutral();
				disable_LED_drive();

				bkl_off = 0;
			}
			else if (reverse_btn.state == open && reverse_btn.state != reverse_btn.last) {
				// PASS
			}


			if (neutral_btn.state == press && neutral_btn.state != neutral_btn.last) {
				disable_LED_park();
				disable_LED_reverse();
				enable_LED_neutral();
				disable_LED_drive();

				bkl_off = 0;
			}
			else if (neutral_btn.state == open && neutral_btn.state != neutral_btn.last) {
				// PASS
			}


			if (drive_btn.state == press && drive_btn.state != drive_btn.last) {
				disable_LED_park();
				disable_LED_reverse();
				disable_LED_neutral();
				enable_LED_drive();

				bkl_off = 0;
			}
			else if (drive_btn.state == open && drive_btn.state != drive_btn.last) {
				// PASS
			}
		}
	}
}

/***********************************************************************
 * Disables the watchdog timer module.
 *
 * @param void
 * @return void
 **********************************************************************/
void WDOG_disable(void) {
	WDOG->CNT = 0xD928C520; 												// Unlock watchdog
	WDOG->TOVAL = 0x0000FFFF; 												// Maximum timeout value
	WDOG->CS = 0x00002100; 													// Disable watchdog
}

/***********************************************************************
 * Initializes pins PTC0, PTC14, PTC16, PTB0 as GPIO inputs with pull
 * 		up resistors to use for the digital switch in each button.
 *
 * @param void
 * @return void
 **********************************************************************/
void init_digital_pins(void) {
	PCC->PCCn[PCC_PORTC_INDEX] |= 0x40000000;								// Write Clock Gate Control high to allow updating registers
	PCC->PCCn[PCC_PORTB_INDEX] |= 0x40000000;

	// Set Pin Control Registers
	PORTB->PCR[0] = 0x00000103;			 									// Puts pin in alternate mode 1 (GPIO use), enables and sets internal pull up resistor
	PORTC->PCR[0] = 0x00000103;
	PORTC->PCR[14] = 0x00000103;
	PORTC->PCR[16] = 0x00000103;

	// Set Port Data Direction Register for inputs
	PTB->PDDR &=~ (0x1);
	PTC->PDDR &=~ ((0x1 << 16) | (0x1 << 14) | 0x1);
}

/***********************************************************************
 * Initializes the btn structures as well as the nested analog and
 * 		digital contact structures.
 *
 * @param b Pointer to a btn structure
 * @param ch1 ADC channel to use for analog contact 1
 * @param adc1 ADC module to use for analog contact 1
 * @param ch2 ADC channel to use for analog contact 2
 * @param adc2 ADC module to use for analog contact 2
 * @return void
 **********************************************************************/
void init_btn(struct btn *b, uint8_t ch1, uint8_t adc1,
		      uint8_t ch2, uint8_t adc2) {

	enum state initial = none;

	struct analog_contact a1 = {0, 0, 0, 0, 0, 0, 0, 0, initial, 0, adc1, ch1};
	b->A1 = a1;

	struct analog_contact a2 = {0, 0, 0, 0, 0, 0, 0, 0, initial, 0, adc2, ch2};
	b->A2 = a2;

	struct digital_contact d1 = {0, 0, initial};
	b->D1 = d1;
}

/***********************************************************************
 * Given a analog contact structure, this function will perform the
 * 		debouncing and assign a state for the analog contact.
 *
 * @param a Pointer to analog contact structure to debounce
 * @return void
 *
 * @todo Report invalid high state over CAN
 * @todo Report invalid low state over CAN
 * @todo Report transition state over CAN
 **********************************************************************/
void debounce_analog_contact(struct analog_contact *a) {
	uint16_t v = 0;															// Stores the return value from the read_adc function

	if (a->adc == 0) {
		v = read_adc0(a->ch);
	}
	else {
		v = read_adc1(a->ch);
	}

	if (v < FH && v > FL && a->state != press) {							// If state is pressed
		a->press += 1;
		a->unpress = 0;

		if (a->press >= DL) {												// Update state when debounce limit is reached
			a->state = press;
			a->time = count;												// Set time equal to current time since startup whenever state changes
		}
	}
	else if (v < IH && v > IL && a->state != open) {						// If state is unpressed
		a->press = 0;
		a->unpress += 1;

		if (a->unpress >= DL) {
			a->state = open;
			a->time = count;
		}
	}
	else if (v < IL && v > FH && a->state != transition) {					// If state is transition
		if (a->transition_count <= 0)										// If the transition count is 0 start counting to 2500 samples
			a->transition_count = 1;

		a->press = 0;
		a->unpress = 0;
		a->transition += 1;
	}
	else if (v > IH && a->state != high) {		     						// If state is invalid high
		if (a->high_count <= 0)
			a->high_count = 1;

		a->invalid_high += 1;
		a->press = 0;
		a->unpress = 0;
	}
	else if (v < FL && a->state != low) {									// If state is invalid low
		if (a->low_count <= 0)
			a->low_count = 1;

		a->invalid_low += 1;
		a->press = 0;
		a->unpress = 0;
	}

	// Handles the 2000/2500 samples requirement for the transition region
	if (a->transition_count > 0  && a->state != transition) {				// If transition count has been started, increment every call to func
		a->transition_count += 1;

		if (a->transition_count >= 2500) {
			if (a->transition >= 2000) {
				a->state = transition;										// If 2000 of last 2500 samples were transition update state
				a->time = count;
			}
			// After 2500 samples, reset the transition variables
			a->transition = 0;
			a->transition_count = 0;
		}
	}

	// Handles 16/20 sample requirement for invalid low state
	if (a->low_count > 0  && a->state != low) {
		a->low_count += 1;

		if (a->low_count >= 20) {
			if (a->invalid_low >= 16) {
				a->state = low;												// If 16 of last 20 samples were invalid low update state
				a->time = count;
			}
			// After 20 samples, reset the variables
			a->invalid_low = 0;
			a->low_count = 0;
		}
	}

	// Handles 16/20 sample requirement for invalid high state
	if (a->high_count > 0  && a->state != high) {
		a->high_count += 1;

		if (a->high_count >= 20) {
			if (a->invalid_high >= 16) {
				a->state = high;											// If 16 of last 20 samples were invalid high update state
				a->time = count;
			}
			// After 20 samples, reset the variables
			a->invalid_high = 0;
			a->high_count = 0;
		}
	}
}

/***********************************************************************
 * Calls the debounce_analog_contact helper function to debounce both
 * 		analog contacts and debounces the digital contact. After
 * 		debouncing the individual contacts, their states are analyzed to
 * 		set the final debounced button state.
 *
 * @param b Pointer to a btn structure
 * @param digital_condition True or False condition based on GPIO input
 * 				register
 * @return void
 **********************************************************************/
void debounce_btn(struct btn *b, uint32_t digital_condition) {
	// Check analog switches
	debounce_analog_contact(&(b->A1));
	debounce_analog_contact(&(b->A2));

	// Check digital switch
	if (digital_condition == 0 && b->D1.state != press) {					// If state is pressed
		b->D1.press += 1;
		b->D1.unpress = 0;
		if (b->D1.press >= DL) {
			b->D1.state = press;
			b->D1.time = count;
		}
	}
	else if (digital_condition != 0 && b->D1.state != open) {	    		// If state is unpressed
		b->D1.press = 0;
		b->D1.unpress += 1;
		if (b->D1.unpress >= DL) {
			b->D1.state = open;
			b->D1.time = count;
		}
	}

	// Determine final debounced state based on the states of the three contacts
	if (b->A1.state == b->A2.state && b->A1.state == b->D1.state) {
		b->last = b->state;
		b->state = b->A1.state;												// If all three states are the same that as the overall button state
	}

//	printf("Analog 1: %d,\tAnalog 2: %d,\tDigital 1: %d\n", b->A1.state, b->A2.state, b->D1.state);   // DEBUG USE ONLY

}
