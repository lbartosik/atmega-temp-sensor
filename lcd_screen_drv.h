#ifndef _LCD_SCREEN
#define _LCD_SCREEN

#define LCD_SCREEN_DDR DDRC
#define LCD_SCREEN_PORT PORTC
#define LCD_SCREEN_LED_PIN PIN_C5
#define LCD_SCREEN_SCE_PIN PIN_C4
#define LCD_SCREEN_RESET_PIN PIN_C3
#define LCD_SCREEN_DC_PIN PIN_C2
#define LCD_SCREEN_DATA_PIN PIN_C1
#define LCD_SCREEN_SCK_PIN PIN_C0





typedef enum
{
	CONTROL_CMD = 0x01,
	DATA_CMD = 0x02

} lcd_cmd_t;

void lcd_screen_init(void);
void write_byte(lcd_cmd_t lcd_cmd, u8 byte);


#endif

