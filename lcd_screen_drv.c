#include <stdio.h>
#include <avr/io.h> 
#include <util/delay.h>    
#include "common.h" 
#include "lcd_screen_drv.h"

void lcd_screen_init(void)
{
	// set all lines to output high except lcd_screen_led_pin	
    LCD_SCREEN_PORT |= (1 << LCD_SCREEN_SCK_PIN); 
    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_SCK_PIN);   


	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_DATA_PIN); 
    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_DATA_PIN);   


	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_DC_PIN); 
    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_DC_PIN);   


	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_RESET_PIN); 
    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_RESET_PIN);   

	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_SCE_PIN); 
    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_SCE_PIN);     

    LCD_SCREEN_DDR |= (1 << LCD_SCREEN_LED_PIN);                
    //LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_LED_PIN); 
	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_LED_PIN); 

	// reset LCD
	LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_RESET_PIN); 
	_delay_us(100);
	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_RESET_PIN); 
}

void write_byte(lcd_cmd_t lcd_cmd, u8 byte)
{
	u8 i;

	// check if this is control or data byte
	if (lcd_cmd == CONTROL_CMD)
		LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_DC_PIN);   	
	else
		LCD_SCREEN_PORT |= (1 << LCD_SCREEN_DC_PIN);   


	// change SCK and SCE to low
	LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_SCE_PIN);  
	LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_SCK_PIN);  
	
	
	// write byte
	for (i=0; i<8; i++)
	{
		if ((byte & 0x80) > 0)
			// write logic "1"
			LCD_SCREEN_PORT |= (1 << LCD_SCREEN_DATA_PIN);  
		else
			// write logic "0"
			LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_DATA_PIN);  
		
		_delay_us(100);
	
		// change SCK to high
		LCD_SCREEN_PORT |= (1 << LCD_SCREEN_SCK_PIN);  
		// update byte
		byte = byte << 1;
		
		_delay_us(100);
		// change SCK to low
		LCD_SCREEN_PORT &= ~(1 << LCD_SCREEN_SCK_PIN);  
	}
		
		
    _delay_us(100);
	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_SCE_PIN);    
	LCD_SCREEN_PORT |= (1 << LCD_SCREEN_SCK_PIN);    
}



 
