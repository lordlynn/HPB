/***********************************************************************
 * This code implements phase 2 of HPB prototype. Buttons, LEDs, and CAN
 * 		functionality have been implemented.
 *
 * Author: 	Zac Lynn
 * Version: Phase2.1
 **********************************************************************/
#include "S32K146.h" 														// Include peripheral declarations S32K144
#include "stdlib.h"

// Custom libraries
#include "ClockInit.h"
#include "TimerInterrupt.h"
#include "ADC.h"
#include "LED.h"
#include "CAN.h"

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

/** Maximum difference between count and btn.time such that two button presses are considered simultaneous. This amount is added to the button debounce time as padding **/
#define SIMULTANEOUS_BTN_TIME 75

/**	Maximum allowed time in ms between digital and analog contacts closing for debouncing**/
#define BTN_TIMEOUT 150

/** Maximum time in ms to try sending a CAN message before allowing the next message to try to send **/
#define CAN_TX_TIMEOUT 5000

/** Keeps track of the state of individual contacts as well as the overall debounced state for a button **/
enum state {
	/** When the analog contact voltage is less than the pressed range **/
	low=1,

	/** When the analog contact voltage is within the pressed range **/
	press=2,

	/** When the analog contact voltage is between the pressed and open ranges **/
	transition=3,

	/** When the analog contact voltage is within the open range **/
	open=4,

	/** When the analog contact voltage is greater than the open range **/
	high=5,

	/** Initial state. If this state persists something is wrong **/
	none=0
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

	/** Stores the most recent voltage reading **/
	uint16_t voltage;

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

	/** Value of count variable at last state change **/
	uint32_t time;
};

void WDOG_disable(void);														// Disables WDT. USED FOR EARLY DEV ONLY. ADD WDT LATER
void init_digital_pins(void);													// Initialize PTC0,14,16 and PTB0 as GPIO inputs with pull up resistors
void init_btn(struct btn *b, uint8_t ch1, uint8_t adc1,							// Initializes the nested structs
		      uint8_t ch2, uint8_t adc2);
void enable_button_source(uint8_t en);											// Sets the 5v_GRPx_nEN outputs HIGH using one hot encoded parameter
void disable_button_source(uint8_t en);											// Sets the 5v_GRPx_nEN outputs LOW using one hot encoded parameter
void debounce_btn(struct btn *b, uint32_t digital_condition,
								unsigned char btn_id);							// Debounces a button with 2 analog and 1 digital contacts
void debounce_analog_contact(struct analog_contact *a);							// Helper function for debounce_btn
void btn_menu(struct btn *park_btn, struct btn *reverse_btn,
			  struct btn *neutral_btn, struct btn *drive_btn);					// Button menu function updates LEDs and performs actions according to button states
int is_valid_press(struct btn *b, uint32_t *time);
int btn_diagnostic_report(struct btn *b, char btn_id);

int main(void) {
	WDOG_disable(); 															// Disable Watchdog

	SOSC_init_16MHz(); 															// Initialize 16MHz external system oscillator (xtal)
	SPLL_init_160MHz(); 														// Initialize SPLL to 160 MHz with 16MHz MHz SOSC
	NormalRUNmode_80MHz(); 														// Sysclk = 80MHz, core = 40Mhz,  flash = 20MHz
	start_systick(1.0);															// Start systick with interrupt every 1ms. global variable "count" increments on every interrupt

	CAN0_init();

	PWM_init();																	// Initializes LED pins
	init_adc();																	// Initializes analog switch pins
	init_digital_pins();														// Initializes digital switch pins

	// Structures used for debouncing for each button
	struct btn park_btn;
	struct btn reverse_btn;
	struct btn neutral_btn;
	struct btn drive_btn;

	// Initialize the structure's variables to their starting values
	init_btn(&park_btn,     9, 0, 8, 1);										// ADC0-CH9  and  ADC1-CH8
	init_btn(&reverse_btn, 15, 0, 2, 1);										// ADC0-CH15 and  ADC1-CH2
	init_btn(&neutral_btn, 13, 0, 3, 1);										// ADC0-CH13 and  ADC1-CH3
	init_btn(&drive_btn,    5, 0, 6, 1);										// ADC0-CH5  and  ADC1-CH1

	// On startup have all LEDs off, turn on bkl and park indicator on first btn press
	enable_button_source(0x7);													// 0x7 enables 5V_GRPx_nEN channels 1-3 to power the buttons

	// Set backlight intensity. Default 50%
	set_bkl_c1(50);
	set_bkl_c2(50);
	set_bkl_p(99);
	set_bkl_r(99);
	set_bkl_n(99);
	set_bkl_d(20);

	// Set indicator intensity. Default 50%
	set_p1(13);
	set_p2(13);
	set_r1(11);
	set_r2(11);
	set_n1(13);
	set_n2(13);

	uint32_t last_BTN = 0;
	uint32_t last_CAN = 0;

	while (1) {
		if (millis % 5 == 0 && millis != last_BTN) {							// Every 5ms try to debounce buttons and respond accordingly

			debounce_btn(&park_btn,    (PTC->PDIR & (0x1 << 0)),  'P');
			debounce_btn(&reverse_btn, (PTC->PDIR & (0x1 << 16)), 'R');
			debounce_btn(&neutral_btn, (PTC->PDIR & (0x1 << 14)), 'N');
			debounce_btn(&drive_btn,   (PTB->PDIR & (0x1 << 0)),  'D');

			btn_menu(&park_btn, &reverse_btn, &neutral_btn, &drive_btn);

			last_BTN = millis;
		}
		if (millis % 50 == 0 && millis != last_CAN) {							// Every 50 ms check if a CAN msg was received
			if ((CAN0->IFLAG1 >> 4) & 1) {										// Check if an LED on/off command was received
				CAN0_receive_msg(4);
			}
			else if  ((CAN0->IFLAG1 >> 5) & 1) {								// Check if an LED tuning command was received
				CAN0_receive_msg(5);
			}
			else
				continue;														// If no new message was read into buffers then do not try to decode LED commands

			if (((RxID >> 26) & 0x7) == 1 || ((RxID >> 26) & 0x7) == 2) {		// If an LED command was recieved, decode it
				LED_controls(RxDATA, RxID);
			}
			last_CAN = millis;
		}
	}
}


/***********************************************************************
 * Given the button structures for each button, this function reacts to
 * 		buttons being pressed by turning LEDs on/off and sending CAN
 * 		reports. Simultaneous button push ignoring is handled here.
 *
 * @param park_btn Pointer to park button structure
 * @param reverse_btn Pointer to reverse button structure
 * @param neutral_btn Pointer to neutral button structure
 * @param drive_btn Pointer to drive button structure
 *
 * @return void
 **********************************************************************/
void btn_menu(struct btn *park_btn, struct btn *reverse_btn,
			  struct btn *neutral_btn, struct btn *drive_btn) {
	static uint8_t bkl_off = 0;
	static int ISFIRST = 0;														// USED TO CREATE THE WAKE UP FUNCTION. REMOVE AFTER DEMO

	/* Check that two buttons are not pushed at the same time. If two
	 * buttons are pushed at the same time the last state is set to
	 * press so that neither button can enter their respective press
	 * function below */
	if (park_btn->state == press) {
		if (reverse_btn->state == press) {
			reverse_btn->last = press;
			park_btn->last = press;
		}
		if (neutral_btn->state == press) {
			neutral_btn->last = press;
			park_btn->last = press;
		}
		if (drive_btn->state == press) {
			drive_btn->last = press;
			park_btn->last = press;
		}
	}
	else if (reverse_btn->state == press) {
		if (neutral_btn->state == press) {
			neutral_btn->last = press;
			reverse_btn->last = press;
		}
		if (drive_btn->state == press) {
			drive_btn->last = press;
			reverse_btn->last = press;
		}
	}
	else if (drive_btn->state == press) {
		if (neutral_btn->state == press) {
			neutral_btn->last = press;
			drive_btn->last = press;
		}
	}

	// If only one button is pushed

	if (park_btn->state == press && park_btn->state != park_btn->last &&
			millis - park_btn->time >= SIMULTANEOUS_BTN_TIME) {
		park_btn->last = park_btn->state;


		if (ISFIRST == 0) {														// THIS STATEMENT IS USED TO "WAKE UP" ON FIRST BUTTON PRESS
			enable_LED_park();
			enable_bkl();
			ISFIRST = 1;
			return;
		}

		if (btn_diagnostic_report(park_btn, 'P')) {
			enable_LED_park();
			disable_LED_reverse();
			disable_LED_neutral();
			disable_LED_drive();
		}

		bkl_off += 1;
		if (bkl_off >= 5) {
			bkl_off = 0;

			if ((FTM0->SC & FTM_SC_PWMEN2_MASK) == FTM_SC_PWMEN2_MASK) {		// If bkl pwm ch is enabled, disable bkl
				disable_bkl();
			}
			else {
				enable_bkl();
			}
		}
	}
	else if (park_btn->state == open && park_btn->state != park_btn->last) {
		park_btn->last = park_btn->state;
		btn_diagnostic_report(park_btn, 'P');
	}

	if (reverse_btn->state == press && reverse_btn->state != reverse_btn->last &&
			millis - reverse_btn->time >= SIMULTANEOUS_BTN_TIME) {
		reverse_btn->last = reverse_btn->state;

		if (ISFIRST == 0) {														// THIS STATEMENT IS USED TO "WAKE UP" ON FIRST BUTTON PRESS
			enable_LED_park();
			enable_bkl();
			ISFIRST = 1;
			return;
		}
		if (btn_diagnostic_report(reverse_btn, 'R')) {
			disable_LED_park();
			enable_LED_reverse();
			disable_LED_neutral();
			disable_LED_drive();
		}
		bkl_off = 0;
	}
	else if (reverse_btn->state == open && reverse_btn->state != reverse_btn->last) {
		reverse_btn->last = reverse_btn->state;
		btn_diagnostic_report(reverse_btn, 'R');
	}

	if (neutral_btn->state == press && neutral_btn->state != neutral_btn->last &&
			millis - neutral_btn->time >= SIMULTANEOUS_BTN_TIME) {
		neutral_btn->last = neutral_btn->state;

		if (ISFIRST == 0) {														// THIS STATEMENT IS USED TO "WAKE UP" ON FIRST BUTTON PRESS
			enable_LED_park();
			enable_bkl();
			ISFIRST = 1;
			return;
		}
		if (btn_diagnostic_report(neutral_btn, 'N')) {
			disable_LED_park();
			disable_LED_reverse();
			enable_LED_neutral();
			disable_LED_drive();
		}

		bkl_off = 0;
	}
	else if (neutral_btn->state == open && neutral_btn->state != neutral_btn->last) {
		neutral_btn->last = neutral_btn->state;
		btn_diagnostic_report(neutral_btn, 'N');
	}

	if (drive_btn->state == press && drive_btn->state != drive_btn->last &&
			millis - drive_btn->time >= SIMULTANEOUS_BTN_TIME) {
		drive_btn->last = drive_btn->state;

		if (ISFIRST == 0) {														// THIS STATEMENT IS USED TO "WAKE UP" ON FIRST BUTTON PRESS
			enable_LED_park();
			enable_bkl();
			ISFIRST = 1;
			return;
		}
		if (btn_diagnostic_report(drive_btn, 'D')) {
			disable_LED_park();
			disable_LED_reverse();
			disable_LED_neutral();
			enable_LED_drive();
		}
		bkl_off = 0;
	}
	else if (drive_btn->state == open && drive_btn->state != drive_btn->last) {
		drive_btn->last = drive_btn->state;
		btn_diagnostic_report(drive_btn, 'D');
	}
}


/***********************************************************************
 * Given a pointer to a button structure and character identifier, this
 * 		function reports over CAN whether a good or bad state change
 * 		has occurred.
 *
 * @param b Pointer to button structure
 * @param btn_id character identifier to use when sending button
 * 			diagnostics over CAN
 * @return Returns true if the button press was good and false if it was
 * 			bad.
 **********************************************************************/
int btn_diagnostic_report(struct btn *b, char btn_id) {
	uint32_t time = 0xFFFFFFFF;
	uint32_t timeout = millis;

	if (is_valid_press(b, &time)) {												// Report goood state changes
		while (!(CAN0->IFLAG1 & 0x1) && millis - timeout < CAN_TX_TIMEOUT);		// Wait until last message has succeeded to send next message
		CAN0_transmit_msg((unsigned char[]) {btn_id, b->state, b->A1.state,
				  b->A2.state, b->D1.state, (time & 0xFF000000) >> 24,
				  (time & 0x00FF0000) >> 16, (time & 0x0000FF00) >> 8,
				  (time & 0x000000FF)}, 9, 3);
		return 1;
	}
	else {																		// Report bad state changes
		while (!(CAN0->IFLAG1 & 0x1) && millis - timeout < CAN_TX_TIMEOUT);
		CAN0_transmit_msg((unsigned char[]) {btn_id, b->state,
				b->A1.state, (b->A1.voltage & 0xFF00) >> 8, b->A1.voltage & 0x00FF,
				b->A2.state, (b->A2.voltage & 0xFF00) >> 8, b->A2.voltage & 0x00FF,
				b->D1.state, (time & 0xFF000000) >> 24, (time & 0x00FF0000) >> 16,
				(time & 0x0000FF00) >> 8, time & 0x000000FF}, 13, 4);
		return 0;
	}
}


/***********************************************************************
 * Given a button struct and a pointer to a uint32_t, this function
 * 		determines if a good state change has taken place and saves the
 * 		delta time from the digital and closest analog contact.
 *
 * @param b Pointer to button struct
 * @param time Pointer to uint32_t which will hold delta time
 * @return returns true for good state change and false if bad state
 * 				change
 **********************************************************************/
int is_valid_press(struct btn *b, uint32_t *time) {
	uint32_t times[2];

	/* Check that the digital contact and the analog contacts have the
	 * same state before calculating the delta time between their state
	 * changes. The delta time makes no sense unless at least two contacts
	 * have closed, and if that is the case only the shortest matters
	 * since it only takes two contacts to make a good press/open */
	if (b->D1.state == b->A1.state)
		times[0] = abs(b->A1.time - b->D1.time);
	else
		times[0] = 0xFFFFFFFF;													// If no time is used for these contacts set the variable to 32bit int max
	if (b->D1.state == b->A2.state)
		times[1] = abs(b->A2.time - b->D1.time);
	else
		times[1] = 0xFFFFFFFF;

	/* Digital contact + 1 analog contact is required for debounce,
	 * thus timeout failure occurs when neither analog contact
	 * debounces within the timeout limit to the digitial contact.
	 *
	 * If the delta time is reported over CAN as all Fs then ignore it */
	if (times[0] < times[1])
		*time = times[0];
	else
		*time = times[1];

	/* Check for invalid press state. i.e. digital contact required and
	 * at least 1 analog contact. Check for invalid open states */
	if ((b->D1.state != press || (b->A1.state != press && b->A2.state != press)) &&
	    (b->D1.state != open  || (b->A1.state != open  && b->A2.state != open)))
		return 0;


	if (times[0] < BTN_TIMEOUT || times[1] < BTN_TIMEOUT) {
		return 1;
	}
	return 0;
}


/***********************************************************************
 * Given a analog contact structure, this function will perform the
 * 		debouncing and assign a state for the analog contact.
 *
 * @param a Pointer to analog contact structure to debounce
 *
 * @return void
 **********************************************************************/
void debounce_analog_contact(struct analog_contact *a) {
	uint16_t v = 0;																// Stores the return value from the read_adc function

	if (a->adc == 0) {
		v = read_adc0(a->ch);
	}
	else {
		v = read_adc1(a->ch);
	}
	a->voltage = v;

	if (v < FH && v > FL && a->state != press) {								// If state is pressed
		a->press += 1;
		a->unpress = 0;

		if (a->press >= DL) {													// Update state when debounce limit is reached
			a->state = press;
			a->time = millis;													// Set time equal to current time since startup whenever state changes
		}
	}
	else if (v < IH && v > IL && a->state != open) {							// If state is unpressed
		a->press = 0;
		a->unpress += 1;

		if (a->unpress >= DL) {
			a->state = open;
			a->time = millis;
		}
	}
	else if (v < IL && v > FH && a->state != transition) {						// If state is transition
		if (a->transition_count <= 0)											// If the transition count is 0 start counting to 2500 samples
			a->transition_count = 1;

		a->press = 0;
		a->unpress = 0;
		a->transition += 1;
	}
	else if (v > IH && a->state != high) {		     							// If state is invalid high
		if (a->high_count <= 0)
			a->high_count = 1;

		a->invalid_high += 1;
		a->press = 0;
		a->unpress = 0;
	}
	else if (v < FL && a->state != low) {										// If state is invalid low
		if (a->low_count <= 0)
			a->low_count = 1;

		a->invalid_low += 1;
		a->press = 0;
		a->unpress = 0;
	}

	// Handles the 2000/2500 samples requirement for the transition region
	if (a->transition_count > 0  && a->state != transition) {					// If transition count has been started, increment every call to func
		a->transition_count += 1;

		if (a->transition_count >= 2500) {
			if (a->transition >= 2000) {
				a->state = transition;											// If 2000 of last 2500 samples were transition update state
				a->time = millis;
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
				a->state = low;													// If 16 of last 20 samples were invalid low update state
				a->time = millis;
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
				a->state = high;												// If 16 of last 20 samples were invalid high update state
				a->time = millis;
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
 * @param btn_id character identifier to use when sending button
 * 				diagnostics over CAN
 * @return void
 **********************************************************************/
void debounce_btn(struct btn *b, uint32_t digital_condition, unsigned char btn_id) {
	uint8_t flag = 0;
	// Check analog switches
	debounce_analog_contact(&(b->A1));
	debounce_analog_contact(&(b->A2));

	// Check digital switch
	if (digital_condition == 0 && b->D1.state != press) {						// If state is pressed
		b->D1.press += 1;
		b->D1.unpress = 0;
		if (b->D1.press >= DL) {
			b->D1.state = press;
			b->D1.time = millis;
		}
	}
	else if (digital_condition != 0 && b->D1.state != open) {	    			// If state is unpressed
		b->D1.press = 0;
		b->D1.unpress += 1;
		if (b->D1.unpress >= DL) {
			b->D1.state = open;
			b->D1.time = millis;
		}
	}

	// Determine final debounced state based on the states of the three contacts
	// -- Code that requires the digital contact and a single analog contact contacts in agreement
	if (b->D1.state == press && (b->A1.state == press || b->A2.state == press) && b->state != press) {
		b->state = press;
		b->time = millis;
	}
	else if (b->D1.state == open && (b->A1.state == open || b->A2.state == open) && b->state != open) {
		b->state = open;
		b->time = millis;
	}
	else {																		// Used to report invalid states
		if (b->D1.state != open && b->D1.state != press) flag++;
		if (b->A1.state != open && b->A1.state != press) flag++;
		if (b->A2.state != open && b->A2.state != press) flag++;

		if (flag >= 2) {
			if (b->D1.state == b->A1.state || b->D1.state == b->A2.state) {
				b->state = b->D1.state;
			}
			else if (b->A1.state == b->A2.state) {
				b->state = b->A1.state;
			}
			else {
				b->state = none;
			}

			b->time = millis;

			if (b->last != b->state) {
				b->last = b->state;
				btn_diagnostic_report(b, btn_id); 								// If at least 2 contacts are debounced outside of press or open states report invalid state
			}
		}
	}
}


/***********************************************************************
 * Initializes pins PTC0, PTC14, PTC16, PTB0 as GPIO inputs with pull
 * 		up resistors to use for the digital switch in each button.
 * 		Additionally the 5V_GRPx_nEN pins are initialized here and left
 * 		LOW.
 *
 * @param void
 * @return void
 **********************************************************************/
void init_digital_pins(void) {
	// Button pins
	PCC->PCCn[PCC_PORTC_INDEX] |= 0x40000000;									// Write Clock Gate Control high to allow updating registers
	PCC->PCCn[PCC_PORTB_INDEX] |= 0x40000000;

	// Set Pin Control Registers
	PORTB->PCR[0] = 0x00000103;			 										// Puts pin in alternate mode 1 (GPIO use), enables and sets internal pull up resistor
	PORTC->PCR[0] = 0x00000103;
	PORTC->PCR[14] = 0x00000103;
	PORTC->PCR[16] = 0x00000103;

	// Set Port Data Direction Register for inputs
	PTB->PDDR &=~ (0x1);
	PTC->PDDR &=~ ((0x1 << 16) | (0x1 << 14) | 0x1);

	// 5V_GRPx_nEN pins
	PCC->PCCn[PCC_PORTD_INDEX] |= 0x40000000;

	PORTB->PCR[8] = 0x00000102;													// Sets pins in GPIO mode with pulldown resistor enabled
	PORTB->PCR[11] = 0x00000102;
	PORTD->PCR[17] = 0x00000102;

	PTB->PDDR |= (0x1 << 8) | (0x1 << 11);										// Set pins as outputs
	PTD->PDDR |= (0x1 << 17);

	PTB->PSOR &=~ (0x1 << 8) | (0x1 << 11);										// Set outputs low
	PTD->PSOR &=~ (0x1 << 17);
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

	enum state initial = open;
	b->time = 0;
	b->state = initial;
	b->last = initial;

	struct analog_contact a1 = {0, 0, 0, 0, 0, 0, 0, 0, initial, 0, 0, adc1, ch1};
	b->A1 = a1;

	struct analog_contact a2 = {0, 0, 0, 0, 0, 0, 0, 0, initial, 0, 0, adc2, ch2};
	b->A2 = a2;

	struct digital_contact d1 = {0, 0, initial};
	b->D1 = d1;
}


/***********************************************************************
 * Enables the 5V_GRPx_nEN lines which provide 5v to the buttons. The
 * 		input parameter should use one hot encoding where:
 * 				0x01 - enables 5V_GRP1_nEN - B11
 * 				0x02 - enables 5V_GRP2_nEN - B8
 * 				0x04 - enables 5V_GRP3_nEN - D17
 *
 * @param en One hot encoded byte. Bits 0-2 enable outputs 1-3.
 *
 * @return void
 **********************************************************************/
void enable_button_source(uint8_t en) {
	if ((en & 0x1) == 0x1) {
		PTB->PSOR |= (0x1 << 11);
	}
	if ((en & 0x2) == 0x2) {
		PTB->PSOR |= (0x1 << 8);
	}
	if ((en & 0x4) == 0x4) {
		PTD->PSOR |= (0x1 << 17);
	}
}


/***********************************************************************
 * Disables the 5V_GRPx_nEN lines which provide 5v to the buttons. The
 * 		input parameter should use one hot encoding where:
 * 				0x01 - disables 5V_GRP1_nEN - B11
 * 				0x02 - disables 5V_GRP2_nEN - B8
 * 				0x04 - disables 5V_GRP3_nEN - D17
 *
 * @param en One hot encoded byte. Bits 0-2 enable outputs 1-3.
 *
 * @return void
 **********************************************************************/
void disable_button_source(uint8_t en) {
	if ((en & 0x1) == 0x1) {
		PTB->PSOR &=~ (0x1 << 11);
	}
	if ((en & 0x2) == 0x2) {
		PTB->PSOR &=~ (0x1 << 8);
	}
	if ((en & 0x4) == 0x4) {
		PTD->PSOR &=~ (0x1 << 17);
	}
}


/***********************************************************************
 * Disables the watchdog timer module.
 *
 * @param void
 * @return void
 **********************************************************************/
void WDOG_disable(void) {
	WDOG->CNT = 0xD928C520; 													// Unlock watchdog
	WDOG->TOVAL = 0x0000FFFF; 													// Maximum timeout value
	WDOG->CS = 0x00002100; 														// Disable watchdog
}
