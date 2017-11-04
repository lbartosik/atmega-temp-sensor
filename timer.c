#include <avr/io.h>    
#include "common.h" 
#include "timer.h"

void start_timer0(void)
{
	// disable timer 0
	TCCR0 = 0;	
	// zero timer 0 counter register
	TCNT0 = 0;
	// start timer 0, use clk i/o without prescaler
	TCCR0 = CLK_IO_NO_PRESC;
}

void delay_ticks(u16 ticks)
{
	//u8 tcnt0 = TCNT0 + ticks;
	
	//while (TCNT0 < tcnt0);
}