/*
 * interrupts.c
 *
 * Created: 22/09/2024 4:16:05 pm
 *  Author: ryane
 */ 

#define F_CPU 2000000UL

/* STANDARD HEADER FILES */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* OUR HEADER FILES */
#include "interrupts.h"
#include "timer0.h"
#include "adc.h"

volatile extern uint8_t sample_type;

ISR(INT0_vect) {
	EIMSK &= ~(1 << INT0); // Turns off external interrupt
	adc_set_channel(sample_type); // Sets ADC to sample correct port
	timer0_start_0prescaler(); // Starts timer0
}

void int0_init() {
	EICRA |= ((1 << ISC00) | (1 << ISC01));		// Rising edge
}