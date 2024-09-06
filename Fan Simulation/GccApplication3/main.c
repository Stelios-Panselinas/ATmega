#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define TCB1_value (0x273F)
#define TCB0_value_for_1ms (0x101F)
#define TCB0_value_for_half_ms (0x080F)

int fan_ON = 0; //μεταβλητή με την οποία ελέγχουμε αν με το πάτημα του κουμπιού ο ανεμιστήρας πρέπει να ανοίξει ή να κλείσει
int enable_but_press_times = 0;
int edgecounterTCB0;
int edgecounterTCB1;

//void CLOCK_init (void);
//void PORT_init (void);
//void TCB1_init (void);

//Initialize clock
void CLOCK_init (){
	// Enable writing to protected register
	CPU_CCP = CCP_IOREG_gc;
	// Disable CLK_PER Prescaler
	CLKCTRL.MCLKCTRLB = 0 << CLKCTRL_PEN_bp;
	// Enable writing to protected register
	CPU_CCP = CCP_IOREG_gc;
	// Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K)
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
	// Wait for system oscillator changing to finish
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm)
	{
		;
	}
}

// Initialize ports
void PORT_init (void)
{
	PORTD.DIR = PIN0_bm|PIN1_bm;
	PORTD.OUT |= PIN0_bm|PIN1_bm;
}


void TCB0_init (bool period_of_1ms) //Παλμός για τις λεπίδες 1ms και 0,5ms
{
	if(period_of_1ms){
		// Load the Compare or Capture register with the timeout value for period of 1 ms
		TCB0.CCMP = TCB0_value_for_1ms;
		}else{
		//Load the Compare or Capture register with the timeout value for period of 0.5 ms
		TCB0.CCMP = TCB0_value_for_half_ms;
	}
	// Enable TCB and set CLK_PER divider to 1 (No Pre scaling)
	TCB0.CTRLA = TCB_ENABLE_bm;
	// Enable Pin Output and configure TCB  in 8-bit PWM mode
	TCB0.CTRLB |= TCB_CCMPEN_bm | TCB_CNTMODE_PWM8_gc;
	// Enable Capture or Timeout interrupt
	TCB0.INTCTRL = TCB_CAPT_bm;
}

void TCB1_init (void) //Παλμός για βαση Tb = 2ms και Db = 60%.
{
	// Load the Compare or Capture register with the timeout value
	TCB1.CCMP = TCB1_value;
	// Enable TCB and set CLK_PER divider to 1 (No Pre scaling)
	TCB1.CTRLA = TCB_ENABLE_bm;
	// Enable Pin Output and configure TCB  in 8-bit PWM mode
	TCB1.CTRLB |= TCB_CCMPEN_bm | TCB_CNTMODE_PWM8_gc;
	// Enable Capture or Timeout interrupt
	TCB1.INTCTRL = TCB_CAPT_bm;
}

int main(void)
{
	PORT_init();
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	sei();
	while (1)
	{
		sei();
	}
	cli();
}


ISR(PORTF_PORT_vect){//Interrupt for starting/turning off the fan.
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	enable_but_press_times ++;
	
	if (enable_but_press_times == 1 && fan_ON == 0){
		CLOCK_init();//Initialize clock
		TCB0_init(1);//Ενεργοποίηση λεπίδων με περίοδο 1ms και duty cycle 50%
		TCB1_init();//Ενεργοποίηση βάσης με περίοδο 2ms και duty cycle 60%
		fan_ON = 1;
	}else if(enable_but_press_times == 2 && fan_ON == 1){
		TCB0_init(0);//Ενεργοποίηση λεπίδων με περίοδο 0.5ms και duty cycle 50%
	}else if(enable_but_press_times == 3){
		TCB1.CTRLA &= ~TCB_ENABLE_bm;
		TCB0.CTRLA &= ~TCB_ENABLE_bm;
		enable_but_press_times = 0;
		fan_ON=0;
	}
	cli();
}


/*ISR(TCB0_INT_vect)
{
	int y = TCB0.INTFLAGS;
	int x = TCB1.INTFLAGS;

	if (TCB0_INT_vect && TCB1.INTFLAGS){
		fan_ON=!fan_ON;
		TCB0.INTFLAGS=y;
		TCB1.INTFLAGS=x;
		if (fan_ON){
			PORTD.OUTCLR |=PIN1_bm;
			}else{
			PORTD.OUT |=PIN1_bm;
		}
	}

}*/


/*ISR (TCB1_INT_vect )
{
	int y = TCB0.INTFLAGS;
	int x = TCB1.INTFLAGS;
	
	if (TCB1_INT_vect && TCB0.INTFLAGS){
		TCB0.INTFLAGS=y;
		TCB1.INTFLAGS=x;
		return;
	}else{
		base_ON = !base_ON;
		if (base_ON){
			PORTD.OUTCLR |=PIN0_bm;
			}else{
			PORTD.OUT |=PIN0_bm;
		}
	}
	
}*/



/*ISR(PORTF_PORT_vect){//Interrupt for starting/turning off the fan.
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	
	enable_but_press_times ++;
	
	if(fan_ON == 0 && enable_but_press_times == 1){
		fan_ON = 1;
		TCB0_init(1);
		TCB1_init();
	}
	else if(enable_but_press_times == 2 && fan_ON == 1){
		fan_ON = 1;
		TCB0_init(0);
		//TCB1_init();
	}else{
		TCB0.CTRLA = 0;
		TCB1.CTRLA = 0;
		fan_ON = 0;
	}
}*/

//Για τους 2 TCB να χρησιμοποιήσουμε μια ISR που ενεργοποιείται από τον timer(TCB0) με την μικρότερη περίοδο.
//Μέσα σε αυτή την ISR κοιτώντας το TCB1.INTFLAGS καταλαβαίνουμε αν έχει προκύψει interrupt και από τον TCB1 και το αντιμετωπίζουμε ανάλογα.


ISR(TCB0_INT_vect)
{
	// Clear the interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	
	if(edgecounterTCB0 == 0){
		// Open the LED 0
		PORTD.OUTCLR = PIN0_bm;
	}
	else{
		// Close LED 0
		PORTD.OUT |= PIN0_bm;
	}

	// TCB1 on a rising edge
	if(TCB1.INTFLAGS == 1)
	{
		
		// Clear the interrupt flag
		TCB1.INTFLAGS = TCB_CAPT_bm;

		if(edgecounterTCB1 % 2 == 0)
		{
			// Open the LED 1
			PORTD.OUTCLR = PIN1_bm;
		}

		else if(edgecounterTCB1 % 2 == 1)
		{
			// Close LED 1
			PORTD.OUT |= PIN1_bm;

		}
		edgecounterTCB1 = (edgecounterTCB1 + 1) % 2;
	}

	edgecounterTCB0 = (edgecounterTCB0 + 1) % 2;
}









