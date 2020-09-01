#ifndef masslib_mass_calc_utils_h
#define masslib_mass_calc_utils_h

#include "../config.h"
#include "../globals.h"
#include "math.h"



enum response mass_compute_mean_std(ts_type * series, unsigned long long series_size, unsigned long long query_size, ts_type * mean_std);


enum response sliding_dot_product(ts_type * series_ts, ts_type * query_ts, unsigned long long series_size,
			      unsigned long long query_size, fftwf_complex * out, fftwf_complex * out1, fftwf_complex * out3,
			      ts_type * final_dot_product_dp,
				  ts_type * output_dp,
				  fftwf_plan plan_backward);

enum response reverse_signal(ts_type * query_ts, unsigned long long n);

#endif
