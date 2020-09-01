#ifndef dstreelib_ts_h
#define dstreelib_ts_h
#include "../config.h"
#include "../globals.h"
#include <math.h>

enum response ts_parse_str(char ts_str[], ts_type *ts_out, int ts_size, const char * delims);
void ts_print(ts_type *ts, int size);
float ts_euclidean_distance(ts_type * t, ts_type * s, int size);
ts_type ts_euclidean_distance_reordered(ts_type * q, ts_type * t , int j , int  size ,ts_type bsf, int * order);

#endif

