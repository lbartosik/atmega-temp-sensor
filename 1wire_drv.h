#ifndef _1WIRE
#define _1WIRE

#define DQ_DDR DDRB
#define DQ_PORT PORTB
#define DQ_PIN PINB

#define DQ_DATA_PIN PIN_B2
#define DQ_INIT_LOOPS 6

// crc define
#define POLYNOMIAL 0x8C  /* 100110001 followed by 0's */

u8 one_wire_cmd(one_wire_cmd_t cmd, one_wire_sub_cmd_t sub_cmd, u8* wdata, u8 wsize, u8* rdata, u8 rsize);
u8 crc8(u8* message, int size);

u8 one_wire_init_seq(void);
u8 one_wire_bit_read(void);
u8 one_wire_byte_read(void);
void one_wire_byte_write(u8 byte);
// don't make this function static
void one_wire_bit_write(u8 bit);

#endif