#include <stdio.h>
#include <avr/io.h> 
#include <util/delay.h> 
#include <avr/pgmspace.h>   
#include "common.h" 
#include "temp_sensor.h"
#include "1wire_drv.h" 
#include "usart_drv.h"

static temp_sensor_ctrl_t temp_sensor_ctrl;

u8 read_scratch_pad(void)
{
	u8 result = TRUE;

	// execute 1 wire command	
	result = one_wire_cmd(SKIP_ROM_CMD, READ_SCRATCHPAD, NULL, 0, (u8*) &temp_sensor_ctrl.scratchpad, 9);
	if (result == FALSE)
	{
		temp_sensor_ctrl.stats.is_present_error++;
		return result;
	}

	// calculate crc
	temp_sensor_ctrl.scratchpad.calc_crc = crc8((u8*) &temp_sensor_ctrl.scratchpad, 8);		
	if (temp_sensor_ctrl.scratchpad.calc_crc != temp_sensor_ctrl.scratchpad.crc)
	{
		temp_sensor_ctrl.stats.crc_error++;
		result = FALSE;
	}
	
	return result;
}

u8 init_temp_conversion(void)
{
	u8 result = TRUE;

	// execute 1 wire command	
	result = one_wire_cmd(SKIP_ROM_CMD, CONVERT_TEMP, NULL, 0, NULL, 0);
	if (result == FALSE)
	{
		temp_sensor_ctrl.stats.is_present_error++;
		return result;
	}
	
	return result;
}

config_reg_t get_temp_config(void)
{
	return temp_sensor_ctrl.scratchpad.config_reg;
}

temp_sensor_stats_t* get_temp_sensor_stats(void)
{
	return &temp_sensor_ctrl.stats;
}

u16 get_temp_value_raw(void)
{
	u16 temp = temp_sensor_ctrl.scratchpad.temp_msb << 8 | temp_sensor_ctrl.scratchpad.temp_lsb;
	temp &= (ALL_BITS_MASK << (MAX_TEMP_RESOLUTION - temp_sensor_ctrl.scratchpad.config_reg.cfg)); 
	return temp;
}

u8 get_temp_value_int(void)
{
	u16 temp = temp_sensor_ctrl.scratchpad.temp_msb << 8 | temp_sensor_ctrl.scratchpad.temp_lsb;
	// remove fractional bits
	temp >>= FRACTIONAL_BITS_NUM;
	return (u8) temp;
}

u8 get_temp_value_frac(void)
{
	u16 temp = get_temp_value_raw();
	u16 val;
	
	// remove integer bits
	temp = +temp;	
	temp &= FRAC_BITS_MASK;
	
	// 0.0625 is our unit
	// 0.0625 -> 25, 0.1 -> 40	
	val = (temp*25 + 20)/40; 
	
	return (u8) val;
}

u8 get_temp_resolution(void)
{
	u8 temp_resol_bits = 0;
	
	switch(temp_sensor_ctrl.scratchpad.config_reg.cfg)
	{
		case RES_9_BITS:
			temp_resol_bits = 9;
		break;
			
		case RES_10_BITS:
			temp_resol_bits = 10;
		break;

		case RES_11_BITS:
			temp_resol_bits = 11;
		break;
		
		case RES_12_BITS:
			temp_resol_bits = 12;
		break;		
	}
	
	return temp_resol_bits;
}

u8 set_temp_resolution(temp_resolution_t temp_resolution)
{						
#ifdef INIT_DEBUG	
	static char response[] PROGMEM = "resolution changed to = %x, cfg = %x\n->";	
	usart_ctrl_t* usart_ptr = usart_get_ctrl_blk();
#endif    
	u8 result = TRUE;	
	
	// compare temperature resolutions
	if (temp_sensor_ctrl.scratchpad.config_reg.cfg != temp_resolution)
	{				
		// set resolution
		temp_sensor_ctrl.scratchpad.config_reg.cfg = temp_resolution;
	
		// write scratchpad first 3 bytes
		result = one_wire_cmd(SKIP_ROM_CMD, WRITE_SCRATCHPAD, (u8*) &temp_sensor_ctrl.scratchpad.temp_h_reg, 3, NULL, 0);
		if (result == FALSE)
		{
			temp_sensor_ctrl.stats.is_present_error++;
			return result;
		}
		
		// write scratchpad 3 bytes to eeprom	
		result = one_wire_cmd(SKIP_ROM_CMD, COPY_SCRATCHPAD, NULL, 0, NULL, 0);
		if (result == FALSE)
		{
			temp_sensor_ctrl.stats.is_present_error++;
			return result;
		}
		
#ifdef INIT_DEBUG		
		snprintf_P((char*) usart_ptr->tx_buf, USART_TX_BUF_SIZE, response, temp_resolution, *((u8*)&temp_sensor_ctrl.scratchpad.config_reg));
		usart_send(usart_ptr);				
#endif
	}		
	
	return result;
}
