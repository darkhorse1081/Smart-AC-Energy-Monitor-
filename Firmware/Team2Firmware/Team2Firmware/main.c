/*
 * main.c
 *
 * Created: 16/08/2024 4:48:24 pm
 * Author : TEAM TECH TITANS TWO
 *
 * The aim of this project is to display the power dissipated by a source voltage 14 Vrms +- 10%
 * 500Hz +- 2% when a load of the range 2.5 VA to 7.5 VA is attached to it.
 *
 * Some important information:
 *		- ADC Conversion Rate = 10kHz or slower
 *		- LCD Display Information: Voltage (Vrms), Peak Current (mA) and Power (W)
 *		- LCD Scroll Rate = 1s
 *		- UART Specifications: 9600 Baud, 8N1 with no parity
 *		- Information Transferred via UART: Voltage, Peak Current, Power
 */ 

#define F_CPU 2000000UL // 2MHz CPU clock

/* STANDARD HEADER FILE */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* OUR HEADER FILES */
#include "common.h"
#include "uart.h"
#include "display.h"
#include "timer0.h"
#include "timer1.h"
#include "adc.h"
#include "interrupts.h"
#include "energy_calculations.h"

volatile uint16_t number = 0;
volatile uint8_t sample = 0;
volatile uint8_t sample_type = 0;

extern volatile uint16_t voltage_samples[32];
extern volatile uint16_t current_samples[32];

extern volatile int16_t voltage_values[32];
extern volatile int16_t current_values[32];

static volatile uint32_t prior_voltage[5] = {0,0,0,0,0};
static volatile uint32_t prior_current[5] = {0,0,0,0,0};
static volatile uint32_t prior_power[5] = {0,0,0,0,0};

static volatile uint32_t ave_voltage = 0;
static volatile uint32_t ave_current = 0;
static volatile uint32_t ave_power = 0;

volatile uint32_t voltage_rms = 0;
volatile uint32_t current_rms = 0;
volatile uint32_t peak_current = 0;
volatile uint32_t real_power = 0;

extern volatile uint16_t count_second;
volatile uint8_t count_multiple_second = 0;
volatile uint8_t last_num = 0;
volatile uint8_t dp_pos = 0;
volatile uint8_t title_num = 0;
volatile uint8_t adc_fin = 0;

/* DECLARED FUNCTIONS */
void get_voltage_samples();
void get_current_samples();
void update_last_five();
void estimate_average();

int main(void)
{	
	energy_calculations_init();
	uart_init(BAUD_RATE);
	display_init();
	adc_init();
	timer0_init();
	timer1_init();
	int0_init(); 
	sei();
    while (1) 
    {	
		if (count_second > 1000) { // Counts to 1 second.
			/* SAMPLING */
			get_voltage_samples(); 
			get_current_samples();
			
			/* CONVERSIONS */
			convert_adc_all(); // Converts the ADC value to the corresponding voltage
			conversion_to_ac_voltage();  // Converts V_vf to V_ac
			conversion_to_load_current(); // Converts V_if to I_L
			
			/* REQUIRED VALUES */
			get_voltage_rms();	// Uses Riemann sum to estimate the RMS Voltage
			get_current_rms();  // Uses Riemann sum to estimate the RMS Current
			get_current_peak(); // Multiplies the RMS Current by sqrt(2), Assuming that the input wave is Sinusoid
			power();			// Estimates Real power by P = I_rms * V_rms * p.f. , Assuming that the input wave is sinusoid
			
			/* AVERAGES */
			update_last_five(); // An array of the last 5 values in order to estimate the average.
			estimate_average(); // 
			
			/* UART TRANSMISSION */
			transmit_newline();
			transmit_title("V_rms: ");
			transmit_voltage(voltage_rms);
			transmit_newline();
			transmit_title("5 Ave: ");
			transmit_voltage(ave_voltage);
			
			transmit_newline();
			transmit_title("I_pk: ");
			transmit_current(peak_current);
			transmit_newline();
			transmit_title("5 Ave: ");
			transmit_current(ave_current);
			
			transmit_newline();
			transmit_title("Power: ");
			transmit_power(real_power);
			transmit_newline();
			transmit_title("5 Ave: ");
			transmit_power(ave_power);
			
			transmit_newline();
			transmit_title("---------");
			
			count_second = 0;
			count_multiple_second++;
		}
			
		if (count_multiple_second > 2) { // Counts to 3 seconds
			
			if (title_num == 1) {
				/* This if statement updates every three seconds to switch between 
				the next title and the next number on the LED screen. */
				if (last_num == 0) {
					dp_pos = 0;
					number = ave_voltage;
					last_num++;
					} else if (last_num == 1) {
					dp_pos = 1;
					number =  ave_current/10;
					last_num++;
					} else if (last_num == 2) {
					dp_pos = 2;
					number = ave_power/10;
					last_num = 0;
				}
				
				title_num = 0;
			} else {
				number = 0;
				title_num = 1;
			}
			
			count_second = 0;
			count_multiple_second = 0;
		}
	}
}

void get_voltage_samples() {
	TIMSK1 &= ~(1 << OCIE1A); // Turns off Timer1 interrupt
	sample = 0;				// Sets ADC to sample PC0/ADC0 -> V_vf
	sample_type = 0;
	EIMSK |= (1 << INT0);	// Turns on external interrupt, V_zc
	while (sample < 32) {	// Takes 32 samples of the current wave, equivalent to two wave lengths
		if (adc_fin) {		// Polls if the ADC finished its sampling and quantization
			adc_read(sample_type); // Reads the current value in the ADC into the appropriate array.
			sample++;
			adc_fin = 0;
		}
	}
	timer0_stop();			// Turns off timer 0
	TIMSK1 |= (1 << OCIE1A);  // Restarts timer1 interrupt
}

void get_current_samples() { // Refer to the above comments
	TIMSK1 &= ~(1 << OCIE1A);
	sample_type = 1; // Sets ADC to sample PC1/ADC1 -> V_if
	sample = 0;
	EIMSK |= (1 << INT0);
	while (sample < 32) {
		if (adc_fin) {
			adc_read(sample_type);
			adc_fin = 0;
			sample++;
		}
	}
	timer0_stop();
	TIMSK1 |= (1 << OCIE1A);
}

void update_last_five() {
	/* Updates the last 5 values into their respective arrays*/
	prior_power[4] = prior_power[3];
	prior_current[4] = prior_current[3];
	prior_voltage[4] = prior_voltage[3];
	
	prior_power[3] = prior_power[2];
	prior_current[3] = prior_current[2];
	prior_voltage[3] = prior_voltage[2];
	
	prior_power[2] = prior_power[1];
	prior_current[2] = prior_current[1];
	prior_voltage[2] = prior_voltage[1];
	
	prior_power[1] = prior_power[0];
	prior_current[1] = prior_current[0];
	prior_voltage[1] = prior_voltage[0];
	
	prior_power[0] = real_power;
	prior_current[0] = peak_current;
	prior_voltage[0] = voltage_rms;
}

void estimate_average() {
	/* Gets the average of the current,power,voltage arrays */
	uint32_t current_sum = 0;
	uint32_t power_sum = 0;
	uint32_t voltage_sum = 0;
	
	for (uint8_t i = 0; i < 5; i++) {
		current_sum += prior_current[i];
		voltage_sum += prior_voltage[i];
		power_sum += prior_power[i];
	}
	
	ave_power = power_sum / 5;
	ave_current = current_sum / 5;
	ave_voltage = voltage_sum / 5;
}

