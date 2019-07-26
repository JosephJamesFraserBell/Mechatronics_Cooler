/*
 * Sonar2.cpp
 *
 * Created: 7/22/2019 5:04:04 PM
 * Author : gabriel101
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Functions
void delay_T_msec_timer1(char choice);
void wait(volatile int multiple, volatile char time_choice);
void sendPulse();

int main(void)
{
    DDRC = 0b00000001;
	DDRB = 0b00000001;
	PORTB = 0b00000000;
    PORTC = 0b00000001;
	EICRA = 0b00001010;
	EIMSK = 1<<INT1;
	sei();
	
    while (1) 
    {
		sendPulse();
		/*if (PINC & 0b00000011) {
			PORTC = 0b00000000;
			wait(500,2);
			PORTC = 0b00000001;
			wait(500,2);
		}*/
    }
}

ISR(INT1_vect) {
	
	PORTC = 0b00000000;
	wait(500,2);
	PORTC = 0b00000001;
	wait(500,2);
	EIFR = EIFR | 1<<INTF1;
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
	//T = 0.125 msec for prescaler set to 8 and count of 250 (preload counter with 65,535?5)
	//T = 1 msec for prescaler set to 64 and count of 250 (preload counter with 65,535?5)
	//T = 4 msec for prescaler set to 256 and count of 250 (preload counter with 65,535?5)
	//T = 16 msec for prescaler set to 1,024 and count of 250 (preload counter with 65,535?5)
	//Default: T = .0156 msec for no prescaler and count of 250 (preload counter with 65,535?5)
	TCCR1A = 0x00; // clears WGM00 and WGM01 (bits 0 and 1) to ensure Timer/Counter is in normal mode.
	TCNT1 = 5; //Preload Timer1 register with 0 and count to 250
	// ** Note that TCNT1 only addresses the Low byte of Timer1 register. To access the full register, must access low and high bytes (TCNT1L and TCNT1H).
	switch ( choice ) { // choose prescaler
		case 1:
		TCCR1B = 1<<CS11;//TCCR1B = 0x02; // Start TIMER1, Normal mode, crystal clock, prescaler = 8
		break;
		case 2:
		TCCR1B = 1<<CS11 | 1<<CS10;//TCCR1B = 0x03; // Start TIMER1, Normal mode, crystal clock, prescaler = 64
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
	while (TCNT1 < 255){
	} // exits when count = 250
	
	TCCR1B = 0x00; // Stop TIMER1
	TIFR1 = 0x1<<TOV1; // Clear TOV1 (note that this is an odd bit in that it
	//is cleared by writing a 1 to it)
} // end delay_T_msec_timer1()

void sendPulse(){
	PORTB = 0b00000001;
	_delay_us(15);
	PORTB = 0b00000000;
}
