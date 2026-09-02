/*
*	Energy Calculations:
*		- Power
*		- voltage rms
*		- Current rms
*		- Peak current
*
*	If ADC is set to capture a sample every 0.1ms (10kHz).
*
*	Accurately estimating RMS Voltage.
*	Overestimating Peak Current.
*	Underestimating Real Power.
*	^ This is all in ideal conditions.
*/

/* STANDARD HEADER FILES */
#include <avr/io.h>

/* OUR HEADER FILES */
#include "energy_calculations.h"
#include "uart.h"

volatile uint16_t voltage_samples[32]; // These will be the sampled values
volatile uint16_t current_samples[32]; // These will be the sampled values

volatile int16_t voltage_values[32];	// Converted values
volatile int16_t current_values[32];	// Converted values

extern volatile uint32_t voltage_rms; 
extern volatile uint32_t current_rms; 
extern volatile uint32_t peak_current;
extern volatile uint32_t real_power;

void energy_calculations_init() {}

/*
* Voltage Measurement:
*
* G_vs: Step down voltage achieved by voltage divider
* G_vo: Differential amplification
* V_off: offset
*
* V_vf = G_vs * G_vo * V_AC + V_off
*
* Since this is the input of the signal, we need to rearrange in order
* to find the V_rms of the original V_AC.
*
* V_AC = (V_vf - V_off) / (G_vs * G_vo)
*/
int16_t find_ac_voltage(uint16_t V_vf) {
	return (((int32_t)V_vf - V_off) * 7067) / 321;
}

/*
* Current Measurement:
* 
* G_is: shunt resistor
* G_io: differential amplification
* V_off: offset
*
* V_if = G_is * G_io * I_L + V_off
*
* Since this is the input signal we need to rearrange in order
* to find I_L (A).
*
* I_L = (V_if - V_off) / (G_is * G_io)
*/
int16_t find_load_current(uint16_t V_if) {
	return ((((int32_t)((int16_t)V_if - 1981) * 1000) / 3540) * 3860) / 1000 ;
}


/*
*	So for reference we would have an array of many voltage values
*	Each of these values would need to be passed through the above
*	equations in order to be converted into their predictive V_AC/I_L
*	values.
*/

void conversion_to_ac_voltage() {
	for (uint8_t i = 0; i < 32; i++) {
		voltage_values[i] = find_ac_voltage(voltage_samples[i]);
	}
}

void conversion_to_load_current() {
	for (uint8_t i = 0; i < 32; i++) {
		current_values[i] = find_load_current(current_samples[i]);
	}
}

/* Since we are not using a dedicated circuit to work out the peak values
*  we will be estimating the values using the following formulas.
*
*  V_AC_rms = sqrt(1/T_p * integral(V_AC^2 dt, [T_p,0])
*
*  I_L_rms = sqrt(1/T_p * integral(I_L^2 dt, [T_p, 0])
*
*  Since we cannot sample and store a continuous wave, we must revert to using
*  discrete value to estimate the above quantities. We can do this using the
*  Riemann sum.
*
*  If we have taken N ADC samples at regular (delta)t_sample intervals over
*  one time period of the signal, the time period T_p = N(delta)t_sample
*
*  V_AC_rms^2 = (1 /(N * (delta)t_sample)) * sum(V_AC^2[i] * (delta)t_sample, [for i in range(N-1)])
*  I_L_rms^2 = (1 /(N * (delta)t_sample)) * sum(I_L^2[i] * (delta)t_sample, [for i in range(N-1)])
*
*/

void get_voltage_rms() {
	
	uint8_t n = 31; 
	uint32_t total = 0;
	
	for (uint8_t i = 1; i < 32; i++) { // weird values are being seen in the first sample, so we are just removing them.
		total += integer_rounding_by_1000((uint32_t)abs_(voltage_values[i])) * integer_rounding_by_1000((uint32_t)abs_(voltage_values[i])); 
		// Summing the square of the values
	}
	
	uint32_t riemann_value_squared = total/n;
	uint32_t riemann_value = integer_sqrt(riemann_value_squared);
	uint32_t make_up = 0;
	uint32_t difference = riemann_value_squared - (riemann_value * riemann_value);
	
	/* An integer dividing system to get the extra stuff through integer
	arithmetic, allows us to not use floats */
	if (difference > 2)
	{
		for (int i = 0; i < riemann_value; i++)
		{
			if (i * 2 >= difference)
			{
				make_up = i * (100 / riemann_value);
				break;
			}
		}
	}
	else if (difference == 0)
	{
		make_up = 0;
	}
	else
	{
		make_up = 1;
	}
	riemann_value *= 100; // Shifting the value to the right
	riemann_value += make_up; // Adding the extra stuff
	
	voltage_rms = riemann_value;
}

void get_current_rms() {
	
	uint8_t n = 31;
	uint32_t total = 0;
	
	for (uint8_t i = 1; i < 32; i++) {
		total += ((uint32_t)abs_(current_values[i])) * ((uint32_t)abs_(current_values[i]));
	}
	
	uint32_t riemann_value_squared = total/n;
	uint32_t riemann_value = integer_sqrt(riemann_value_squared);

	uint32_t make_up = 0;
	uint32_t difference = riemann_value_squared - (riemann_value * riemann_value);
	
	/* An integer dividing system to get the extra stuff through integer
	arithmetic, allows us to not use floats */
	if (difference > 2)
	{
		for (int i = 0; i < riemann_value; i++)
		{
			if (i * 2 >= difference)
			{
				make_up = i * (100 / riemann_value);
				break;
			}
		}
	}
	else if (difference == 0)
	{
		make_up = 0;
	}
	else
	{
		make_up = 1;
	}
	riemann_value *= 100; // Shifting value to the right
	riemann_value += make_up; // Adding the extra stuff
	
	current_rms = riemann_value;
}

void get_current_peak() {
	/* As it is possible to find the peak current by multiplying the values by sqrt(2), for
	an assumed sinusoid. We thought that this would be better as it is possible to is the peak of a wave when sampling. */
	peak_current = integer_rounding_by_1000(current_rms * 1414); 
}

/*
*  Estimating Power Using RMSs
*
*  P = V_rms * I_rms * p.f.
*		We are assuming that we can do this as the input wave is essentially a sinusoid.
*
*/

void power() {
	uint16_t i_max = 0;
	uint8_t time_max_i = 0;
	uint16_t v_max = 0;
	uint8_t time_max_v = 0;
	
	/* Using the timings from each of the first peaks (as there are two
	waveforms) to find the difference in time in order to find the power factor */
	
	for (uint8_t i = 1; i < 16; i++) { // Getting the peak values through iteration
		if (i_max < current_samples[i]) {
			i_max = current_samples[i];
			time_max_i = i;
		}
		if (v_max < voltage_samples[i]) {
			v_max = voltage_samples[i];
			time_max_v = i;
		}
	}
	
	uint16_t power_factor = (16 - (time_max_i - time_max_v))  * 100;
	uint16_t time_period = 2000;
	real_power = (integer_rounding_by_1000(current_rms * voltage_rms) * power_factor) / time_period;
}

uint16_t abs_(int16_t value) {
	// To find the absolute values of a number
	if (value < 0) {
		return ~(value) + 1;
	} else {
		return value;
	}
}

 uint32_t integer_sqrt(uint32_t value)
 {
	 if (value <= 0) {
		return 0; // Return 0 for non-positive values
	 }
	 uint32_t result = value;

	 // Continue iterating until the result stops changing
	 for (uint8_t i = 0; i < 15; i++)
	 {
		 result = (result + value / result) / 2; // Newton's iteration
	 }

	 return result;
 }

uint32_t integer_rounding_by_1000(uint32_t value) {
	// Integer division doesn't round it truncates, this allows us to round correctly
	value = value / 100;
	if (value % 10 > 4) {
		return value / 10 + 1;
	} else {
		return value / 10;
	}
}


