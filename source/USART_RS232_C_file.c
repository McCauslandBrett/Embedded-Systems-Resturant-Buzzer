/*
 * USART_RS232_C_file.c
 * http://www.electronicwings.com
 *
 */

#include "USART_RS232_H_file.h"						/* Include USART header file */

void USART_Init(unsigned long BAUDRATE)				/* USART initialize function */
{
	// UCSR0B |= (1 << RXEN0) | (1 << TXEN0) //usart blocking
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);				/* Enable USART transmitter and receiver */
	UCSR0C |= (1 << UMSEL00)| (1 << UCSZ01) | (1 << UCSZ00);	/* Write USCRC for 8 bit data and 1 stop bit */
	UBRR0L = BAUD_PRESCALE;
	// Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRR0H = (BAUD_PRESCALE >> 8);
}

char USART_RxChar()									/* Data receiving function */
{
	while (!(UCSR0A & (1 << RXC0)));					/* Wait until new data receive */
	return(UDR0);									/* Get and return received data */
}

void USART_TxChar(char data)						/* Data transmitting function */
{
	UDR0 = data;										/* Write data to be transmitting in UDR */
	while (!(UCSR0A & (1<<UDRE0)));					/* Wait until data transmit and buffer get empty */
}

void USART_SendString(char *str)					/* Send string of USART data function */
{
	int i=0;
	while (str[i]!=0)
	{
		USART_TxChar(str[i]);						/* Send each char of string till the NULL */
		i++;
	}
}
