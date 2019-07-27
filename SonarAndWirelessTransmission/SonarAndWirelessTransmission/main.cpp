/*
 * SonarAndWirelessTransmission.cpp
 *
 * Created: 7/26/2019 5:41:53 PM
 * Author : gabriel101
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Functions
void initialize_usart(); // function to set up USART
void transmit_data_usart(unsigned char x);
void delay_T_msec_timer1(char choice);
void wait(volatile int multiple, volatile char time_choice);
void sendPulse();

//There will be 4 values that will be transmitted
// 0 will be for the cooler's normal function (Go)
// 70 will be for the cooler to open the lid and stop everything
// 140 value will be for the cooler to close the lid and go
// 210 value will be the kill switch

unsigned char ButtonValue = 0;
unsigned char counter = 0; // This will be used in INT0
unsigned char counter2 = 0; // This will be used in INT1

int main(void)
{
	DDRC = 0b00000001;
	DDRD = 0b00000000;
	PORTC = 0b00000001;
	EICRA = 0b00001010;
	EIMSK = 1<<INT0 | 1<<INT1;
	sei();

    while (1) 
    {
		sendPulse();
		transmit_data_usart(ButtonValue);
    }
}

// This interrupt is for the go-stop switch
ISR(INT0_vect) {
	counter=counter+1;
	if(counter>=2)
	{
		counter=0;
		ButtonValue = 0;
	}
	if (counter == 1)
	{
		ButtonValue = 190;
	}

	EIFR = EIFR | 1<<INTF0;
}

// This interrupt to open/close lid. It will also prevent the cooler from moving if the lid is open
ISR(INT1_vect) {
	
	counter2=counter2+1;
	if(counter2>=2)
	{
		counter=0;
		ButtonValue = 70;
	}
	if (counter2 == 1)
	{
		ButtonValue = 140;
	}

	EIFR = EIFR | 1<<INTF1;
}

void sendPulse() {
	PORTC = 0b00000001;
	_delay_us(15);
	PORTC = 0b00000000;
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

