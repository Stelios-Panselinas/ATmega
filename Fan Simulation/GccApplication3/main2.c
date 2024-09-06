#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define TCB1_value (0x203F)
#define TCB0_value_for_1ms (0x100F)
#define TCB0_value_for_0.5ms (0x080E)

int fan_ON = 0; //ìåôáâëçôÞ ìå ôçí ïðïßá åëÝã÷ïõìå áí ìå ôï ðÜôçìá ôïõ êïõìðéïý ï áíåìéóôÞñáò ðñÝðåé íá áíïßîåé Þ íá êëåßóåé 
int edgecounterTCB0 = 0; //ìåôáâëçôÞ ìå ôçí ïðïßá åëÝã÷ïõìå áí ðñÝðåé íá êëåßóïõìå Þ íá áíïßîïõìå ôá LED 
int edgecounterTCB1 = 0; ////ìåôáâëçôÞ ìå ôçí ïðïßá åëÝã÷ïõìå áí ðñÝðåé íá êëåßóïõìå Þ íá áíïßîïõìå ôá LED 
int enable_but_press_times = 1;

//void CLOCK_init (void);
//void PORT_init (void);
//void TCB1_init (void);

//Initialize clock
void CLOCK_init (){		
	/* Enable writing to protected register */
	CPU_CCP = CCP_IOREG_gc;
	/* Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K) */
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
	
	/* Wait for system oscillator changing to finish */
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
	
	if(period_0f_1ms){
		// Load the Compare or Capture register with the timeout value for period of 1 ms
		TCB0.CCMP = TCB0_value_for_1ms;
	}else{
		//Load the Compare or Capture register with the timeout value for period of 1 ms
		TCB0.CCMP = TCB0_value_for_0.5ms;
	}
	// Enable TCB and set CLK_PER divider to 1 (No Pre scaling)
	TCB0.CTRLA = TCB_ENABLE_bm;
	// Enable Pin Output and configure TCB  in 8-bit PWM mode
	TCB0.CTRLB |= TCB_CCMPEN_bm | TCB_CNTMODE_PWM8_gc;
	// Enable Capture or Timeout interrupt
	TCB0.INTCTRL = TCB_CAPT_bm;
}

void TCB1_init (void)//Ãéá ðåñéóôñïöÞ âÜóçò Tb = 2 ms êáé (duty cycle) Db = 60%.
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
	if (enable_but_press_times == 1){
		CLOCK_init();//Initialize clock
		TCB0_init(1);//Ενεργοποίηση λεπίδων με περίοδο 1ms και duty cycle 50%
		TCB1_init();//Ενεργοποίηση βάσης με περίοδο 2ms και duty cycle 60%
		
	}else if(enable_but_press_times == 2){
		TCΒ0_init(0);//Ενεργοποίηση λεπίδων με περίοδο 0.5ms και duty cycle 50%
	}else if(enable_but_press_times == 3){
		TCB1.CTRLA &= ~TCB_ENABLE_bm;
		TCB0.CTRLA &= ~TCB_ENABLE_bm;
		enable_but_press_times = 0;
	}	
	
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
	cli();
}

//Ãéá ôïõò 2 TCB íá ÷ñçóéìïðïéÞóïõìå ìéá ISR ðïõ åíåñãïðïéåßôáé áðü ôïí timer(TCB0) ìå ôçí ìéêñüôåñç ðåñßïäï.
//ÌÝóá óå áõôÞ ôçí ISR êïéôþíôáò ôï TCB1.INTFLAGS êáôáëáâáßíïõìå áí Ý÷åé ðñïêýøåé interrupt êáé áðü ôïí TCB1 êáé ôï áíôéìåôùðßæïõìå áíÜëïãá. 


ISR(TCB0_INT_vect)
{
	// Clear the interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	
	fan_on = !fan_on;
}

ISR(TCB1_INT_vect)
{
	// Clear the interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	
	base_on = !base_on;
}
