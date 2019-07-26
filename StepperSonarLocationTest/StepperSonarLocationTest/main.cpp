/*
 * StepperSonarLocationTest.cpp
 *
 * Created: 7/26/2019 3:46:14 PM
 * Author : gabriel101
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

void delay_T_msec_timer1(volatile char choice);
void wait(volatile int multiple, volatile char time_choice);
int stepRight(int local_counter);
int stepLeft(int local_counter);
void step();
volatile int counter = 0;

int main(void)
{
    //PC3 - step , PC1 - dir 1 is cw 0 is ccw
	
	DDRC = 0b11111111;
	DDRD = 0b11110111;
	PORTC = 0b00110100;
	PORTD = 0b11110111;
	EICRA = 0b00001000;
	EIMSK = 1<<INT1;
	sei();
	
	int local_counter = 0;
	//PORTC &= ~(1 << PORTC2); //changing direction pin to step right by clearing pc2
    while (1) 
    {
		for (int i = 0; i < 800; i++)
		{
			
			local_counter = stepRight(local_counter);
			counter = local_counter;
		}

		for (int i = 0; i < 800; i++){
			local_counter = stepLeft(local_counter);
			counter = local_counter;
		}
		
    }
	return 0;
}

void wait(volatile int multiple, volatile char time_choice) {
/* This subroutine calls others to create a delay.
   Total delay = multiple*T, where T is in msec and is the delay
created by the called function.
    Inputs: multiple = number of multiples to delay, where multiple is
the number of times an actual delay loop is called.
    Outputs: None
*/
while (multiple > 0) {
    delay_T_msec_timer1(time_choice);
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

ISR(INT1_vect) {
	if (counter <= 264) {
		PORTD = PORTD & 0b01111111;
		wait(1000,2);
	}
	else if (counter <= 528) {
		PORTD = PORTD & 0b10111111;
		wait(1000,2);
	}
	else {
		PORTD = PORTD & 0b11011111;
		wait(1000,2);
	}
	PORTD = PORTD | 0b11110111;
	//EIFR = EIFR | 1<<INTF1;

}

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
