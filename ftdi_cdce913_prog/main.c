//
//
/******************************************************************************/
/* 							 Include files										   */
/******************************************************************************/
/* Standard C libraries */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "ftd2xx.h"
#include "ftdi.h"
#include "cdce913.h"

#include "get_opt.h"
#include "git_commit.h"

const char PRINT_HELP[] = {
"----------------------------------------------------------------------------\n\r"
"- Prog CDCE913 in FTx232x. ----------------------\n\r"
"    Parameters:\n\r"
" --filecfg <file-name> - config BIN file.\n\r"
" --eeprom              - write config file to EEPROM pll.\n\r"
" --write-reg           - write config to registers pll only (NO EEPROM).\n\r"
"----------------------------------------------------------------------------\n\r"
};

const char PRINT_TIRE[] = {"================================================================================\n\r"};
const char PRINT_PROG_NAME[] = {" CDCE913 FTx232x PROGRAMMATOR\n\r"};
const char PRINT_VERSION[] = {" Ver 1.0 2026. Sviridov Georgy. sgot@inbox.ru\n\r"};

//------------------------------------------------------------------------------
typedef struct {
    uint32_t flag_eeprom;
    uint32_t flag_write_reg;
    char *str_filename;
} cmd_line_param_opt_t;

cmd_line_param_opt_t param_opt = {0, 0, 0};

get_opt_t opt[] = {
//   "key",          type,                     flag,                                       int,        str
    {"--filecfg",    GET_OPT_VALUE_TYPE_STR,   0,                                          0,          &param_opt.str_filename },
    {"--eeprom",     GET_OPT_VALUE_TYPE_FLAG,  (get_opt_flag_t*)&param_opt.flag_eeprom,    0,          0          },
    {"--write-reg",  GET_OPT_VALUE_TYPE_FLAG,  (get_opt_flag_t*)&param_opt.flag_write_reg, 0,          0          },
};
const size_t get_opt_n = sizeof(opt) / sizeof(get_opt_t);

// Config size (bytes)
#define CDCE913_CONFIG_SIZE (32)
uint8_t pll_buf[ CDCE913_CONFIG_SIZE ];



//=============================================================================
// MAIN
//=============================================================================
int main(int argc, char* argv[])
{
	FT_HANDLE ftHandle;
	FT_STATUS status = FT_OK;
	size_t i = 0;
	struct stat stat_buf;
	FILE * fi = 0;
	int res = 0;
    size_t r_size = 0;
    size_t result = 0;
	uint8_t data;
	uint8_t reg;
	uint32_t adr_start;

	printf( PRINT_TIRE );
    printf( PRINT_PROG_NAME );
	printf( PRINT_VERSION );
    printf( " GIT = %s\n\r", git_commit_str );
	printf( PRINT_TIRE );

    // razbor perametrov
	if (argc == 1) {
	    printf( PRINT_HELP );
		return 0;
	}

	//for (int i=0;i<argc;i++){
	//	printf("argv[ %d ] = %s\n\r", i, argv[i]);
	//}

    res = get_opt(argc, argv, opt, get_opt_n);
	if (res == -1){
		printf("\n\rERROR: input parameters.\n\r");
		return 1;
	}

	if (param_opt.str_filename == 0) {
		printf("ERROR: config file name is not defined.\n");
		return 1;
	}

	if (stat(param_opt.str_filename, &stat_buf) == -1) {
		printf("ERROR: file %s not found!\n\r", param_opt.str_filename);
		return 1;
	}

	if (stat_buf.st_size == 0 || stat_buf.st_size != CDCE913_CONFIG_SIZE) {
		printf("ERROR: config file size error.\n\r");
		return 1;
	}

	// LOAD file ---------------------------------------------------------------
	printf("Read config file.\n\r");

	fi = fopen(param_opt.str_filename, "rb");
	if ( fi == NULL ) {
		printf("ERROR: open file.\n\r");
		return 1;
	}

	r_size = CDCE913_CONFIG_SIZE;
    result = fread( pll_buf, 1, r_size, fi);
	if (result != r_size) {
	    printf("ERROR: read data from file.\n\r");
		return 1;
	}
    fclose( fi );

	// FTDI --------------------------------------------------------------------
    status = ftdi_open(&ftHandle);
	if (status != FT_OK) {
		printf("ERROR: FTDI not found...\n");
		return 1;
	}
	printf(" FTDI Open - OK\n");

	status = cdce913_read_id(ftHandle);
	if (status != FT_OK) {
		printf("ERROR: FTDI read ID.\n");
		return 1;
	}
	printf("Ok\n");

    printf("PLL Read reg--------------------------------------------------------\n");

    for (i=0; i<CDCE913_REG_ALL; i++) {
        status = cdce913_read_byte(ftHandle, i, &data);
    	if (status) {
	    	printf("res = %d\n", status);
		    return 1;
	    }
        printf(" Adr(%02d) = data(0x%02X)\n", i, data);
    }

	if (param_opt.flag_eeprom == 0 && param_opt.flag_write_reg == 0) {
		return 0;
	}

    printf("PLL Write (No write to flash) ---------------------------------------\n");
    // set default i2c adr
    reg = pll_buf[ CDCE913_REG1_ADR ];
    reg = (reg & ~CDCE913_SLV_ADR_MASK) | CDCE913_SLV_ADR_DEFAULT;
    pll_buf[ CDCE913_REG1_ADR ] = reg;

    adr_start = 1;
    status = cdce913_write_block(ftHandle, adr_start, &pll_buf[adr_start], CDCE913_REG_ALL - adr_start);
	if (status) {
		printf("res = %d\n", status);
		return 1;
	}
    printf("Ok\n");

	if (param_opt.flag_eeprom == 0) {
		return 0;
	}

    printf("PLL Write to flash) -------------------------------------------------\n");

    printf("SET EEWRITE\n");

    reg = pll_buf[ CDCE913_REG6_ADR ] | CDCE913_EEWRITE_BIT;
    status = cdce913_write_byte(ftHandle, CDCE913_REG6_ADR, reg);
   	if (status) {
    	printf("ERROR: res = %d\n", status);
	    return 1;
    }
    //printf("res = %d, adr(%02d) = data(0x%02X)\n", res, CDCE913_REG6_ADR, reg);

    printf("PLL Wait EEPIP\n");
    while (1) {
        status = cdce913_read_byte(ftHandle, CDCE913_REG1_ADR, &data);
    	if (status) {
	    	printf("ERROR: res = %d\n", status);
		    return 1;
	    }
        //printf("res = %d, adr(%02d) = data(0x%02X)\n", res, CDCE913_REG1_ADR, data);
        if ((data & CDCE913_EEPIP_BIT) == 0) {
            printf("Write - complite\n");
            break;
        } else {
            printf("Write - busy\n");
            Sleep(10);
        }
    } // while -----------------------------------------------------------------

    printf("PLL CLEAR EEWRITE\n");

    reg = pll_buf[ CDCE913_REG6_ADR ];
    reg = reg & ~CDCE913_EEWRITE_BIT;
    pll_buf[ CDCE913_REG6_ADR ] = reg;

    status = cdce913_write_byte(ftHandle, CDCE913_REG6_ADR, reg);
   	if (status) {
		printf("ERROR: res = %d\n", status);
	    return 1;
    }
    //printf("res = %d, adr(%02d) = data(0x%02X)\n", res, CDCE913_REG6_ADR, reg);

    printf("OK\n");

	return 0;
}

