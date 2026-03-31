#include <stdint.h>
#include <stdio.h>

#include "ftdi.h"
#include "cdce913.h"

#ifndef DEBUG
#define DEBUG 0
#endif

//------------------------------------------------------------------------------
// CDCE913 write block
// adr    - offset in cdce913
// *d     - *data
// result - =0-ok !=0-error
//------------------------------------------------------------------------------
FT_STATUS cdce913_write_block(FT_HANDLE handle, const uint8_t adr, const uint8_t *d, const uint8_t len)
{
	uint32_t i=0;
	FT_STATUS res;
    uint8_t buf_data_len;
    uint8_t buf[ CDCE913_REG_ALL + 2 ]; // +2 t.k. add in buf (adr + len)= 2 bytes

    uint8_t a = (CDCE913_OFFSET_MASK & adr) | CDCE913_BLOCK_OP;

    buf[0] = a;
    buf[1] = len;

    for (i=0; i<len; i++) {
        buf[2 + i] = d[i];
    }

    buf_data_len = 2 + len;

    res = i2c_write_array(handle, CDCE913_I2C_ADR, buf, buf_data_len);

    return res;
}

//------------------------------------------------------------------------------
// CDCE913 write one byte
// adr    - offset in cdce913
// d      - data
// result - =0-ok !=0-error
//------------------------------------------------------------------------------
FT_STATUS cdce913_write_byte(FT_HANDLE handle, const uint8_t adr, uint8_t d)
{
	FT_STATUS res;
    const uint8_t len = 2;
    uint8_t buf[2];

    uint8_t a = (CDCE913_OFFSET_MASK & adr) | CDCE913_BYTE_OP;

    buf[0] = a;
    buf[1] = d;

    res = i2c_write_array(handle, CDCE913_I2C_ADR, buf, len);

    return res;
}

//------------------------------------------------------------------------------
// CDCE913 read one byte
// adr    - offset in cdce913
// d      - return read data
// result - =0-ok !=0-error
//------------------------------------------------------------------------------
FT_STATUS cdce913_read_byte(FT_HANDLE handle, const uint8_t adr, uint8_t *d)
{
	FT_STATUS res;
    const uint8_t len = 1;
    uint8_t a = (CDCE913_OFFSET_MASK & adr) | CDCE913_BYTE_OP;

    res = i2c_write_array(handle, CDCE913_I2C_ADR, &a, len);

    if (res) {
        return res;
    }

    res = i2c_read_array(handle, CDCE913_I2C_ADR, d, len);

    return res;
}

//------------------------------------------------------------------------------
// CDCE913 Read ID
// return:
//  =0 - ok
// !=0 - error
//------------------------------------------------------------------------------
FT_STATUS cdce913_read_id(FT_HANDLE handle)
{
    uint8_t d;
    FT_STATUS res;

	printf(" CDCE913 Read ID.\n");

	res = cdce913_read_byte(handle, CDCE913_REG0_ID_ADR, &d);
    if (res) return res;

	//printf("D = %d\n", d);

    if ((d & CDCE913_E_EL_MASK) == CDCE913_E_EL_33V) {
        printf("E_EL = %d\n", d & CDCE913_E_EL_MASK);
        printf("CDCE913 - 3.3v\n");
    } else {
        printf("CDCEL913 - 1.8v\n");
    }

    printf("RID = %d\n", d & CDCE913_RID_MASK);

    if ((d & CDCE913_VID_MASK) != CDCE913_VID_CODE) {
        printf("ERROR: CDCE913_VID_CODE\n");
        return FT_IO_ERROR;
    }
    printf("VID = %d\n", d & CDCE913_VID_MASK);

    return FT_OK;
}


