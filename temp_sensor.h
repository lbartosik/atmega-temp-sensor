#ifndef _TEMP_SENSOR
#define _TEMP_SENSOR

#define MAX_TEMP_RESOLUTION 3
#define FRACTIONAL_BITS_NUM 4
#define ALL_BITS_MASK 0xFFFF
#define FRAC_BITS_MASK 0x000F

typedef enum
{
	READ_ROM_CMD = 0x33,
	SKIP_ROM_CMD = 0xCC

} one_wire_cmd_t;

typedef enum
{
	SUBCMD_NONE = 0x01,	
	CONVERT_TEMP = 0x44,	
	WRITE_SCRATCHPAD = 0x4E,
	READ_SCRATCHPAD = 0xBE,		
	COPY_SCRATCHPAD = 0x48,
	RECALL_EEPROM = 0xB8,
	READ_POWER_SUPPLY = 0xB4
		
} one_wire_sub_cmd_t;

typedef enum
{
	RES_9_BITS = 0x00,
	RES_10_BITS = 0x01,
	RES_11_BITS = 0x02,
	RES_12_BITS = 0x03,
	
} temp_resolution_t;

typedef struct
{
	u8 res1:5;
	u8 cfg:2;
	u8 res2:1;	
} config_reg_t;

typedef struct
{
	u8 family_code;
	u8 serial_number[6];
	u8 crc;
	u8 calc_crc;
} rom_code_t;

typedef struct
{
	u16 is_present_error;
	u16 crc_error;
	u16 lcd_update_not_required;
} temp_sensor_stats_t;

typedef struct
{
	u8 temp_lsb;
	u8 temp_msb;
	u8 temp_h_reg;
	u8 temp_l_reg;
	config_reg_t config_reg;
	u8 reserved[3];
	u8 crc;
	u8 calc_crc;
} scratchpad_t;

typedef struct
{
	rom_code_t rom_code;
	scratchpad_t scratchpad;
	temp_sensor_stats_t stats;	
} temp_sensor_ctrl_t;

u8 read_scratch_pad(void);
u8 init_temp_conversion(void);
u16 get_temp_value_raw(void);
u8 get_temp_value_int(void);
u8 get_temp_value_frac(void);
config_reg_t get_temp_config(void);
u8 get_temp_resolution(void);
u8 set_temp_resolution(temp_resolution_t temp_resolution);
temp_sensor_stats_t* get_temp_sensor_stats(void);

#endif