/*
 * display.h
 *
 * Created: 22/09/2024 3:58:37 pm
 *  Author: ryane
 */ 

// NEED TO CHECK ALL PORT ON Final.

#ifndef DISPLAY_H_
#define DISPLAY_H_

// Shift Register:
#define SH_CP PORTC3		// This is for clocking the 8bit output latch
#define SH_DS PORTC4		// This is for inputing data to the 8bit shift register
#define SH_ST PORTC5		// This is for clocking the 8bit shift register
#define SET_DISPLAY_NUM_AS_OUTPUT ((1 << SH_ST) | (1 << SH_DS) | (1 << SH_CP))

// Display Digits:
#define Ds1 PORTD4
#define Ds2 PORTD5
#define Ds3 PORTD6
#define Ds4 PORTD7
#define SET_DISPLAY_DIGIT_AS_OUTPUT ((1 << Ds1) | (1 << Ds2) | (1 << Ds3) | (1 << Ds4))

#define CLEAR_DIGITS 0b11110000 

#define V 0b00111110
#define O 0b00111111
#define L 0b00111000
#define T 0b01111000
#define A 0b01110111
#define M1 0b00110011
#define M2 0b00100111
#define P 0b01110011
#define W2 0b00011110
#define W1 0b00111100

#define ZERO 0x3F
#define ONE 0x06
#define TWO 0x5B
#define THREE 0x4F
#define FOUR 0x66
#define FIVE 0x6D
#define SIX 0x7D
#define SEVEN 0x07
#define EIGHT 0x7F
#define NINE 0x67

// Functions:

void display_init();
void toggle_SH_CP();
void toggle_SH_ST();
void display_digit(uint8_t digit);
void seperate_and_load_characters(uint16_t number, uint8_t decimal_pos);
void send_next_character_to_display();

#endif /* DISPLAY_H_ */