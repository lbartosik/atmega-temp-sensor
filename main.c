#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h> 
#include <util/delay.h>   
#include <avr/interrupt.h>
#include "common.h" 
#include "usart_drv.h" 
#include "timer_drv.h" 
#include "temp_sensor.h" 
#include "lcd_screen_drv.h" 
#include "nokia5110.h"         
    
int main(void) 
{ 
	int i;
	
	//DDRC |= (1 << PIN_C5);                
    //PORTC &= ~(1 << PIN_C5); 

	LED_DDR |= (1 << LED_DATA_PIN);                
    LED_PORT &= ~(1 << LED_DATA_PIN); 	
				
	// initialize lcd
	nokia_lcd_init();
	
 	/*
	// initialize lcd
	lcd_screen_init();
	
	write_byte(CONTROL_CMD, 0x21);
	write_byte(CONTROL_CMD, 0x90);
 	 
	write_byte(CONTROL_CMD, 0x20);
	write_byte(CONTROL_CMD, 0x09);
	write_byte(CONTROL_CMD, 0x08);
	write_byte(CONTROL_CMD, 0x0C);
	for (i = 0; i < 20; i++)
		write_byte(DATA_CMD, 0xDD);
	*/
			 
    // initialize usart   	
	//usart_init();	
		
	// configure temperature sensor and initialize first temperature conversions
	read_scratch_pad();
	set_temp_resolution(RES_11_BITS);
	init_temp_conversion();		
		
	// enable interrupts in status register (GIE bit)
	SREG |= (1 << SREG_I);
	
	// we need to wait of at least one second before entering sleep mode because of 32,768 clock oscillator
	for (i = 0; i < 20; i++)
		_delay_ms(50);
	
	// display temperature
	update_temperature_on_lcd(FALSE);
	
	// setup timer2 async operation
	setup_timer2_asynch();
				
	while(TRUE)
    {			
		//while ((UCSR0B & (1 << UDRIE0)) > 0);				
		//_delay_ms(10);
	
		enter_sleep_mode();	         	
    } 
	
    return 0; 
}

void update_temperature_on_lcd(u8 force_update)
{
	u8 old_temp_value_int, old_temp_value_frac;
	u8 new_temp_value_int, new_temp_value_frac;
	char temp_str[10];
	u8 abs_val;
	
	// compare if temperature has changes because there might be no need to update lcd display
	old_temp_value_int = get_temp_value_int();
	old_temp_value_frac = get_temp_value_frac();
	
    // read scratchpad
	read_scratch_pad();
	
	new_temp_value_int = get_temp_value_int();
	new_temp_value_frac = get_temp_value_frac();

	if (new_temp_value_int != old_temp_value_int ||
		new_temp_value_frac != old_temp_value_frac ||
		force_update)
	{	
		abs_val = abs(new_temp_value_int);
		if (new_temp_value_int >= 0)
			temp_str[0] = '+';
		else
			temp_str[0] = '-';
		
		// convert temp to string						
		snprintf(temp_str+1, 10, "%d.%d", new_temp_value_int, new_temp_value_frac); 
					
		// update lcd screen with new temperature
		nokia_lcd_clear(); 
		nokia_lcd_set_cursor(9, 0);
		nokia_lcd_write_string("Temperatura", 1);
		nokia_lcd_set_cursor(0, 16);	
		nokia_lcd_write_string(temp_str, 3);			
		nokia_lcd_render();				
		
	}
	else
	{
		temp_sensor_stats_t* stats = get_temp_sensor_stats();
		
		// increase statistic
		stats->	lcd_update_not_required++;
	}
		
	// initalize temperature conversion
	init_temp_conversion();
	
	//static u8 on = 1;
	
	//nokia_lcd_power(on);
		
	//on ^= 0x1;
}


