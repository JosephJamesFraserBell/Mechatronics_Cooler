/*
 * Wireless_Transmission.cpp
 *
 * Created: 7/23/2019 11:45:26 AM
 * Author : gabriel101
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


void initialize_usart(); // function to set up USART
void transmit_data_usart(unsigned char x);
void delay_T_msec_timer1(char choice);
void wait(volatile int multiple, volatile char time_choice);


volatile int temp = 1;
unsigned char counter = 0;
unsigned char send = 0;

int main(void)
{
	
	DDRD = 0b11110011;
	DDRB = 0b11111111;
	//DDRC = 0b00000000;
	PORTB = 0b11111111;
	//PORTD = 0b00000000;
	PRR = 0x00; // clear Power Reduction ADC bit (0) in PRR register
	ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0; // 0x87 // 0b10000111 // Set ADC Enable bit (7) in ADCSRA register, and set ADC prescaler to 128 (bits 2?0 of ADCSRA = ADPS2?ADPS0 = 111)
	ADMUX = 0<<REFS1 | 1<<REFS0 | 1<<ADLAR; //0x60; // 0b01100000 // select Analog Reference voltage to be AVcc (bits7?6 of ADMUX = 01), //left justification (bit 5 of ADMUX = ADLAR = 1);
	//and select channel 0 (bits 3?0 of ADMUX = MUX3?MUX0 = 000)
	EICRA = 0b00001010;
	EIMSK = 1<<INT0;
	
	TCCR0A = 1<<COM0A1 | 0<<COM0A0 | 1<<WGM01 | 1<<WGM00;      // Set non-inverting mode on OC0A pin (COMA1:0 = 10; Fast PWM (WGM1:0 bits = bits 1:0 = 11) (Note that we are not affecting OC0B because COMB0:1 bits stay at default = 00)
	TCCR0B = 0<<CS02 | 1<<CS01 | 1<<CS00; // Set base PWM frequency (CS02:0 - bits 2-0 = 011 for prescaler of 64, for approximately 1kHz base frequency)
	// PWM is now running on selected pin at selected base frequency.  Duty cycle is set by loading/changing value in OCR0A register.

	
	sei();
	initialize_usart(); // Initialize the USART with desired parameters
	
	while(1)  {
		transmit_data_usart(send);
		//PORTB = (temp<<PORTB0);
		PORTB = temp;
	}
	return 0;
}

ISR(INT0_vect) {
	//PORTB = temp;
	wait(250,2);
	counter=counter+1;
	if(counter>=2)
	{
		counter=0;	
		send = 0;
	}
	if (counter == 1) 
	{
		send = 220;
		PORTB = 0b00000000;
		wait(250,2);
	}
	else
	{
		PORTB = 0b11111111;
		wait(250,2);
	}
	//transmit_data_usart(counter); // Transmit desired character repeatedly – which is
	temp = PORTB;
	
	EIFR = EIFR | 1<<INTF0;
	
}

// Function to transmit the data
void initialize_usart(void) // function to set up USART
{
	UCSR0B = (1<<TXEN0); // enable serial transmission
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00) ; // Asynchronous mode, 8-bit data; no parity; 1 stop bit
	UBRR0L = 0x67; // 9,600 baud if Fosc = 16MHz
}

void transmit_data_usart(unsigned char x) // Function to transmit a character
{
	while (!(UCSR0A & (1<<UDRE0))); // Poll to make sure transmit buffer is ready,
	// then send data
	UDR0 = x;
}

void wait(volatile int multiple, volatile char time_choice) {
	while (multiple > 0) {
		delay_T_msec_timer1(time_choice);
		multiple--;
	}
}

void delay_T_msec_timer1(volatile char choice) {
	//
	// ***Note that since the Timer1 register is 16 bits, the delays can be much higher than shown here.
	// This subroutine creates a delay of T msec using TIMER1 with prescaler on clock, where, for a 16MHz clock:
	//T = 0.125 msec for prescaler set to 8 and count of 250 (preload counter with 65,535-5)
	//T = 1 msec for prescaler set to 64 and count of 250 (preload counter with 65,535-5)
	//T = 4 msec for prescaler set to 256 and count of 250 (preload counter with 65,535-5)
	//T = 16 msec for prescaler set to 1,024 and count of 250 (preload counter with 65,535-5)
	//Default: T = .0156 msec for no prescaler and count of 250 (preload counter with 65,535-5)

	//Inputs: None
	//Outputs: None

	TCCR1A = 0x00; // clears WGM00 and WGM01 (bits 0 and 1) to ensure Timer/Counter is in normal mode.
	TCNT1 = 0;  // preload load TIMER1 with 5 if counting to 255 (count must reach 65,535-5 = 250)
	// or preload with 0 and count to 250

	switch ( choice ) { // choose prescaler
		case 1:
		TCCR1B = 1<<CS11;//TCCR1B = 0x02; // Start TIMER1, Normal mode, crystal clock, prescaler = 8
		break;
		case 2:
		TCCR1B =  1<<CS11 | 1<<CS10;//TCCR1B = 0x03;  // Start TIMER1, Normal mode, crystal clock, prescaler = 64
		break;
		case 3:
		TCCR1B = 1<<CS12;//TCCR1B = 0x04; // Start TIMER1, Normal mode, crystal clock, prescaler = 256
		break;
		case 4:
		TCCR1B = 1<<CS12 | 1<<CS10;//TCCR1B = 0x05; // Start TIMER1, Normal mode, crystal clock, prescaler = 1024
		break;
		default:
		TCCR1A = 1<<CS10;//TCCR1B = 0x01; Start TIMER1, Normal mode, crystal clock, no prescaler
		break;
	}

	//while ((TIFR1 & (0x1<<TOV1)) == 0); // wait for TOV1 to roll over at 255 (requires preload of 65,535-5 to make count = 250)
	// How does this while loop work?? See notes
	while (TCNT1 < 0xfa); // exits when count = 250 (requires preload of 0 to make count = 250)

	TCCR1B = 0x00; // Stop TIMER1
	//TIFR1 = 0x1<<TOV1;  // Clear TOV1 (note that this is an odd bit in that it
	//is cleared by writing a 1 to it)

} // end delay_T_msec_timer1()