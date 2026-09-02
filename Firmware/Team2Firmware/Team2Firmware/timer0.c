/*
 * timer0.c
 *
 * Created: 22/09/2024 4:00:04 pm
 *  Author: ryane
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer0.h"
#include "adc.h"
#include "uart.h"

ISR(TIMER0_COMPA_vect) {} // Don't need an interrupt, using it to auto trigger the ADC

void timer0_init() {
	TCCR0A |= (1 << WGM01);				// Clear on compare match with ocr0a
	OCR0A = 248;						// Sets counter for timer to reset = 148us 
	TIMSK0 |= (1 << OCIE0A);			// Enables interrupts
}

void timer0_start_0prescaler() {
	TCNT0 = 0x00;						// Resets count of timer
	TCCR0B |= (1 << CS00);				// I/O Clock prescaler == 0, => resolution = 0.5us
}

void timer0_stop() {					// Stops timer0
	TCCR0B &= ~((7 << CS00));
}

