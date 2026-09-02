/*
 * adc.h
 *
 * Created: 22/09/2024 3:58:15 pm
 *  Author: ryane
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>


#define AVCC (1 << REFS0)
#define VOLTAGE_SIGNAL (1 << PORTC0)
#define CURRENT_SIGNAL (1 << PORTC1)

#define RESOLUTION 4880 //uV

void adc_init();
void adc_read(uint8_t input);
void adc_set_channel(uint8_t channel);

void voltage_read();
void current_read();

uint16_t convert_adc_to_voltage(uint16_t value);
void convert_adc_all();

#endif /* ADC_H_ */