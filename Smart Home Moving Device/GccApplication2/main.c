#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

//periods initialize
#define T1 20
#define T2 40
#define T3 200

int	moving_forward = PIN1_bm;
int turning_left = PIN2_bm;
int turning_right = PIN0_bm;
int stop = PIN2_bm|PIN1_bm|PIN0_bm;
int left_turns = 0;

//flags initialize
bool turn_180 = 0;
bool moving_bakwards = 0;
bool period_of_1ms_have_passed = 0;
bool period_of_2ms_have_passed = 0;
bool have_finished = 0;

void period_of_1ms(){
	period_of_1ms_have_passed = 0;
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc уел 207)
	TCA0.SINGLE.CMP0 = T1;//When CMP0 reaches this value -> interrupt
	//CLOCK_FREQUENCY/1024
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc уел 224
	TCA0.SINGLE.CTRLA |= 1; //Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; //Interrupt Enable (=0x10)
	sei();
}

void period_of_2ms(){
	period_of_2ms_have_passed = 0;
	TCB0.CCMP = T2;
	TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB0.CNT = 0;
	TCB0.CTRLB = 0;
	TCB0.CTRLB = TCB_CCMPEN_bm;
	TCB0.INTCTRL = TCB_CAPT_bm;
	sei();
}

void adc_setup(){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;//10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; //Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;//The bit
	ADC0.DBGCTRL |= ADC_DBGRUN_bm; //Enable Debug Mode
	//Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINLT
	sei();
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
}

int main(){
	//LED's direction initialize
	PORTD.DIR |= PIN0_bm; //PIN is output
	PORTD.DIR |= PIN1_bm; //PIN is output
	PORTD.DIR |= PIN2_bm; //PIN is output
	
	adc_setup();
	PORTD.OUTCLR= moving_forward;
	
	while(left_turns<=5){
		period_of_1ms();
		//Searching for front wall
		if(period_of_1ms_have_passed){
			ADC0.WINLT |= 10; //Set threshold
			ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
			ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT < WINLT
			ADC0.CTRLA |= ADC_FREERUN_bm; //Free-Running mode enabled
			ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
			sei();
		}
		period_of_2ms();
		//Searching for right wall
		if(period_of_2ms_have_passed){
			ADC0.WINHT |= 10; //Set threshold
			ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
			ADC0.CTRLE |= ADC_WINCM0_bm; //Interrupt when RESULT > WINLT
			ADC0.CTRLA &= ~(1 << 1); //Disable Free-Running mode
			ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
			sei();
		}
		PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
		sei(); //enable interrupts
		while(turn_180){
			TCB1.CCMP = T3;
			TCB1.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
			TCB1.CNT = 0;
			TCB1.CTRLB = 0;
			TCB1.CTRLB = TCB_CCMPEN_bm;
			TCB1.INTCTRL = TCB_CAPT_bm;
			sei();
		}
	}
}//sensors inputISR(ADC0_WCOMP_vect){
	if (!moving_bakwards && period_of_2ms_have_passed){
		PORTD.OUT |= stop; // all led off
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;
		PORTD.OUTCLR= turning_right; //LED0 is on
		PORTD.OUT |= stop; //all leds off
		PORTD.OUTCLR= moving_forward; //LED1 is on
		left_turns--;
	}else if(!moving_bakwards && period_of_1ms_have_passed){
		PORTD.OUT |= stop; // all led off
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;
		PORTD.OUTCLR= turning_left; //LED2 is on
		PORTD.OUT |= stop; //all leds off
		PORTD.OUTCLR= moving_forward; //LED1 is on
		left_turns++;
	}else if(moving_bakwards && period_of_1ms_have_passed){
		PORTD.OUT |= stop; // all led off
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;
		PORTD.OUTCLR= turning_right; //LED0 is on
		PORTD.OUT |= stop; //all leds off
		PORTD.OUTCLR= moving_forward; //LED1 is on
		left_turns--;
	}else if(moving_bakwards && period_of_2ms_have_passed){
		PORTD.OUT |= stop; // all led off
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;
		PORTD.OUTCLR= turning_left; //LED0 is on
		PORTD.OUT |= stop; //all leds off
		PORTD.OUTCLR= moving_forward; //LED1 is on
		left_turns++;
	}
	cli();
}//right sensor after_2msISR(TCB0_INT_vect){
	period_of_2ms_have_passed = 1;
	TCB0.INTFLAGS |= TCB_CAPT_bm; // Clear interrupt flag
	
	cli();
}


//forward sensor after_1ms
ISR(TCA0_CMP0_vect){
	period_of_1ms_have_passed = 1;
	TCB1.INTFLAGS |= TCB_CAPT_bm; // Clear interrupt flag
	
	cli();
}

//moving backwards (button)
ISR(PORTF_PORT_vect){
	PORTD.OUT |= stop; // all LED off
	int y = PORTF.INTFLAGS; 
	PORTF.INTFLAGS=y; //clear the interrupt flag
	PORTD.OUTCLR = stop; // all LED on
	turn_180 = 1;
	
	cli();
}

//180 turn duration
ISR(TCB1_INT_vect){
	TCB1.INTFLAGS |= TCB_CAPT_bm; // Clear interrupt flag
	turn_180 = 0;
	ADC0.CTRLA &= ~(1 << 1); //Disable Free-Running mode
	ADC0.COMMAND |= ADC_STCONV_bm; //Start Conversion
	moving_bakwards = 1;
	
	cli();
}