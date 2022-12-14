/***********************************************************************
 * This code implements phase 2 of HPB prototype. Buttons, LEDs, and CAN
 * 	functionality have been implemented.
 *
 * Author:  Zac Lynn
 * Version: Phase2.1
 * See:     https://www.nxp.com/docs/en/application-note/AN5303.pdf
 **********************************************************************/

#ifndef LED_H_
#define LED_H_

/***********************************************************************
 * Initializes PWM signals that drive the indicator and	backlight LEDs.
 * 		Initial duty cycles are also set here.
 *
 * @param void
 * @return void
 **********************************************************************/
void PWM_init(void);


/***********************************************************************
 * Decodes a CAN message into its corresponding LED command.
 *
 * @param unsigned char data us a byte array received from CAN
 * @param uint32_t id stores the type of commmand
 * @return uint8_t which is false if no LEDs were chnaged and true if
 * 						LED settings were udated
 **********************************************************************/
uint8_t LED_controls(unsigned char data[], uint32_t id);


/***********************************************************************
 * Enables FTMn output channels for the park indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void enable_LED_park(void);


/***********************************************************************
 * Disables FTMn output channels for the park indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void disable_LED_park(void);


/***********************************************************************
 * Enables FTMn output channels for the reverse indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void enable_LED_reverse(void);


/***********************************************************************
 * Disables FTMn output channels for the reverse indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void disable_LED_reverse(void);


/***********************************************************************
 * Enables FTMn output channels for the neutral indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void enable_LED_neutral(void);


/***********************************************************************
 * Disables FTMn output channels for the neutral indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void disable_LED_neutral(void);


/***********************************************************************
 * Enables FTMn output channels for the drive indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void enable_LED_drive(void);


/***********************************************************************
 * Disables FTMn output channels for the drive indicators.
 *
 * @param void
 * @return void
 **********************************************************************/
void disable_LED_drive(void);


/***********************************************************************
 * Enables FTMn output channels for the backlights.
 *
 * @param void
 * @return void
 **********************************************************************/
void enable_bkl(void);


/***********************************************************************
 * Disables FTMn output channels for the backlights.
 *
 * @param void
 * @return void
 **********************************************************************/
void disable_bkl(void);


/***********************************************************************
 * Sets the duty cycle for parking indicator 1 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_p1(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for parking indicator 2 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_p2(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for reverse indicator 1 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_r1(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for reverse indicator 2 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_r2(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for neutral indicator 1 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_n1(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for neutral indicator 2 as a percentage of the
 * 		PWM period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_n2(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for drive indicator 1 as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_d1(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for drive indicator 2 as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_d2(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for drive indicator 3 as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_d3(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for drive indicator 4 as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_d4(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for park backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_p(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for reverse backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_r(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for neutral backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_n(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for drive backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_d(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for c1 backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_c1(uint8_t brightness);


/***********************************************************************
 * Sets the duty cycle for c2 backlight as a percentage of the PWM
 * 		period. Used to dim/calibrate LEDs.
 *
 * @param brightness Integer between 0-100 to set duty cycle
 * 						proportionately
 * @return void
 **********************************************************************/
void set_bkl_c2(uint8_t brightness);


#endif /* LED_H_ */
