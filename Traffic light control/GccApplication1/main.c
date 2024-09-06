#include <avr/io.h>
#include <avr/interrupt.h>
#define value 20
int x=0; //logic flag
int main() {
	PORTD.DIR |= 0b00000010; //PIN is output
	PORTD.OUTCLR= 0b00000010;//LED is on
	//(σελ 219, 224, 205) 16-bit counter high and low
	TCA0.SINGLE.CNT = 0; //clear counter
	TCA0.SINGLE.CTRLB = 0; //Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc σελ 207)
	TCA0.SINGLE.CMP0 = value;//When CMP0 reaches this value -> interrupt
	//CLOCK_FREQUENCY/1024
	TCA0.SINGLE.CTRLA = 0x7<<1; //TCA_SINGLE_CLKSEL_DIV1024_gc σελ 224
	TCA0.SINGLE.CTRLA |= 1; //Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; //Interrupt Enable (=0x10)
	sei(); //begin accepting interrupt signals
	while (x==0) {
		; //similar to a nop function
	}
	PORTD.OUT |= PIN1_bm; //LED is off
	cli();
}
ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0; //Disable
	int intflags = TCA0.SINGLE.INTFLAGS; //Procedure to
	TCA0.SINGLE.INTFLAGS=intflags; //clear interrupt flag
	x=1; //change flag to get out of the loop
}