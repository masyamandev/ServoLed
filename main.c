#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define DDR_MASK 0b00011110
#define PORT_MASK 0b00000000

#define COUNTER_SWITCH (F_CPU * 13 / 10 / 1000 / 256) // central point: 13ms
#define COUNTER_HYSTERESIS (COUNTER_SWITCH / 20) // 10% in total

volatile uint16_t timeCounter;

volatile uint8_t ledsOn;

int main(void)
{
	cli();

	DDRB = DDR_MASK;
	PORTB = PORT_MASK;
	
	timeCounter = 0;
	ledsOn = 0;

	// init timer
	// prescale timer to 1/1st the clock rate
	TCCR0B |= (1<<CS00);
	// enable timer overflow interrupt
	TIMSK0 |= (1<<TOIE0);
	
	// init pc0 interrupt
	// Any logical change on INT0 generates an interrupt request.
	MCUCR |= (0 << ISC01) | (1 << ISC00);
	// External Interrupt Request 0 Enable
	GIMSK |= (1 << PCIE);
	// Set pin mask
	PCMSK = (1 << PCINT0);
	
	sei();
	
	while(1)
	{
	}
}

// Timer interrupt
ISR(TIM0_OVF_vect)
{
	timeCounter ++;
	// Control LEDs
	PORTB = (ledsOn) ? (DDR_MASK | PORT_MASK) : PORT_MASK;
	//PORTB = DDR_MASK & ((ledsOn) ? 0x01010100 : 0x10101010); // Doesn't work :(
	//PORTB = (ledsOn) ? (1 << PB4) : (1 << PB3);
}

// PWM change interrupt
ISR(PCINT0_vect)
{ 
	if (PINB & (1 << PB0)) {
		// Start of PWM impulse
		timeCounter = 0;
	} else {
		// End of PWM impulse
		if (timeCounter > COUNTER_SWITCH + COUNTER_HYSTERESIS) {
			ledsOn = 1;
		}
		if (timeCounter < COUNTER_SWITCH - COUNTER_HYSTERESIS) {
			ledsOn = 0;
		}
	}
}
