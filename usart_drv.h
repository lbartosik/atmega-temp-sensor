#ifndef _USART
#define _USART

// USART baud rate 4800 bps
#define BAUD 4800
#define USART_UBBR (F_CPU/16/BAUD-1)

// USART Rx, Tx buffers sizes in bytes
#define USART_RX_BUF_SIZE 32
#define USART_TX_BUF_SIZE 96

// Number of command line parameters that we can hold including command name
#define ARGV_SIZE 3

#define USART_LF '\n'
#define USART_NULL '\0'

typedef struct 
{
    // Rx stats
	u16 frame_error;
	u16 data_overrun;
	u16 parity_error;
	// Tx stats
	u16 dropped_cnt;

} usart_stats_t;

typedef struct
{
	u8 argc;
	char* argv[ARGV_SIZE];
} cmd_line_t;


typedef struct
{
	usart_stats_t stats;
	cmd_line_t cmd_line;
	u8 rx_wptr;
	u8 tx_rptr;
	u8 rx_buf[USART_RX_BUF_SIZE];
	u8 tx_buf[USART_TX_BUF_SIZE];		

} usart_ctrl_t;

typedef void (*func_ptr)(usart_ctrl_t* usart_ctrl);

typedef struct
{
	char* cmd_name;
	func_ptr cmd_func_name;
} cmds_t;


void usart_init(void);
void usart_send(usart_ctrl_t* usart_ctrl);
usart_ctrl_t* usart_get_ctrl_blk(void);

#endif