/*
*	Author: TEAM TECH TITANS TWO
*
*	Important specifications:
*		- We are using AVCC which is a 5V reference.
*		- If set to 10kHz we will sample a wave every 100us, if the wave is 500Hz then it will capture 20 samples in one
*		full cycle.
*		- Voltage Range: *** 5V ***
*		- 10 bit ADC means our resolution/Voltage-step is: 5V/(2^10bits) ~=  *** 4.88mV ***
*		- ATmega328P has an successive approximation register (SAR) ADC, this is lower
*		cost and size, low power consumption, adjustable resolution. Mid-range conversion
*		speed.
*		- f_sample(max) = 1 / (t_acquisition(min) + t_conversion(min))
*		- t_acquisition = 5 * time_constant; time_constant = (R_signal + R_sample) * C_sample
*			- allows the charging of the capacitor in order to capture a sample of input voltage
*		- t_conversion: one cycle per bit of resolution. *** 65us-260us ***
*		- ADC should be clocked between 50kHz and 200kHz to maximize accuracy.
*		- ADC uses multiplexer, so we have to shift between two ports to capture both
*		current and voltage.
*		- Single or continuous conversion mode.
*		- Interrupt on conversion completion available.
*		- Total conversion takes *** 13 cycles in normal conversion mode *** and *** 13.5 cycles
*		in auto triggered. ***
*/

/* OUR HEADER FILES */
#include "common.h"
#include "adc.h"
#include "uart.h"

/* STANDARD HEADER FILES */
#include <avr/io.h>
#include <avr/interrupt.h>

volatile extern uint8_t sample;
volatile extern uint8_t sample_type;
volatile extern uint8_t adc_fin;

// Variables are defined in "energy-calculations.c" thus must have the extern keyword. 
volatile extern uint16_t voltage_samples[32]; 
volatile extern uint16_t current_samples[32];

ISR(ADC_vect) {
	adc_fin = 1;
}

void adc_init() {
	ADMUX |= AVCC;
	ADCSRA |= ((1 << ADEN) | (1 << ADPS2) | (1 << ADIE) | (1 << ADATE));  // Interrupts enabled, trigger mode, 16 prescaler -> 125kHz, adc enable.
	ADCSRB |= ((1 << ADTS0) | (1 << ADTS1));							//Timer0 compare match A interrupt.
	DDRC &= ~(VOLTAGE_SIGNAL | CURRENT_SIGNAL);							// Setting to inputs
	//DIDR0 |= ((1 << ADC5D) | (1 << ADC4D) | (1 << ADC3D) | (1 << ADC2D)); // Setting bits to turn off buffer and save power on unused ADC inputs, ADC6 and 7 don't have one.
}  

void voltage_read() {
	voltage_samples[sample] = ADC;
}

void current_read() {
	current_samples[sample] = ADC;
}

 void adc_read(uint8_t input) {
	 // reads ADC into the correct sample array
	if (input == 0) {
		voltage_read();
	} else if (input == 1) {
		current_read();
	}
}

void adc_set_channel(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel); // Sets the channel of ADC to specified
}

uint16_t convert_adc_to_voltage(uint16_t value) {
	return ((uint32_t)value * RESOLUTION) / 1000; // Returns value in microvolts
}

void convert_adc_all() {
	// Converts the values from ADC into their corresponding voltage values.
	for (uint8_t i = 0; i < 32; i++) {
		voltage_samples[i] = convert_adc_to_voltage(voltage_samples[i]);
		current_samples[i] = convert_adc_to_voltage(current_samples[i]);
	}
}

