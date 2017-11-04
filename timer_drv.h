#ifndef _TIMER
#define _TIMER

typedef enum 
{
	TC1_CLK_IO_NO_PRESC   = 0x0,
	TC1_CLK_IO_1_PRESC    = 0x1,
	TC1_CLK_IO_8_PRESC    = 0x2,
	TC1_CLK_IO_64_PRESC   = 0x3,
	TC1_CLK_IO_256_PRESC  = 0x4,
	TC1_CLK_IO_1024_PRESC = 0x5
} tc1_resolution_t;

typedef enum
{
	TC2_CLK_IO_NO_PRESC   = 0x1,
	TC2_CLK_IO_1024_PRESC = 0x7
} tc2_resolution_t;


#define TC1_OVERFLOW 0xFFFF
#define US_PER_SECOND 1000000
#define CONVERT_TICKS_TO_US(ticks, resolution) (((u32)ticks)*((u32)resolution))


#define TIMER2_WAKEUP_CNT 0x2

void start_timer1_meas(tc1_resolution_t res);
u16 stop_timer1_meas(void);
u16 get_timer1_meas_resolution(void);

void setup_timer2_asynch(void);
void enter_sleep_mode(void);

#endif