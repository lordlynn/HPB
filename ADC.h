/***********************************************************************
 * This code initializes ADC0 and ADC1 and checks that no bits are stuck
 * 		high or low. This code also contains functions to read voltages
 * 		using both ADC modules.
 *
 * Author:  Zac Lynn
 * Version: Phase2.1
 * See: 	https://www.nxp.com/docs/en/application-note/AN12217.pdf
 **********************************************************************/
#ifndef ADC_H_
#define ADC_H_


/***********************************************************************
 * Initializes ADC input channels and call check_adc function. ADC is
 * 		initialized in 10-bit mode.
 *
 * @param void
 * @return void
 *
 * @todo Determine if ADCs should be in 10 or 12 bit mode
 **********************************************************************/
void init_adc(void);


/***********************************************************************
 * Checks ADC integrity by reading the reference voltage and confirming
 * 		all bits are high, then reads ground and confirms all bits are
 * 		low. Additionally, the bandgap voltage is checked here.
 *
 * @param void
 * @return void
 **********************************************************************/
void check_adc(void);


/***********************************************************************
 * Starts a voltage conversion using ADC0 on a specified channel.
 *
 * @param ch ADC0 input channel to read
 * @return Voltage reading in mV as an integer
 **********************************************************************/
uint16_t read_adc0(uint8_t ch);


/***********************************************************************
 * Starts a voltage conversion using ADC1 on a specified channel.
 *
 * @param ch ADC1 input channel to read
 * @return Voltage reading in mV as an integer
 **********************************************************************/
uint16_t read_adc1(uint8_t ch);


#endif /* ADC_H_ */
