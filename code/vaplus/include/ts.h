#ifndef sfaib_ts_h
#define sfalib_ts_h
#include "../config.h"
#include "../globals.h"
#include <math.h>

enum response ts_parse_str(char ts_str[], ts_type *ts_out, int ts_size, const char * delims);
void ts_print(ts_type *ts, int size);
ts_type ts_euclidean_distance_non_opt(ts_type * t, ts_type * s, unsigned int size);
ts_type ts_euclidean_distance_reordered(ts_type * q, ts_type * t , unsigned int j , unsigned int  size ,ts_type bsf, int * order);
ts_type ts_euclidean_distance(ts_type * q, ts_type * t, unsigned int size, ts_type bsf);




#endif

