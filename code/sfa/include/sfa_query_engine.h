#ifndef al_sfa_sfa_query_engine_h
#define al_sfa_sfa_query_engine_h
#include "../config.h"
#include "../globals.h"

#include "sfa_trie.h"
#include "sfa_node.h"

typedef struct query_result {
    ts_type distance;
    struct sfa_node *node;
    size_t pqueue_position;
};

/// Data structure for sorting the query.
typedef struct q_index
{   double value;
        int    index;
};

static int cmp_pri(double next, double curr)
{
	return (next > curr);
}


static double
get_pri(void *a)
{
	return (double) ((struct query_result *) a)->distance;
}


static void
set_pri(void *a, double pri)
{
	((struct query_result *) a)->distance = (float)pri;
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

struct query_result approximate_search (ts_type *query_ts, unsigned char * query_sfa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf,struct sfa_trie *trie);

  struct query_result exact_search (ts_type *query_ts, ts_type *query_fft, unsigned char * query_sfa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, struct sfa_trie *trie,ts_type minimum_distance);


void sfa_calc_tlb_leaf (ts_type *query_ts, ts_type *query_fft,
			unsigned char * query_sfa, struct sfa_trie *trie, struct sfa_node * curr_node);
void sfa_calc_tlb (ts_type *query_ts, ts_type *query_fft,
		     unsigned char * query_sfa, struct sfa_trie *sfa, struct sfa_node * curr_node);


#endif
