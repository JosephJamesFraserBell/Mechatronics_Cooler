/*
 * Wheels_Wireless_Reciever.cpp
 *
 * Created: 7/29/2019 10:01:25 PM
 * Author : gabriel101
 */ 

#include <avr/io.h>

void initialize_usart(); // function to set up USART
void wait(volatile int multiple, volatile char time_choice);
void delay_T_msec_timer1(volatile char choice);
void Check_Button_Value();
void Check_Direction();

unsigned char Value;

int main(void)
{
	DDRC = 0b00000000;
    DDRD = 0b11110011;
	PORTD = 0b11110011;
	DDRB = 0b11111111;
	PORTB = 0b00000011;
	PORTC = 0b00000000;
	// MOTOR STUFF
	OCR0A = 200;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
	OCR0B = 200;
	TCCR0A = 1<<COM0A1 | 0<<COM0A0  | 1<<COM0B1 | 0<<COM0B0 | 1<<WGM01 | 1<<WGM00;      // Set non-inverting mode on OC0A pin (COMA1:0 = 10; Fast PWM (WGM1:0 bits = bits 1:0 = 11) (Note that we are not affecting OC0B because COMB0:1 bits stay at default = 00)
	TCCR0B = 0<<CS02 | 1<<CS01 | 1<<CS00; // Set base PWM frequency (CS02:0 - bits 2-0 = 011 for prescaler of 64, for approximately 1kHz base frequency)
	// PWM is now running on selected pin at selected base frequency.  Duty cycle is set by loading/changing value in OCR0A register.

	initialize_usart(); // Initialize the USART with desired parameters
    while (1) 
    {
		
		
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
		
		while(PINC | 0b00100000){
			OCR0A = 0;
			OCR0B = 0;
		}
		
		Check_Button_Value();
    }
}

void initialize_usart(void) // function to set up USART
{
	UCSR0B = (1<<RXEN0); // enable serial transmission
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // Asynchronous mode, 8-bit data; no parity; 1 stop bit
	UBRR0L = 0x67; // 9,600 baud if Fosc = 16MHz
}

void wait(volatile int multiple, volatile char time_choice) {
	while (multiple > 0) {
		delay_T_msec_timer1(time_choice);
		if (Value > 32)
			multiple = 0;
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

void Check_Button_Value() {
	while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
	Value = UDR0; // Read the data
	
	if (Value <= 10){
		//OCR0A = 200;
		//OCR0B = 200;
		
		Check_Direction();
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	while (Value > 11 && Value <= 20) {
		// READ THE VALUES FROM THE WIRELESS TRANSMITTER
		OCR0A = 0;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
		OCR0B = 0;
		
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
		
	}
	while (Value > 21 && Value <= 30) {
		OCR0A = 0;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
		OCR0B = 0;
		
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	while (Value > 31 && Value <= 40) {
		OCR0A = 0;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
		OCR0B = 0;
		
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	
}

void Check_Direction() {
	while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
	Value = UDR0; // Read the data
	//while(PINC | 0b00100000){
	//	OCR0A = 0;
	//	OCR0B = 0;
	}
	if( Value < 90 ){
		
		OCR0A = 200;
		OCR0B = 0;
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	else if( Value > 90 && Value <= 170 ){
		
		OCR0A = 200;
		OCR0B = 200;
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	else if( Value > 170  ){
		
		OCR0A = 0;
		OCR0B = 200;
		while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
		Value = UDR0; // Read the data
	}
	/*
		if( Value > 41 && Value <= 50 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 51 && Value <= 60 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 61 && Value <= 70 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 71 && Value <= 80 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 81 && Value <= 90 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 91 && Value <= 100 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 101 && Value <= 110 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 111 && Value <= 120 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 121 && Value <= 130 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 131 && Value <= 140 ){
			
			OCR0A = 0;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 141 && Value <= 150 ){
			
			OCR0A = 200;
			OCR0B = 200;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 151 && Value <= 160 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 161 && Value <= 170 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 171 && Value <= 180 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 181 && Value <= 190 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 191 && Value <= 200 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 201 && Value <= 210 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 211 && Value <= 220 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 221 && Value <= 230 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 231 && Value <= 240 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		else if( Value > 241 && Value <= 250 ){
			
			OCR0A = 200;
			OCR0B = 0;
			while (! (UCSR0A & (1<<RXC0))); // Wait until new data arrives
			Value = UDR0; // Read the data
		}
		*/
}

