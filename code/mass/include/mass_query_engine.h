#ifndef masslib_mass_query_engine_h
#define masslib_mass_query_engine_h
#include "../config.h"
#include "../globals.h"

#include "mass_instance.h"

typedef struct query_result {
    ts_type distance;
    unsigned long long offset;
    ts_type * series;
} query_result;

typedef struct index_distance_profile{	
	double best_dist;
	int best_idx;
	double * dist_profile;
	int * idx_profile;
	int length;
} id_prof;


enum response update_bsf_distance(ts_type * qt, ts_type * mean_std_query, ts_type * mean_std_series, int idx_q,
				    unsigned long long series_size, unsigned long long query_size, boolean b_self_join,
				  ts_type * series_ts,
				  ts_type * query_ts,				  
				  query_result * bsf);
#endif
