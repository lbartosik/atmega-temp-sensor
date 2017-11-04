#ifndef _COMMON
#define _COMMON

//#define INIT_DEBUG 

typedef signed char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define TRUE 1
#define FALSE 0

#define PIN_C5 5
#define PIN_C4 4
#define PIN_C3 3
#define PIN_C2 2
#define PIN_C1 1
#define PIN_C0 0

#define PIN_B5 5
#define PIN_B4 4
#define PIN_B3 3
#define PIN_B2 2
#define PIN_B1 1
#define PIN_B0 0

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_DATA_PIN PIN_B1

void update_temperature_on_lcd(u8 force_update);

#endif