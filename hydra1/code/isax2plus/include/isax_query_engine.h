//
//  isax_query_engine.h
//  al_isax
//
//  Created by Kostas Zoumpatianos on 4/13/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef al_isax_isax_query_engine_h
#define al_isax_isax_query_engine_h
#include "config.h"
#include "globals.h"
#include "isax_index.h"
#include "isax_node.h"

typedef struct query_result {
    float distance;
    file_position_type raw_file_position;
    isax_node *node;
    size_t pqueue_position;
} query_result;

/// Data structure for sorting the query.
typedef struct q_index
    {   double value;
        int    index;
    } q_index;


static int
cmp_pri(double next, double curr)
{
	return (next > curr);
}


static double
get_pri(void *a)
{
	return (double) ((query_result *) a)->distance;
}


static void
set_pri(void *a, double pri)
{
	((query_result *) a)->distance = (float)pri;
}


static size_t
get_pos(void *a)
{
	return ((query_result *) a)->pqueue_position;
}


static void
set_pos(void *a, size_t pos)
{
	((query_result *) a)->pqueue_position = pos;
}

query_result approximate_search (ts_type *query_ts, ts_type *query_paa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf,isax_index *index);
query_result exact_search (ts_type *query_ts, ts_type *query_paa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, isax_index *index,ts_type minimum_distance);
void isax2plus_calc_tlb (ts_type *query_ts, ts_type * query_paa, isax_index *index, isax_node * curr_node);

  //query_result approximate_search (ts_type *ts, ts_type *paa, isax_index *index);
  //query_result exact_search (ts_type *ts, ts_type *paa, isax_index *index,
  //					   float minimum_distance);
#endif
