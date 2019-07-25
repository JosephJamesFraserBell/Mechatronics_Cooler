/*
 * lab9_spi.cpp
 *
 * Created: 7/9/2019 1:18:01 PM
 * Author : joseph
 */ 

#include <avr/io.h>

int send_to_MAX7221(unsigned char command, unsigned char data);
void wait(volatile int multiple, volatile char time_choice);
void delay_T_msec_timer1(volatile char choice);

int main(void)
{
	// Set up Master SPI
	SPCR = 1<<SPE | 1<<MSTR | 1<<SPR0;
    DDRB = 1<<PORTB5 | 1<<PORTB3 | 1<<PORTB2; // Set pins SCK, MOSI, and SS as output
	
	send_to_MAX7221(0b00001001,0b11111111);
	send_to_MAX7221(0b00001011,0b00000011);
	send_to_MAX7221(0b00001100,0b00000001);
	send_to_MAX7221(0b00001010,0b11111111);
	while (1) {
	send_to_MAX7221(0b00000001,0b00001000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000001);

	send_to_MAX7221(0b00000001,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000001);
	
	send_to_MAX7221(0b00000001,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000001);
	
	send_to_MAX7221(0b00000001,0b00001000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000000);
	wait(1000,2);
	send_to_MAX7221(0b00001100,0b00000001);
	
	send_to_MAX7221(0b00000001,0b00000101);
    wait(5000,2);
		
	};
}

int send_to_MAX7221(unsigned char command, unsigned char data)
{
	PORTB = PORTB & 0b11111011; // Clear PB2, which is the SS bit, so that
	// transmission can begin
	SPDR = command; // Send command
	while(!(SPSR & (1<<SPIF))); // Wait for transmission to finish
	SPDR = data; // Send data
	while(!(SPSR & (1<<SPIF))); // Wait for transmission to finish
	PORTB = PORTB | 0b00000100; // Return PB2 to 1, which is the SS bit, to end
	// transmission
	return 0;
}

void wait(volatile int multiple, volatile char time_choice) {
	//*** wait ***
	/* This subroutine calls others to create a delay.
		 Total delay = multiple*T, where T is in msec and is the delay created by the called function.
	
		Inputs: multiple = number of multiples to delay, where multiple is the number of times an actual delay loop is called.
		Outputs: None
	*/
	
	while (multiple > 0) {
		delay_T_msec_timer1(time_choice); // we are choosing case 2, which is a 1 msec delay
		multiple--;
	}
} // end wait()

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