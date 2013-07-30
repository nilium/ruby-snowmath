#include "maths_local.h"
#define __SNOW__MATHS_C__

s_float_t S_FLOAT_EPSILON = s_float_lit(
                            #ifdef USE_FLOAT
                            1.0e-6
                            #else
                            1.0e-9
                            #endif
                            );
