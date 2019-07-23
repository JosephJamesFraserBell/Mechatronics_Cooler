/*
 * motors.cpp
 *
 * Created: 7/19/2019 2:26:40 PM
 * Author : Joseph
 */ 

#include <stdio.h>
#include <avr/io.h>
//#include <avr/interrupt.h>

volatile int i = 0;
volatile int counter = 0;
void delay_T_msec_timer1(volatile char choice);
void wait(volatile int multiple, volatile char time_choice);

int main(void)
{
	DDRD = 0b11111111;  // setting all of D to output
	//PD0 = M2 Reverse - PD1 = M2 Forward - PD2 = M1 Forward - PD3 = M1 Reverse
	PORTD = 0b11110000; // set all bits on PORTD so that all outputs are off and motors are off
	//(active low)
	
	/*
	EICRA = 0b00001010; //setting type of interrupt for INT1 and INT0 to falling
	EIMSK = 0b00000011; //enabling both interrupts
	sei();
	*/
	
	
	OCR0A = 0;       // Load $00 into OCR0 to set initial duty cycle to 0 (motor off)
	OCR0B = 0;
	TCCR0A = 1<<COM0A1 | 0<<COM0A0 | 1<<COM0B1 | 0<<COM0B0 | 1<<WGM01 | 1<<WGM00;      // Set non-inverting mode on OC0A pin (COMA1:0 = 10; Fast PWM (WGM1:0 bits = bits 1:0 = 11) (Note that we are not affecting OC0B because COMB0:1 bits stay at default = 00)
	TCCR0B = 0<<CS02 | 1<<CS01 | 1<<CS00; // Set base PWM frequency (CS02:0 - bits 2-0 = 011 for prescaler of 64, for approximately 1kHz base frequency)
	// PWM is now running on selected pin at selected base frequency.  Duty cycle is set by loading/changing value in OCR0A register.

	PORTD |= 1 << PORTD0;
	//PORTD |= 1 << PORTD3;
	//PORTD &= ~(1 << PORTD5);
	//PORTD &= ~(1 << PORTD6);					 
    
    //OCR0B = 155;
    while(1)
    {
		//OCR0A = 255;
		//OCR0B = 255;
	    
    }
    
    return(0);
}//end of main

/*
int main(void)
{
	DDRD= 0b11111111;// Make OC0A (PD6) and OC0B (PD5) output bits ?? theseare the PWM pins;
	PORTD = 0b10111110;
	OCR0A= 0x00; // Load $00 into OCR0 to set initial duty cycle to0 (motoroff)
	TCCR0A= 1<<COM0A1 | 1<<COM0A0 | 1<<WGM01 | 1<<WGM00; 
	TCCR0B= 0<<CS02 | 1<<CS01 | 1<<CS00; // Set base PWM frequency (CS02:0 ? bits2?0= 011 for prescaler of 64, for approximately 1kHz base frequency)
	// PWM is now running on selected pin at selected base frequency. Duty cycle isset by loading/changing value in OCR0A register.
	while(1)
	{
		OCR0A= 0xFF; // Load
	}
	return(0);
}

ISR(INT1_vect){ //decrease speed
	wait(250,2);
	switch (counter)
	{
	case -2:
		PORTC = 0b11111101;
		OCR0A = 155;
		counter++;
		break;
	case -1:
		PORTC = 0b11111011;
		OCR0A = 0;
		counter++;
		break;
	case 0:
		PORTC = 0b11110111;
		PORTD = 0b11111110;
		OCR0A = 155;
		counter++;
		break;
	case 1:
		PORTC = 0b11101111;
		OCR0A = 255;
		counter++;
		break;
	case 2:
		counter = counter;
		OCR0A = 255;
		break;
	default:
		PORTC = 0b11110101; //turn on all 2 alternating lights to indicate a problem
		wait(250,2);
		PORTC = 0b11111010;
		wait(250,2);
	}
	
	
	
	EIFR = EIFR | 0b00000010; //clearing interrupt flag
	//TCNT0 = 5; //reset preload on counter 
	wait(250,2);
}//end of ISR1

ISR(INT0_vect){ //increase speed
	wait(250,2);
	switch (counter)
	{
		case -2:
			counter = counter;
			OCR0A = 255;
			break;
		case -1:
			PORTC = 0b11111110;
			OCR0A = 255;
			counter--;
			break;
		case 0:
			PORTC = 0b11111101;
			PORTD = 0b11111101;
			OCR0A = 155;
			counter--;
			break;
		case 1:
			PORTC = 0b11111011;
			OCR0A = 0;
			counter--;
			break;
		case 2:
			PORTC = 0b11110111;
			OCR0A = 155;
			counter--;
			break;
		default:
			PORTC = 0b11110101; //turn on all 2 alternating lights to indicate a problem
			wait(250,2);
			PORTC = 0b11111010;
			wait(250,2);	
	}
	EIFR = EIFR | 0b00000001; //clearing interrupt flag
	//TCNT0 = 5; //reset preload on counter
	wait(250,2);
}//end of ISR0
*/

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



