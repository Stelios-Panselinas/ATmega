#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

int value = 0;
#define TCB0_value (0x1931)
int edgecounterTCB0 = 0;
bool potisma;

void adc_init(){
	//initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; //10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; //The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	//Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.WINHT |= 20; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= 0x04; //Interrupt when RESULT < WINLT || RESULT > WINHT
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
}

void portd_init(){
	PORTD.DIR |= PIN1_bm; //PIN is output
	PORTD.DIR |= PIN0_bm; //PIN is output
	PORTD.DIR |= PIN2_bm; //PIN is output
}

void timer_init(){          
	/* Load the Compare or Capture register with the timeout value*/
    TCB0.CCMP = value;
    /* Enable TCB and set CLK_PER divider to 1 (No Prescaling) */
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
    /* Configure TCB in Periodic Timeout mode */
    TCB0.CTRLB = TCB_CNTMODE_TIMEOUT_gc;
    /* Enable Capture or Timeout interrupt */
    TCB0.INTCTRL = TCB_CAPT_bm;
}

void TCB0_init () //PWM{     	// Load the Compare or Capture register with the timeout value	TCB0.CCMP = TCB0_value;   // Enable TCB and set CLK_PER divider to 1 (No Pre scaling)	TCB0.CTRLA = TCB_ENABLE_bm;
	// Enable Pin Output and configure TCB  in 8-bit PWM mode
	TCB0.CTRLB |= TCB_CCMPEN_bm | TCB_CNTMODE_PWM8_gc;
	// Enable Capture or Timeout interrupt
	TCB0.INTCTRL = TCB_CAPT_bm;
}

int main(){
	adc_init();
	portd_init();
	
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	sei();
	while(1){
		;
	}
}
ISR(PORTF_PORT_vect){
	cli();
	asm("break");
	if((PORTF.INTFLAGS & 0b00100000) == 0b00100000 && potisma){
		 timer_init();
	}else if((PORTF.INTFLAGS & 0b01000000) == 0b01000000 && !potisma){
		 TCB0_init (); 
	}else{
		PORTD.OUTCLR = PIN2_bm | PIN1_bm | PIN0_bm;
		_delay_ms(3);
		PORTD.OUT = PIN2_bm | PIN1_bm | PIN0_bm;
		_delay_ms(3);
	}
	int y = PORTF.INTFLAGS; //Procedure to
	PORTF.INTFLAGS=y; //clear the interrupt flag
	sei();
}

ISR(ADC0_WCOMP_vect){
	cli();
	asm("break");
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	value = ADC0.WINLT - ADC0.RES;
	if(ADC0.RES < ADC0.WINLT){
		potisma = 1;
		PORTD.OUTCLR= PIN0_bm; //LED is on
		ADC0.INTCTRL &= ~ADC_WCMP_bm; //Disable Interrupts for WCM
	}
	else if(ADC0.RES > ADC0.WINHT){
		potisma = 0;
		PORTD.OUTCLR= PIN1_bm; //LED is on
		ADC0.INTCTRL &= ~ADC_WCMP_bm; //Disable Interrupts for WCM
	}
	sei();
}ISR(TCA0_CMP0_vect){
	asm("break");
	cli();
	TCA0.SINGLE.CTRLA = 0; //Disable
	PORTD.OUT= PIN1_bm; //LED is on
	int intflags = TCA0.SINGLE.INTFLAGS; //Procedure to
	TCA0.SINGLE.INTFLAGS=intflags; //clear interrupt flag
	ADC0.INTCTRL |= ADC_WCMP_bm; //Disable Interrupts for WCM
	sei();
}


ISR(TCB0_INT_vect)
{
	asm("break");
	cli();
	// Clear the interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	if(edgecounterTCB0 % 4 == 0){
		// Close the LED 2
		PORTD.OUT |= PIN2_bm;
		PORTD.OUT |= PIN1_bm;
	}else if(edgecounterTCB0 % 2 == 1){
		// Close LED 1
		PORTD.OUT |= PIN2_bm;
	}else if(edgecounterTCB0 % 2 == 0){
		// Open LED 1
		PORTD.OUTCLR |= PIN2_bm;
	}
	edgecounterTCB0 = edgecounterTCB0 + 1;
	ADC0.INTCTRL |= ADC_WCMP_bm; //Disable Interrupts for WCM
	TCB0.CTRLA = 0;
	sei();
}

	