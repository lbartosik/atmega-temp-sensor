#include <avr/io.h>  
#include <avr/sleep.h> 
#include <avr/interrupt.h> 
#include "common.h" 
#include "timer_drv.h"

#include "usart_drv.h"

static u8 timer2_cnt = 0x00;
static u16 timer1_meas_start = 0x00;
static tc1_resolution_t timer1_meas_resolution = 0x00;

void start_timer1_meas(tc1_resolution_t res)
{	
	// disable timer 1
	TCCR1B = TC1_CLK_IO_NO_PRESC;	
	// zero timer 1 counter register
	TCNT1 = 0;	
	// cleat timer 1 overflow flag
	TIFR1 = 1 << TOV1;	
	// start timer 1 with selected resolution
	TCCR1B = res;	
	// save timer 1 counter and resolution
	timer1_meas_start = TCNT1;
	timer1_meas_resolution  = res;
}

u16 stop_timer1_meas(void)
{
	u16 timer1_meas_stop, diff;
	
	// get timer 1 counter
	timer1_meas_stop = TCNT1;
	// disable timer 1
	TCCR1B = TC1_CLK_IO_NO_PRESC;	
	// check it timer 1 overflowed
	if (TIFR1 & (1 << TOV1))
		diff = TC1_OVERFLOW;
	else
		diff = timer1_meas_stop - timer1_meas_start;
		
	return diff;
}

u16 get_timer1_meas_resolution(void)
{
	u16 res;

	switch (timer1_meas_resolution)
	{
		case TC1_CLK_IO_NO_PRESC:
			res = 0;
			break;
		case TC1_CLK_IO_1_PRESC:
			res = 1;
			break;			
		case TC1_CLK_IO_8_PRESC:
			res = 8;			
			break;
		case TC1_CLK_IO_64_PRESC:
			res = 64;
			break;
		case TC1_CLK_IO_256_PRESC:
			res = 256;
			break;
		case TC1_CLK_IO_1024_PRESC:
			res = 1024;
			break;
		default:
			res = 0;
			break;		
	}
	
	return res;
}

void setup_timer2_asynch(void)
{
	// disable timer2 interrupts
	TIMSK2 = 0x00;
	
	// switch to async operation for timer2
	ASSR |= (1 << AS2);
	
	// clear registers
	OCR2A = 0x00;
	OCR2B = 0x00;
	TCNT2 = 0x00;
	TCCR2A = 0x00;
	// use 1024 prescaler
	TCCR2B = TC2_CLK_IO_1024_PRESC;

    // wait for the registers to be updated	
	while ((ASSR & (1 << OCR2AUB)) != 0);
 	while ((ASSR & (1 << OCR2BUB)) != 0);
	while ((ASSR & (1 << TCN2UB)) != 0);	
	while ((ASSR & (1 << TCR2AUB)) != 0);
 	while ((ASSR & (1 << TCR2BUB)) != 0);	


	// clear interrrupt flags
	TIFR2 = 0x00;
	
	// enable timer2 overflow interrupt
	TIMSK2 |= (1 << TOIE2);
	
}

void enter_sleep_mode(void)
{	
    // set register value
	OCR2A = 0x00;
	// wait for the registers to be updated	
	while ((ASSR & (1 << OCR2AUB)) != 0);		

	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli(); 
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();        	
}

 void temp_sensor_status_cmd(usart_ctrl_t* usart_ctrl);

// Timer2 Overflow interrupt
ISR(TIMER2_OVF_vect)
{
	// increase counter
	timer2_cnt++;
	
	if (timer2_cnt >= TIMER2_WAKEUP_CNT)
	{
		// clear counter
		timer2_cnt = 0x00;
		
		// update temperature on display	
		update_temperature_on_lcd(FALSE);
		
		// toggle LED
		//PORTC ^= (1 << PIN_C5);	
		
		temp_sensor_status_cmd(usart_get_ctrl_blk());			
	}
	
	// we will go to sleep mode in main loop	
}

