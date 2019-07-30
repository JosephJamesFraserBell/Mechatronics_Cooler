/*
 * StepperSonarTransmitter.cpp
 *
 * Created: 7/29/2019 8:59:13 PM
 * Author : gabriel101
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


// Functions
void delay_T_msec_timer1(char choice);
void wait(volatile int multiple, volatile char time_choice);
void wait_stepper(volatile int multiple, volatile char time_choice);
void initialize_usart(); // function to set up USART
void transmit_data_usart(unsigned char x);
void Check_Button_Value(int local_counter);
void Wheel_Direction(int local_counter);

int stepRight(int local_counter);
int stepLeft(int local_counter);

volatile int counter = 0;
unsigned char SonarValue = 0;
unsigned char ButtonValue = 0;
int dir = 0;

int main(void)
{
	// Initialize Data Registers
	DDRD = 0b11110111;
	DDRC = 0b11111111;
	PORTC = 0b00110000;
	
	EICRA = 0b00001000;
	EIMSK = 1<<INT1;
	sei();
	
	int local_counter = 0;
    initialize_usart(); // Initialize the USART with desired parameters
	while (1) 
    {
		transmit_data_usart(SonarValue);
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		ButtonValue = UDR0; // Read the data
		for (int i = 0; i < 800; i++){
			dir = 0;
			// THIS IS TO GET THE STEPPER MOTOR TO PAN TO THE LEFT
			local_counter = stepRight(local_counter);
			counter = local_counter;
			
			// READ THE VALUES FROM THE WIRELESS TRANSMITTER
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			ButtonValue = UDR0; // Read the data
			Check_Button_Value(local_counter);
		}

		for (int i = 0; i < 800; i++){
			dir = 1;
			// THIS IS TO GET THE STEPPER MOTOR TO PAN TO THE LEFT
			local_counter = stepLeft(local_counter);
			counter = local_counter;
			
			// READ THE VALUES FROM THE WIRELESS TRANSMITTER
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			ButtonValue = UDR0; // Read the data
			Check_Button_Value(local_counter);
		}
    }
}

ISR(INT1_vect) {
	while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
	ButtonValue = UDR0; // Read the data
	
	Check_Button_Value(counter);
	transmit_data_usart(SonarValue);
	wait(1000,2);	
		
	EIFR = EIFR | 1<<INTF1;
}
// Function to transmit the data
void initialize_usart(void) // function to set up USART
{
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); // enable serial transmission
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
		if (ButtonValue > 32)
			multiple = 0;
		multiple--;
	}
}

void wait_stepper(volatile int multiple, volatile char time_choice) {
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

int stepRight(int counter_value){
	int counter_right = counter_value + 1;
	PORTC &= ~(1 << PORTC1); //changing direction to step right by clearing pc1
	wait(5,2);
	PORTC ^= 1 << PORTC3;
	wait(1,1);
	PORTC ^= 1 << PORTC3;
	wait(1,1);
	
	return counter_right;
}

int stepLeft(int counter_value){
	int counter_left = counter_value - 1;
	PORTC |= 1 << PORTC1; //changing directon pin to step left by setting pc1
	wait(5,2);
	PORTC ^= 1 << PORTC3;
	wait(1,1);
	PORTC ^= 1 << PORTC3;
	wait(1,1);
	
	return counter_left;
}

void Check_Button_Value(int local_counter) {
	while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
	ButtonValue = UDR0; // Read the data
	
	if (ButtonValue <= 35){
		SonarValue = 5;
		transmit_data_usart(SonarValue);
		Wheel_Direction(local_counter);
		transmit_data_usart(SonarValue);
	}
	while (ButtonValue > 35 && ButtonValue <= 105) {
		SonarValue = 15;
		transmit_data_usart(SonarValue);
		
		// READ THE VALUES FROM THE WIRELESS TRANSMITTER
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		ButtonValue = UDR0; // Read the data
	}
	if (ButtonValue > 105 && ButtonValue <= 175) {
		SonarValue = 25;
		transmit_data_usart(SonarValue);
		//wait_stepper(5000,2); // this is the wait for the lid to close
		SonarValue = 5;
	}
	while (ButtonValue > 175) {
		SonarValue = 35;
		transmit_data_usart(SonarValue);
		
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		ButtonValue = UDR0; // Read the data
	}
}

void Wheel_Direction(int counter_value) {
	if (dir == 0) {
		if( counter_value <= 456 ){
			SonarValue=155;
		}
		else if( counter_value <= 494 ){
			SonarValue=165;
		}
		else if( counter_value <= 532 ){
			SonarValue=175;
		}
		else if( counter_value <= 570 ){
			SonarValue=185;
		}
		else if( counter_value <= 608 ){
			SonarValue=195;
		}
		else if( counter_value <= 646 ){
			SonarValue=205;
		}
		else if( counter_value <= 684 ){
			SonarValue=215;
		}
		else if( counter_value <= 722 ){
			SonarValue=225;
		}
		else if( counter_value <= 760 ){
			SonarValue=235;
		}
		else if( counter_value <= 800 ){
			SonarValue=245;
		}
	}
	else if (dir == 1) {
		if( counter_value <= 38 ){
			SonarValue=45;
		}
		else if( counter_value <= 76 ){
			SonarValue=55;
		}
		else if( counter_value <= 114 ){
			SonarValue=65;
		}
		else if( counter_value <= 152 ){
			SonarValue=75;
		}
		else if( counter_value <= 190 ){
			SonarValue=85;
		}
		else if( counter_value <= 228 ){
			SonarValue=95;
		}
		else if( counter_value <= 266 ){
			SonarValue=105;
		}
		else if( counter_value <= 304 ){
			SonarValue=115;
		}
		else if( counter_value <= 342 ){
			SonarValue=125;
		}
		else if( counter_value <= 380 ){
			SonarValue=135;
		}
	else
		SonarValue=145;
	}
}



