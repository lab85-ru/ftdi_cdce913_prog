// get_opt for Windows
// v1.0 2024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "get_opt.h"
#include "conv_str_to_uint32.h"


//==============================================================================
// type string DEC or HEX
//
// return:
// -1 error(format string)
// > 0 len in string
//==============================================================================
static type_string_e get_type_string(const char * st)
{
	size_t len   = strlen(st);
	size_t index = 0;
	char c;
	type_string_e tse;

    if (len == 0) {
		return GTS_ERROR;
    }

	// scan
	while(index != len) {
		c = *(st + index);
		switch (index) {
			case 0: {
				if (c >= '0' && c <= '9') tse = GTS_DEC;
				else return GTS_ERROR;
				break;
			}

			case 1: {
				if (c >= '0' && c <= '9') tse = GTS_DEC;
				else if (c == 'x' || c == 'X') tse = GTS_HEX;
				else return GTS_ERROR;
				break;
		    }

			default: {
				if (tse == GTS_DEC && (c >= '0' && c <= '9')) {
                    break;
                } else if (tse == GTS_HEX && ((c >= '0' && c <= '9') ||
                                              (c >= 'a' && c <= 'f') ||
                                              (c >= 'A' && c <= 'F'))) {
                    break;
                } else {
                    return GTS_ERROR;
                }
				break;
			}

		} // switch(index){
		index++;
	} // while(index != len){

	if (tse == GTS_DEC && len > LEN_STRING_DEC_MAX) return GTS_ERROR;
	if (tse == GTS_HEX && len > LEN_STRING_HEX_MAX) return GTS_ERROR;

	return tse;
}

//==============================================================================
// conver string -> uint32
//
// return:
// -1 error(format string)
// > 0 len in string
//==============================================================================
static int conv_to_uint32(const char *st, uint32_t *ui32)
{
	uint32_t ui = 0;

	switch ( get_type_string(st) ) {
	case GTS_HEX: {
		return conv_str_to_uint32( (uint8_t*)st, ui32);
	}

	case GTS_DEC: {
        ui = atol(st);
		*ui32 = (uint32_t)ui;
		return strlen(st);
	}

	default:
		return -1;
	} // switch ( get_type_string(st) ){

}

//------------------------------------------------------------------------------
// Find & parse paramter from cmd line.
// return:
//    x - kolihestvo naydenih parametrov
//    0 - no parameter
//   -1 - error parameter
//------------------------------------------------------------------------------
int get_opt(const int argc, char** argv, get_opt_t *opt, const uint32_t opt_size)
{
	int res           = 0;
	int param_counter = 0;
	uint32_t i        = 0;
	uint32_t j        = 0;
	int flag_param_ok = 0;

	if (argc == 1) return 0;

	// i start from 1, t.k. =0 programm name
    i = 1;
    while (i != argc) {
        j = 0;
        flag_param_ok = 0;

        while (j < opt_size) {
            if ( strcmp(opt[j].key, argv[i]) == 0) {
                param_counter++;

                // flag --------------------------------------------------------
                if (opt[j].flag) {
                    *opt[j].flag = GET_OPT_FLAG_ENABLE;
                }
                if (opt[j].type == GET_OPT_VALUE_TYPE_FLAG) {
                    flag_param_ok = 1;// OK parameter Processed ->exit next get parameter.
                    break;
                }

                // int / str ---------------------------------------------------
                if (opt[j].type == GET_OPT_VALUE_TYPE_INT ||
                    opt[j].type == GET_OPT_VALUE_TYPE_STR) {

                    i++; // set index from KEY to VALUE
                    if (i >= argc) {
                        printf("ERROR: param(%s)-OK, value not defined.\n",
                               opt[j].key);
                        return -1;
                    }

                    // str -----------------------------------------------------
                    if (opt[j].value_str && opt[j].type == GET_OPT_VALUE_TYPE_STR) {
                        *opt[j].value_str = argv[i];
                        flag_param_ok = 1;// OK parameter Processed ->exit next get parameter.
                        break;
                    }

                    // int -----------------------------------------------------
                    if (opt[j].type == GET_OPT_VALUE_TYPE_INT) {
                        if (opt[j].value_int == 0) {
                            printf("FATAL ERROR: opt[%d]->value_int = NULL\n", j);
                            return -1;
                        }
                        res = conv_to_uint32(argv[i], opt[j].value_int);
                        if (res < 0) {
                            printf("ERROR: input parameter value: %s = %s\n",
                                   opt[j].key, *opt[j].value_str);
                            return -1;
                        }
                    } // if (opt[j]->type == GET_OPT_VALUE_TYPE_INT) {
                } // if (opt[j]->type == GET_OPT_VALUE_TYPE_xxx
                flag_param_ok = 1;// OK parameter Processed ->exit next get parameter.
                break;
            } // if (strcmp(opt[j]->key, argv[i]) == 0) {
            j++;
        } // while (j < opt_size) {

        if (flag_param_ok == 0) {
            printf("ERROR: input <%s> parameter NOT found.\n", argv[i]);
            return -1;
        }

        i++;

	} //for (int i=0; i<argc; i++){

    return param_counter;
}
