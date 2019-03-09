#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "common.h" 
#include "usart_drv.h"
#include "timer_drv.h" 
#include "temp_sensor.h"
#include "1wire_drv.h"
#include "lcd_screen_drv.h"

static void usart_process_rcv_data(usart_ctrl_t* usart_ctrl);

static void usart_status_cmd(usart_ctrl_t* usart_ctrl);
 void temp_sensor_status_cmd(usart_ctrl_t* usart_ctrl);
static void read_scratchpad_cmd(usart_ctrl_t* usart_ctrl);
static void init_temp_conversion_cmd(usart_ctrl_t* usart_ctrl);
static void delay_test_cmd(usart_ctrl_t* usart_ctrl);
static void lcd_led_ctrl_cmd(usart_ctrl_t* usart_ctrl);
static void usart_help_cmd(usart_ctrl_t* usart_ctrl);

  
static usart_ctrl_t usart_ctrl;

static char usart_help_str[] PROGMEM = "help";
static char usart_status_str[] PROGMEM = "usart_status";
static char temp_sensor_status_str[] PROGMEM = "temp_sensor_status";
static char read_scratchpad_str[] PROGMEM = "read_scratchpad";
static char init_temp_conversion_str[] PROGMEM = "init_temp_conversion";
static char delay_test_str[] PROGMEM = "delay";
static char lcd_led_ctrl[] PROGMEM = "lcd_led_ctrl";

static cmds_t cmds[] PROGMEM =
{	
	{ usart_help_str, usart_help_cmd },
	{ usart_status_str, usart_status_cmd },
	{ temp_sensor_status_str, temp_sensor_status_cmd },
	{ read_scratchpad_str, read_scratchpad_cmd },	
	{ init_temp_conversion_str, init_temp_conversion_cmd },
	{ delay_test_str, delay_test_cmd },	
	{ lcd_led_ctrl, lcd_led_ctrl_cmd },	
};

void usart_init(void)
{
	memset(&usart_ctrl, 0, sizeof(usart_ctrl_t));

	// set baud rate 4800 bpsk
	UBRR0H = (u8)((USART_UBBR)>>8);
	UBRR0L = (u8) USART_UBBR;
	
	// set frame format: 8 data bits, no parity bit, 1 stop bit
	UCSR0C = (3<<UCSZ00);
	
	// enable receiver and transmitter
	UCSR0B = (1<<RXEN0) |(1<<TXEN0);
	
	// enable USART Rx complete interrupt
	UCSR0B |= (1 << RXCIE0);
}

usart_ctrl_t* usart_get_ctrl_blk(void)
{
	return &usart_ctrl;
}

void usart_send(usart_ctrl_t* usart_ctrl)
{


	// if UDRIE is enabled then we can't send the msg
	if ((UCSR0B & (1 << UDRIE0)) > 0)
	{
		usart_ctrl->stats.dropped_cnt++;
		return;
	}

	if (usart_ctrl->tx_buf[0] == USART_NULL)
	{
			
		// nothing to send;
		return;
	}
	else
	{
		// enable UDRIE interrupt
		UCSR0B |= (1 << UDRIE0);
		// send first character
		usart_ctrl->tx_rptr = 0;
		UDR0 = usart_ctrl->tx_buf[usart_ctrl->tx_rptr++];

	}	
}

// USART Rx Complete interrupt
ISR(USART_RX_vect)
{
    // service errors
	if ((UCSR0A & (1 << FE0)) > 0)
	    // frame error
		usart_ctrl.stats.frame_error++;
	
	if ((UCSR0A & (1 << DOR0)) > 0)
	    // data overrun
		usart_ctrl.stats.data_overrun++;	

	// read received character
	u8 val = UDR0;
	
	// check if received value is a new line
	if (val == USART_LF)
	{
		// process received data
		usart_process_rcv_data(&usart_ctrl);
		usart_ctrl.rx_wptr = 0;
		memset(&usart_ctrl.rx_buf[0], 0, USART_RX_BUF_SIZE);
	}
	else
	{	
		// store value in USART receive buffer
		usart_ctrl.rx_buf[usart_ctrl.rx_wptr++] = val;
		if (usart_ctrl.rx_wptr >= USART_RX_BUF_SIZE)
		{
			usart_ctrl.rx_wptr = 0;
			memset(&usart_ctrl.rx_buf[0], 0, USART_RX_BUF_SIZE);
		}
	}	

	// debug just to see that we are receiving something
	//PORTB ^= (1 << PIN_B1);
	LED_PORT ^= (1 << LED_DATA_PIN); 			
}

// USART Data Register Empty interrupt
ISR(USART_UDRE_vect)
{
	if (usart_ctrl.tx_buf[usart_ctrl.tx_rptr] == USART_NULL)
	{
		// end of transmission, disable UDRIE interrupt
		UCSR0B ^= 1 << UDRIE0;		
	}
	else
	{
		// send next character
		UDR0 = usart_ctrl.tx_buf[usart_ctrl.tx_rptr++];	
	}
}

static void usart_process_rcv_data(usart_ctrl_t* usart_ctrl)
{	
    func_ptr func = NULL;
	u8 msg_parser_size = sizeof(cmds)/sizeof(cmds_t);
	u8 i, found = FALSE;
	u8 cmd_name_len;
	
	memset(&usart_ctrl->cmd_line, 0, sizeof(cmd_line_t));
	
	// split received buffer
	usart_ctrl->cmd_line.argv[0] = strtok((char*) usart_ctrl->rx_buf, " ");
	for (i = 1; i < ARGV_SIZE; i++)
	{
		usart_ctrl->cmd_line.argv[i] = strtok(NULL, " ");
	}
	
	// count number of params
	i = 0;
	while (i < ARGV_SIZE)
	{
		if (usart_ctrl->cmd_line.argv[i] != NULL)
			usart_ctrl->cmd_line.argc++;
		else
			break;
		i++;
	}
		
	
	for (i = 0; i < msg_parser_size; i++)
	{	
		cmd_name_len = strlen_P((PGM_P)pgm_read_word(&cmds[i].cmd_name));

		if (//(usart_ctrl->rx_wptr == cmd_name_len) &&
		   
			(strncmp_P((char*) usart_ctrl->cmd_line.argv[0], (PGM_P)pgm_read_word(&cmds[i].cmd_name), cmd_name_len) == 0))
			{
			    // read function pointer from program memory
				func = (func_ptr) pgm_read_word(&cmds[i].cmd_func_name);
				// call function
				func(usart_ctrl);
				found = TRUE;
			}
	}	

	if (found == FALSE)
	{
		static char response[] PROGMEM = "unknown command\n->";
		snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response);
		usart_send(usart_ctrl);
	}	
}

static void usart_status_cmd(usart_ctrl_t* usart_ctrl)
{
	static char response[] PROGMEM = "USART STATUS\nframe error %d\ndata overrun %d\ndropped msgs %d\n->";
	
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response, usart_ctrl->stats.frame_error, usart_ctrl->stats.data_overrun, usart_ctrl->stats.dropped_cnt);
	usart_send(usart_ctrl);
}

static void delay_test_cmd(usart_ctrl_t* usart_ctrl)
{	
    /*
	static char response[] PROGMEM = "pr = %d, diff = %d, crc = %x, 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x 0x%2.0x\n->";
	u8 ticks_start, ticks_stop, diff;	
	u8 is_present = FALSE;
	u8 crc;
				
	u8 rom_code[9];
	memset(rom_code, 0, 9);
	
	//is_present = one_wire_cmd(READ_ROM_CMD, SUBCMD_NONE, NULL, 0, &rom_code[0], 8);
	is_present = one_wire_cmd(SKIP_ROM_CMD, READ_SCRATCHPAD, NULL, 0, &rom_code[0], 9);
	
	ticks_start = TCNT0;		
	//read_scratch_pad();	
	ticks_stop = TCNT0;

    if (ticks_stop > ticks_start)
		diff = ticks_stop - ticks_start;
	else
		diff = ticks_stop + (255 - ticks_start);	
	
	crc = crc8(rom_code, 8);
	
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response, is_present, diff, crc, rom_code[0], rom_code[1], rom_code[2], rom_code[3], 
																					  rom_code[4], rom_code[5], rom_code[6], rom_code[7], rom_code[8]);																				  
	*/
	
	static char response1[] PROGMEM = "Delay in ticks %u, resolution %u\n->";	
	static char response2[] PROGMEM = "Overflow, delay in ticks %u, resolution %u\n->";	
	u16 diff;	
	
	start_timer1_meas(TC1_CLK_IO_64_PRESC);
	
	update_temperature_on_lcd(TRUE);
		   
	diff = stop_timer1_meas();	
	
	/*
	// update lcd screen with new temperature
	nokia_lcd_clear();                          3,1 ms 
	nokia_lcd_set_cursor(9, 0);                 64 us
	nokia_lcd_write_string("Temperatura", 1);   72 ms
	nokia_lcd_set_cursor(0, 16);	            64 us
	nokia_lcd_write_string("+25.5", 3);		    276 ms	
	nokia_lcd_render();							171 ms
	
	init_temp_conversion();                     3,1 ms
	read_scratch_pad();                         12,4 ms
	
	update_temperature_on_lcd(TRUE);			540 ms	
	
	
	write + render = 519 ms
	temp_stuff = 20 ms
	
	*/
		
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, diff == TC1_OVERFLOW ? response2 : response1, diff, get_timer1_meas_resolution());
	usart_send(usart_ctrl);	
}

extern temp_sensor_ctrl_t temp_sensor_ctrl;

static void read_scratchpad_cmd(usart_ctrl_t* usart_ctrl)
{
	static char response[] PROGMEM = "is_present = %d, temp = %d.%d(0x%x), resol = %dbits(0x%x)\n->";
	static char response1[] PROGMEM = "b0=%x, b1=%x, b2=%x, b3=%x, b4=%x, b5=%x, b6=%x, b7=%x, b8=%x\n->";
	u8 is_present = FALSE;
	
	is_present = read_scratch_pad();			
		
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response, is_present, get_temp_value_int(), get_temp_value_frac(), get_temp_value_raw(), get_temp_resolution(), get_temp_config());	
	usart_send(usart_ctrl);	
	
	
	u8 tmp;
	memcpy(&tmp, &temp_sensor_ctrl.scratchpad.config_reg, 1);
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response1, temp_sensor_ctrl.scratchpad.temp_lsb, temp_sensor_ctrl.scratchpad.temp_msb,
				temp_sensor_ctrl.scratchpad.temp_h_reg, temp_sensor_ctrl.scratchpad.temp_l_reg, tmp, 
				(u8) temp_sensor_ctrl.scratchpad.reserved[0], temp_sensor_ctrl.scratchpad.reserved[1], temp_sensor_ctrl.scratchpad.reserved[2],
				temp_sensor_ctrl.scratchpad.crc);
	usart_send(usart_ctrl);

}

static void init_temp_conversion_cmd(usart_ctrl_t* usart_ctrl)
{
	static char response[] PROGMEM = "ok\n->";

	init_temp_conversion();
	
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response);
	usart_send(usart_ctrl);	
}


static void lcd_led_ctrl_cmd(usart_ctrl_t* usart_ctrl)
{
	static char response[] PROGMEM = "ok\n->";

	LCD_SCREEN_PORT ^= (1 << LCD_SCREEN_LED_PIN); 
	
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response);
	usart_send(usart_ctrl);	
}

static void usart_help_cmd(usart_ctrl_t* usart_ctrl)
{
	static char response[] PROGMEM = "Commands:\nusart_status\nread_scratchpad\ninit_temp_conversion\ndelay\nlcd_led_ctrl\n->";
	
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response);
	usart_send(usart_ctrl);	
}

 void temp_sensor_status_cmd(usart_ctrl_t* usart_ctrl)
{
	temp_sensor_stats_t* stats = get_temp_sensor_stats();
	
	static char response[] PROGMEM = "TEMP SENSOR STATUS\npresent error %d\ncrc error %d\nlcd update not required %d\n->";
					
	snprintf_P((char*) usart_ctrl->tx_buf, USART_TX_BUF_SIZE, response, stats->is_present_error, stats->crc_error, stats->lcd_update_not_required);
	usart_send(usart_ctrl);	

}


