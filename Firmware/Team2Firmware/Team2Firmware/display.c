/*
*	Author: TEAM TECH TITANS TWO
*
*	PORTS NEED TO BE CHECKED!!!
*/

/* STANDARD HEADER FILES */
#include <avr/io.h>

/* OUR HEADER FIELS */
#include "display.h"

static const uint8_t digit_array[10] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
static volatile uint8_t disp_character[4] = {0, 0, 0, 0};
static volatile uint8_t disp_position = 0;

extern volatile uint8_t dp_pos;
extern volatile uint8_t last_num;

void display_init() {
	DDRC |= SET_DISPLAY_NUM_AS_OUTPUT;
	DDRD |= SET_DISPLAY_DIGIT_AS_OUTPUT;
}


void toggle_SH_CP() {
	PORTC ^= (1 << SH_CP);
	PORTC ^= (1 << SH_CP); 
}

void toggle_SH_ST() {
	PORTC ^= (1 << SH_ST);
	PORTC ^= (1 << SH_ST);
}

void display_digit(uint8_t digit) {
	if (digit == 3) {
		PORTD &= ~(1 << Ds4); // Turns on digit 4
	} else if (digit == 2) {
		PORTD &= ~(1 << Ds3); // Turns on digit 3
	} else if (digit == 1) {
		PORTD &= ~(1 << Ds2); // Turns on digit 2
	} else if (digit == 0) {
		PORTD &= ~(1 << Ds1); // Turns on digit 1
	}
}

void seperate_and_load_characters(uint16_t number, uint8_t decimal_pos) {
	if (number != 0) {
		if (decimal_pos == 0) { // Gets the individual digits of the input number
			disp_character[0] = digit_array[number / 1000];
			} else if (decimal_pos == 1) {
			disp_character[1] = digit_array[(number / 100) % 10];
			} else if (decimal_pos == 2) {
			disp_character[2] = digit_array[(number / 10) % 10];
			} else if (decimal_pos == 3) {
			disp_character[3] = digit_array[number % 10];
		}
		
		if (dp_pos == 0) { // Depending on what is being displayed we turn on or off the dp
			disp_character[1] |= 1 << 7;
			} else if (dp_pos == 1) {
			disp_character[2] |= 1 << 7;
			} else if (dp_pos == 2) {
			disp_character[0] |= 1 << 7;
		}
	} else { // If the titles is to be displayed we load in the appropriate values for title.
		if (last_num == 2) {
			disp_character[0] = W1;
			disp_character[1] = W2;
			disp_character[2] = A;
			disp_character[3] = T;
		} else if (last_num == 0) {
			disp_character[0] = V;
			disp_character[1] = O;
			disp_character[2] = L;
			disp_character[3] = T;
		} else if (last_num == 1) {
			disp_character[0] = A;
			disp_character[1] = M1;
			disp_character[2] = M2;
			disp_character[3] = P;
		}
	}
}

void send_next_character_to_display() {
	
	PORTD |= CLEAR_DIGITS; // Turns off display, by sending them high so there is no voltage drop
	PORTC &= ~((1 << SH_CP) | (1 << SH_ST)); // Making sure they are both set to logic 0, will be pushed as overflow on loading
	
	uint8_t bit_pattern = disp_character[disp_position];
	
	for (uint8_t i = 0b10000000; i > 0; i >>= 1) {
		if (i & bit_pattern) {
			PORTC |= (1 << SH_DS); // Sending a high;
			} else {
			PORTC &= ~(1 << SH_DS); // Sending a low;
		}
		toggle_SH_CP(); // Shifting register
	}
	toggle_SH_ST(); // Loading latch
	
	display_digit(disp_position); // displaying digits
	
	if (disp_position == 3) { // updating digit to be displayed
		disp_position = 0;
		} else {
		disp_position++;
	}
}