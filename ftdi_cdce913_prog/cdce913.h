#ifndef CDCE913_H_
#define CDCE913_H_
#include <stdint.h>

#define CDCE913_I2C_ADR     (0x65) // default 7 bit i2c adr
//#define CDCE913_I2C_ADR     (0x64) // default 7 bit i2c adr

// I2C operation
#define CDCE913_BYTE_OP     (0x80)
#define CDCE913_BLOCK_OP    (0x00)
#define CDCE913_OFFSET_MASK (0x1f)

//------------------------------------------------------------------------------
#define CDCE913_REG0_ID_ADR (0x00)

// E_EL Xb Device identification (read-only): 1 is CDCE913 (3.3 V out), 0 is CDCEL913 (1.8 V out)
#define CDCE913_E_EL_MASK   (0x80)
#define CDCE913_E_EL_33V    (0x80)
#define CDCE913_E_EL_18V    (0)

// 6:4 RID Xb Revision identification number (read-only)
#define CDCE913_RID_MASK    (0x70)

// 3:0 VID 1h Vendor identification number (read-only)
#define CDCE913_VID_MASK    (0x0f)
#define CDCE913_VID_CODE    (0x1)

//------------------------------------------------------------------------------
#define CDCE913_REG1_ADR        (0x01)
#define CDCE913_EEPIP_BIT       (1 << 6)
#define CDCE913_SLV_ADR_MASK    (0x03)
#define CDCE913_SLV_ADR_DEFAULT (0x01)

//------------------------------------------------------------------------------
#define CDCE913_REG6_ADR        (0x06)
#define CDCE913_EEWRITE_BIT     (0x01)







// All registers nums
#define CDCE913_REG_ALL   (32)


/*
#define REG_02_M1__INPUT_CLK  (0)
#define REG_02_M1__PLL1_CLK   (1)

#define REG_02_SPICON__SDA_SCL  (0)
#define REG_02_SPICON__S1_S2    (1)

#define REG_02_Y1_ST__Y1_PWRDWN          (0)
#define REG_02_Y1_ST__Y1_DISABLE_3_STATE (1)
#define REG_02_Y1_ST__Y1_DISABLE_LOW     (2)
#define REG_02_Y1_ST__Y1_ENABLE          (3)

typedef struct {
    uint8_t pdiv89:2;
    uint8_t y1_st0:2;
    uint8_t y1_st1:2;
    uint8_t spicon:1;
    uint8_t m1:1;
} cdce913_reg_02_t;

typedef union {
    uint8_t data;
    cdce913_reg_02_t reg;
} cdce913_reg_02_ut;

//------------------------------------------------------------------------------
#define REG_14_MUX1__PLL1         (0)
#define REG_14_MUX1__PLL1_BYPASS  (1)

#define REG_14_M2__OUT_Y2_PDIV1   (0)
#define REG_14_M2__OUT_Y2_PDIV2   (1)

#define REG_14_M3__OUT_Y3_PDIV1   (0)
#define REG_14_M3__OUT_Y3_PDIV2   (1)
#define REG_14_M3__OUT_Y3_PDIV3   (2)
#define REG_14_M3__RESERVED       (3)

#define REG_14_Y23_ST__Y23_DISABLE_3_STATE_PLL_PWRDWN (0)
#define REG_14_Y23_ST__Y23_DISABLE_3_STATE            (1)
#define REG_14_Y23_ST__Y23_DISABLE_LOW                (2)
#define REG_14_Y23_ST__Y23_ENABLE                     (3)

typedef struct {
    uint8_t y2y3_st0:2;
    uint8_t y2y3_st1:2;
    uint8_t m3:2;
    uint8_t m2:1;
    uint8_t mux1:1;
} cdce913_reg_14_t;

typedef union {
    uint8_t data;
    cdce913_reg_14_t reg;
} cdce913_reg_14_ut;

//------------------------------------------------------------------------------
// pll n,r,q,p
typedef struct {
		uint32_t vco_range:2;
		uint32_t p:3;
		uint32_t q:6;
		uint32_t r:9;
		uint32_t n:12;
} cdce913_reg_1b_1a_19_18_t; // big endian -> for load to i2c: reg_1b, reg_1a, reg_19, reg_18

typedef union {
	struct {
		uint32_t vco_range:2;
		uint32_t p:3;
		uint32_t q:6;
		uint32_t r:9;
		uint32_t n:12;
	};
	uint32_t data;
	uint8_t data_byte[4];
} cdce913_reg_1b_1a_19_18_ut;
*/



FT_STATUS cdce913_write_block(FT_HANDLE handle, const uint8_t adr, const uint8_t *d, const uint8_t len);
FT_STATUS cdce913_write_byte(FT_HANDLE handle, const uint8_t adr, uint8_t d);
FT_STATUS cdce913_read_byte(FT_HANDLE handle, const uint8_t adr, uint8_t *d);
FT_STATUS cdce913_read_id(FT_HANDLE handle);
#endif
