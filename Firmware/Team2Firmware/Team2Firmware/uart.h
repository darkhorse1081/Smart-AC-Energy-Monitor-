/*
 * uart.h
 *
 * Created: 22/09/2024 3:59:49 pm
 *  Author: ryane
 */ 


#ifndef UART_H_
#define UART_H_

#define UDR0_IS_NOT_READY (!(UCSR0A & (1<<UDRE0)))
#define TRANSMIT_ENABLE (1 << TXEN0)
#define EIGHTN1 ((1 << UCSZ01) | (1 <<UCSZ00))

void uart_init(uint16_t ubrr);
void uart_transmission(uint8_t data);
void transmit_number(uint8_t data);
void transmit_comma();
void transmit_newline();
void transmit_dp();
void transmit_large_number(uint16_t num);
void transmit_twos_complement(int16_t num);
void transmit_32bit_number(uint32_t num);
void transmit_title(char buffer[10]);
void transmit_voltage(uint32_t num);
void transmit_current(uint32_t num);
void transmit_power(uint32_t num);

#endif /* UART_H_ */