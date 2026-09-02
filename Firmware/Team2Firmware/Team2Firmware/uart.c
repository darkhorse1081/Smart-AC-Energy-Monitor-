/*
*	Author: TEAM TECH TITANS TWO
*
*	This file is for the functions to implement the UART processes.
*/

/* STANDARD HEADER FILES */
#include <avr/io.h>

/* OUR HEADER FILES */
#include "uart.h"

void uart_init(uint16_t ubrr) {
	UCSR0B |= TRANSMIT_ENABLE;
	UCSR0C |= EIGHTN1;
	UBRR0 = ubrr;						// Baud Rate Value
}

void uart_transmission(uint8_t data) {
	while (UDR0_IS_NOT_READY) { }			// Waits until UDR0 is ready before setting new data to be sent
	UDR0 = data;						// Sets new data to be sent
}

void transmit_number(uint8_t data) { // Transmits single digit
	uart_transmission(data + 48);
}

void transmit_comma() { 
	uart_transmission(44);
}

void transmit_newline() {
	uart_transmission(13); // Windows new line
	uart_transmission(10); // Mac and Linux new line
}

void transmit_dp() {
	uart_transmission(46); // Transmit decimal point
}

void transmit_large_number(uint16_t num) { // Transmit up to 6 digit number
	if (num == 0) {
		transmit_number(0);
		return;
	}

	uint8_t buffer[6];
	for (int8_t i = 0; i < 6; i++) {
		buffer[i] = num % 10;
		num /= 10;
	}
	for (int8_t i = 5; i >= 0; i--) {
		transmit_number(buffer[i]);
	}
}

void transmit_twos_complement(int16_t num) {
	uint8_t buffer[7]; // -32768 to +32767
	
	if (num < 0) {
		buffer[6] = 45;
		num = ~(num) + 1; // Converts into positive equivalent
		for (int8_t i = 0; i < 6; i++) {
			buffer[i] = 48 + num % 10;
			num /= 10;
		}
	} else {
		buffer[6] = 43;
		for (int8_t i = 0; i < 6; i++) {
			buffer[i] = 48 + num % 10;
			num /= 10;
		}
	}
	
	for (int8_t i = 6; i >= 0; i--) {
		uart_transmission(buffer[i]);
	}
}

void transmit_32bit_number(uint32_t num) { // Transmit up to 6 digit positive number
	uint8_t buffer[12];
	for (int8_t i = 0; i < 12; i++) {
		buffer[i] = num % 10;
		num /= 10;
	}
	for (int8_t i = 11; i >= 0; i--) {
		transmit_number(buffer[i]);
	}
}

void transmit_title(char buffer[10]) { // Allows the transmission of 9 character strings.
	for (int8_t i = 0; buffer[i] != '\0'; i++) {
		uart_transmission(buffer[i]);
	}
}

void transmit_voltage(uint32_t num) { // Transmits voltage with dp in correct location and correct units
	uint8_t buffer[4];
	for (int8_t i = 0; i < 4; i++) {
		buffer[i] = num % 10;
		num /= 10;
	}
	for (int8_t i = 3; i >= 0; i--) {
		if (i == 1) {
			transmit_dp();
		}
		transmit_number(buffer[i]);
	}
	uart_transmission('V');
}

void transmit_current(uint32_t num) { // Transmits current with correct dp placement/units
	uint8_t buffer[5];
	for (int8_t i = 0; i < 5; i++) {
		buffer[i] = num % 10;
		num /= 10;
	}
	for (int8_t i = 4; i >= 1; i--) {
		if (i == 1) {
			transmit_dp();
		}
		transmit_number(buffer[i]);
	}
	uart_transmission('m');
	uart_transmission('A');
}

void transmit_power(uint32_t num) { // Transmits power with correct dp/units
	uint8_t buffer[6];
	for (int8_t i = 0; i < 6; i++) {
		buffer[i] = num % 10;
		num /= 10;
	}
	for (int8_t i = 4; i >= 1; i--) {
		if (i == 3) {
			transmit_dp();
		}
		transmit_number(buffer[i]);
	}
	uart_transmission('W');
}





