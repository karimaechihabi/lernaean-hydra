#ifndef al_vaplus_vaplus_query_engine_h
#define al_vaplus_vaplus_query_engine_h
#include "../config.h"
#include "../globals.h"

#include "vaplus_index.h"
#include "vaplus_node.h"

typedef struct query_result {
    ts_type lb_distance;
    ts_type ed_distance;
    struct vaplus_node *node;
    size_t pqueue_position;
    unsigned int file_position; //change to file_position_type unsigned long long
};

typedef struct q_index
{   double value;
        int    index;
};


/* The item in the queue with the lowest lb distance has higher priority
   Thus next < curr
*/

static int cmp_pri(double next, double curr)
{
	return (next < curr);
}


/*The priority is the lb distance*/

static double
get_pri(void *a)
{
	return (double) ((struct query_result *) a)->lb_distance;
}


static void
set_pri(void *a, double pri)
{
	((struct query_result *) a)->lb_distance = (float)pri;
}


static size_t
get_pos(void *a)
{
	return ((struct query_result *) a)->pqueue_position;
}


static void
set_pos(void *a, size_t pos)
{
	((struct query_result *) a)->pqueue_position = pos;
}



struct query_result approximate_search (ts_type *query_ts, unsigned char * query_vaplus, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf,struct vaplus_index *index);

struct query_result exact_search (ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx, ts_type * query_ts_reordered, int * query_order, unsigned int offset, struct vaplus_index *index,ts_type minimum_distance);

void vaplus_calc_tlb_leaf (ts_type *query_ts, ts_type *query_fft,
			unsigned char * query_vaplus, struct vaplus_index *index, struct vaplus_node * curr_node);

void vaplus_calc_tlb(ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx, struct vaplus_index *index);

#endif
