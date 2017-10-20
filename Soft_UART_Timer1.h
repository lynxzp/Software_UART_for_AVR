/*
 * Soft_UART_Timer1.h
 *
 * Software UART for ATmega328p using Timer1
 * 
 * Now only tranmission, no receiving
 */ 

#ifndef SOFTUARTTIMER1_H_
#define SOFTUARTTIMER1_H_

//////////////////////////////////////////////////////////////////////////
// Usage (in your .c file):
// 1. define Tx pin: DDR register, PORT register, PIN number:
// #define Soft_UART_TX_PORT PORTD
// #define Soft_UART_TX_DDR DDRD
// #define Soft_UART_TX_PIN 3
// 2. define UART speed
// #define Soft_UART_Baud 9600
// 3. include .h file:
// #include "Soft_UART_Timer1.h"
// 3. initialize software UART, required to call before usage other functions:
// static void Soft_UART_init(void);
// 4. send byte
// static void Soft_UART_send_byte(char byte);
// this function send byte in background,
// only one byte in background, other will wait.
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// From this line begins code to the end of file.
// All code written in .h file for better inline optimization.
// Try to include this file only once per project.
//////////////////////////////////////////////////////////////////////////

volatile char Soft_UART_busy_flag=0;
char Soft_UART_next_byte=0;

//////////////////////////////////////////////////////////////////////////
// Initialization software UART, required to call before usage
//////////////////////////////////////////////////////////////////////////
static void Soft_UART_init(void)
{   // Pin init
	bit_set(Soft_UART_TX_DDR,Soft_UART_TX_PIN);
	bit_set(Soft_UART_TX_PORT,Soft_UART_TX_PIN);
	// Timer1 init
	TCCR1A=(0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(1<<WGM11)|(0<<WGM10);
	TCCR1B=(0<<ICNC1)|(0<<ICES1)|(1<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(0<<CS10); // prescaler 0, timer stopped
	TCCR1C=(0<<FOC1A)|(0<<FOC1B);
	TIMSK1=(0<<ICIE1)|(0<<OCIE1B)|(0<<OCIE1A)|(0<<TOIE1);
	//ICR1=1666;
	ICR1=F_CPU/Soft_UART_Baud;
}

//////////////////////////////////////////////////////////////////////////
// Internal function, start timer, when need to send something
//////////////////////////////////////////////////////////////////////////
static void Soft_UART_Timer1_start(void)
{
	bit_set(TIMSK1,TOIE1);
	bit_set(TCCR1B,CS10);
}

//////////////////////////////////////////////////////////////////////////
// Internal function, stop timer, when nothing to send
//////////////////////////////////////////////////////////////////////////
static void Soft_UART_Timer1_stop(void)
{
	bit_clr(TIMSK1,TOIE1);
	bit_clr(TCCR1B,CS10);
}

//////////////////////////////////////////////////////////////////////////
// Send byte in background, only one byte in background, other - wait
//////////////////////////////////////////////////////////////////////////
static void Soft_UART_send_byte(char byte)
{
	while(Soft_UART_busy_flag)
		;
	Soft_UART_busy_flag=1;
	Soft_UART_Timer1_start();
	Soft_UART_next_byte=byte;
}

//////////////////////////////////////////////////////////////////////////
// Internal function, send next bit
//////////////////////////////////////////////////////////////////////////
ISR(TIMER1_OVF_vect)
{
	static char Soft_UART_pos=0;
	switch(Soft_UART_pos)
		{
			case 0: // send START bit
					bit_clr(Soft_UART_TX_PORT,Soft_UART_TX_PIN);
					break;
			case 9:	// send STOP bit
					bit_set(Soft_UART_TX_PORT,Soft_UART_TX_PIN);
					break;
			case 10:// end of transmission
					Soft_UART_busy_flag=0;
					Soft_UART_pos=0;
					Soft_UART_Timer1_stop();
					return;
			default:// send data bit
					if(bit_tst(Soft_UART_next_byte,Soft_UART_pos-1))
						bit_set(Soft_UART_TX_PORT,Soft_UART_TX_PIN);
					else
						bit_clr(Soft_UART_TX_PORT,Soft_UART_TX_PIN);
					break;
		}
	Soft_UART_pos++;
	bit_change(PORTB,5);
}
#endif /* SOFTUARTTIMER1_H_ */ 
