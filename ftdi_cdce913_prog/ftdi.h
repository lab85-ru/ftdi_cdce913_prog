#ifndef FTDI_H_
#define FTDI_H_

#include <stdint.h>
#include "ftd2xx.h"

/*MPSSE Control Commands*/
#define MPSSE_CMD_SET_DATA_BITS_LOWBYTE		0x80
#define MPSSE_CMD_SET_DATA_BITS_HIGHBYTE	0x82
#define MPSSE_CMD_GET_DATA_BITS_LOWBYTE		0x81
#define MPSSE_CMD_GET_DATA_BITS_HIGHBYTE	0x83

#define MPSSE_CMD_SEND_IMMEDIATE			0x87
#define MPSSE_CMD_ENABLE_3PHASE_CLOCKING	0x8C
#define MPSSE_CMD_DISABLE_3PHASE_CLOCKING	0x8D
#define MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO	0x9E

/*MPSSE Data Command - LSB First */
#define MPSSE_CMD_DATA_LSB_FIRST			0x08

/*MPSSE Data Commands - bit mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BITS_POS_EDGE	0x12
#define MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE	0x13
#define MPSSE_CMD_DATA_IN_BITS_POS_EDGE		0x22
#define MPSSE_CMD_DATA_IN_BITS_NEG_EDGE		0x26
#define MPSSE_CMD_DATA_BITS_IN_POS_OUT_NEG_EDGE	0x33
#define MPSSE_CMD_DATA_BITS_IN_NEG_OUT_POS_EDGE	0x36


/*MPSSE Data Commands - byte mode - MSB first */
#define MPSSE_CMD_DATA_OUT_BYTES_POS_EDGE	0x10
#define MPSSE_CMD_DATA_OUT_BYTES_NEG_EDGE	0x11
#define MPSSE_CMD_DATA_IN_BYTES_POS_EDGE	0x20
#define MPSSE_CMD_DATA_IN_BYTES_NEG_EDGE	0x24
#define MPSSE_CMD_DATA_BYTES_IN_POS_OUT_NEG_EDGE	0x31
#define MPSSE_CMD_DATA_BYTES_IN_NEG_OUT_POS_EDGE	0x34


/*SCL & SDA directions*/
#define DIRECTION_SCLIN_SDAIN				0x10
#define DIRECTION_SCLOUT_SDAIN				0x11
#define DIRECTION_SCLIN_SDAOUT				0x12
#define DIRECTION_SCLOUT_SDAOUT				0x13

/*SCL & SDA values*/
#define VALUE_SCLLOW_SDALOW					0x00
#define VALUE_SCLHIGH_SDALOW				0x01
#define VALUE_SCLLOW_SDAHIGH				0x02
#define VALUE_SCLHIGH_SDAHIGH				0x03

/*Data size in bits*/
#define DATA_SIZE_8BITS						0x07
#define DATA_SIZE_1BIT						0x00



FT_STATUS ftdi_open(FT_HANDLE *handle);
FT_STATUS i2c_read_array(FT_HANDLE handle, const uint8_t i2c_dev_adr, uint8_t *buf, const uint8_t buf_len);
FT_STATUS i2c_write_array(FT_HANDLE handle, const uint8_t i2c_dev_adr, const uint8_t *buf, const uint8_t buf_len);

#endif
