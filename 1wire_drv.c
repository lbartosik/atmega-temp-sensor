#include <stdio.h>
#include <avr/io.h> 
#include <util/delay.h>    
#include "common.h" 
#include "temp_sensor.h" 
#include "1wire_drv.h"

u8 one_wire_init_seq(void)
{    
	u8 is_present = FALSE;
	u8 i;

	// change port to input, no pull up
	DQ_DDR &= ~(1 << DQ_DATA_PIN);
	DQ_PORT &= ~(1 << DQ_DATA_PIN);	
			
	// --- initalization sequence ---
	// change port to output, low 
	DQ_DDR |= (1 << DQ_DATA_PIN);
		
	// wait	
	_delay_us(520);
	
	// change port to input
	DQ_DDR &= ~(1 << DQ_DATA_PIN);
	
	// wait
	_delay_us(40);
		
	for (i = 0; i < DQ_INIT_LOOPS; i++)
	{	
		if ((DQ_PIN & (1 << DQ_DATA_PIN)) == 0)
			is_present = TRUE;
		_delay_us(20);		
	}		
 	 
    // wait
	_delay_us(320);
	// --- end initalization sequence
	
	return is_present;
}

void one_wire_bit_write(u8 bit)
{	
	// --- write bit sequence ---			
	// change port to output (low) - it takes around 3 us to change pin direction to output	
	DQ_DDR |= (1 << DQ_DATA_PIN);	  					
	
	// if bit is logic 1 then we release the bus by changing pin to input, if bit is logic 0 then we keep pin as output low

	if (bit > 0) 
	{	
		// release the bus - it takes around 3 us to change pin direction to input 
		DQ_DDR &= ~(1 << DQ_DATA_PIN);
	}
	
	// wait
	_delay_us(70);		
		
	// release the bus if pin was kept as output low
	DQ_DDR &= ~(1 << DQ_DATA_PIN);
			
	// recovery betweeen slots 1us
	_delay_us(1);
	// --- end write bit sequence ---				
}


u8 one_wire_bit_read(void)
{
	u8 bit;

	// --- read bit sequence ---
	// change port to output (low) - it takes around 3 us to change pin direction to output	
	DQ_DDR |= (1 << DQ_DATA_PIN);							
										
	// release the bus - it takes around 3 us to change pin direction to input
	DQ_DDR &= ~(1 << DQ_DATA_PIN);

	// wait
	_delay_us(10);
	
	// read pin value
	bit = DQ_PIN;
	
	_delay_us(60);
	// --- end read bit sequence ---
	
	bit &= 1 << DQ_DATA_PIN;
	
	return bit;
}

void one_wire_byte_write(u8 byte)
{	
	u8 i, bit;
	
	for (i = 0; i < 8; i++)
	{	
		bit = (byte >> i) & 0x01;	
		one_wire_bit_write(bit);			
	}
}

u8 one_wire_byte_read(void)
{	
	u8 byte = 0;
	u8 i, bit;
	
	for (i = 0; i < 8; i++)
	{	
		bit = one_wire_bit_read();
		if (bit > 0)
			byte |= (0x01 << i);		
	}
	
	return byte;	
}

u8 one_wire_cmd(one_wire_cmd_t cmd, one_wire_sub_cmd_t sub_cmd, u8* wdata, u8 wsize, u8* rdata, u8 rsize)
{
	u8 result = FALSE;
	u8 i;
	
	// 1 wire intialization
	result = one_wire_init_seq();
	if (result == FALSE)
		return result;
			
	// send command
	one_wire_byte_write(cmd);
	
	// send subcommand if needed
	if (sub_cmd != SUBCMD_NONE)	
		one_wire_byte_write(sub_cmd);
	
	// send data to be written if any
	for (i = 0; i < wsize; i++)
		one_wire_byte_write(wdata[i]);
	
	// receive data to be read if any
	for (i = 0; i < rsize; i++)
		rdata[i] = one_wire_byte_read();			
	
	return result;
}

u8 crc8(u8* message, int size)
{
    u8 remainder = 0;	
	u8 bit, byte;
   
    // perform modulo-2 division, a byte at a time    
    for (byte = 0; byte < size; ++byte)
    {        
        // bring the next byte into the remainder.        
        remainder ^= message[byte];
        
        // perform modulo-2 division, a bit at a time.       
        for (bit = 8; bit > 0; --bit)
        {            
            // try to divide the current data bit.            
            if (remainder & 0x01)
            {
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder >> 1);
            }
        }
    }
    
    // the final remainder is the CRC result
    return remainder;
} 
