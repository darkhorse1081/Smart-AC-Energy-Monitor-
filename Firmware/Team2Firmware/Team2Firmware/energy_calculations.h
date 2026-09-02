/*
 * energy_calculations.h
 *
 * Created: 22/09/2024 3:58:58 pm
 *  Author: ryane
 */ 


#ifndef ENERGY_CALCULATIONS_H_ 
#define ENERGY_CALCULATIONS_H_ 

#define V_off 2000

void energy_calculations_init();

int16_t find_ac_voltage(uint16_t V_vf);
int16_t find_load_current(uint16_t V_if);

void conversion_to_ac_voltage();
void conversion_to_load_current();

void get_voltage_rms();
void get_current_rms();
void get_current_peak();
void power();

uint16_t abs_(int16_t value);
uint32_t integer_sqrt(uint32_t value);
uint32_t integer_rounding_by_1000(uint32_t value);

#endif /* ENERGY_CALCULATIONS_H_ */