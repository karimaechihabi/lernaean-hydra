//
//  mass_query_engine.c
//
//  Created by Karima Echihabi on 03/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//
//  Based on code by Michele Linardi on 01/01/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.


#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/mass_query_engine.h"
#include "../include/mass_buffer_manager.h"
#include "../include/mass_instance.h"

#ifdef VALUES
#include <values.h>
#endif


enum response update_bsf_distance(ts_type * qt, ts_type * query_mean_std, ts_type * series_mean_std, int idx_q,
				  unsigned long long series_size, unsigned long long query_size, boolean b_self_join,
				  ts_type *series_ts,
				  ts_type *query_ts,
				  query_result * bsf)
{
	unsigned long long i;

	for(i=0;i<(series_size-query_size+1);i++)
	{
		if(idx_q<(i-(query_size/2)) || idx_q>(i+(query_size/2)) || !b_self_join) // avoid the trivial matches
		{
		  ts_type dist= (2*query_size) * (1- ( (qt[i] - (query_size*query_mean_std[0]*series_mean_std[i]))
			  / (query_size*query_mean_std[1]*series_mean_std[i+(series_size-query_size+1)]) )) ;
		
			if(dist < bsf->distance)
			{
			  bsf->distance = dist;
			  bsf->offset = i;
			}
		}
	}


	return SUCCESS;
}


