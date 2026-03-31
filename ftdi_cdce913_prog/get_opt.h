#ifndef GET_OPT_H_
#define GET_OPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define LEN_STRING_DEC_MAX   (10)
#define LEN_STRING_HEX_MAX   (10)


// type of parameter value
typedef enum {
    GET_OPT_VALUE_TYPE_INT  = 0,  // int32
    GET_OPT_VALUE_TYPE_STR  = 1,  // string
    GET_OPT_VALUE_TYPE_FLAG = 2   // flag
} get_opt_value_type_t;

// flag parameter enable/disable
typedef enum {
    GET_OPT_FLAG_DISABLE = 0,
    GET_OPT_FLAG_ENABLE  = 1
} get_opt_flag_t;

// Structure: Key-Type-Value
// {"key", type, flag, int, str}
typedef struct {
    const char                 *key;
    const get_opt_value_type_t type;
    get_opt_flag_t             *flag;
    uint32_t                   *value_int;
    char                       **value_str;
} get_opt_t;

typedef enum {
    GTS_ERROR,
    GTS_DEC,
    GTS_HEX
} type_string_e;


int get_opt(const int argc, char** argv, get_opt_t *opt, const uint32_t opt_size);

#ifdef __cplusplus
}
#endif


#endif
