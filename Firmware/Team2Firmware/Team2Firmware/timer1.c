/*
*	Author: TEAM TECH TITANS TWO
*
*	This file is for the functions to implement the timer 1 processes.
*	
*	Ryan: Using this timer to set the LCD screen.
*/

/* STANDARD HEADER FILES */
#include <avr/io.h>
#include <avr/interrupt.h>

/* OUR HEADER FILES */
#include "uart.h"
#include "display.h"

volatile uint8_t decimal_pos = 0;
volatile uint16_t count_second = 0;
extern volatile uint16_t number; 

ISR(TIMER1_COMPA_vect){ // Updating the LED display needs to happen all at once so that it does not delay, thats why interrupt
	seperate_and_load_characters(number, decimal_pos);
	
	if (decimal_pos == 3) { // Which position to display
		decimal_pos = 0;
		} else {
		decimal_pos++;
	}
	
	send_next_character_to_display();
	count_second++;
}

void timer1_init() {
	TCCR1A |= 0;									// Normal mode
	OCR1A = 1999;									// 1ms to update LCD.
	TCCR1B |= (1 << WGM12) | (1 << CS10);		    // Clear on compare match with OCR1A, 1 prescaler               
	TIMSK1 |= (1 << OCIE1A);						// Enable Input Capture Interrupt for Timer1

}

void timer1_stop() {
	// Clear the clock select bits (CS12, CS11, CS10) to stop Timer1
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
}

