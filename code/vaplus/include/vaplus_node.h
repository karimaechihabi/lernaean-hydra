#ifndef vapluslib_vaplus_node_h
#define vapluslib_vaplus_node_h


#include "../config.h"
#include "../globals.h"    
#include "vaplus_file_buffer.h"


struct vaplus_node {
    struct vaplus_node *children;
    struct vaplus_node *next;
  
    struct vaplus_node *parent;

    struct vaplus_file_buffer * file_buffer;

    char * filename;

    unsigned char * vaplus_word;
 
    ts_type * min_values;
    ts_type * max_values;
  
    unsigned int prefix_length;
  
    unsigned int node_size;
    unsigned int level;

    boolean is_leaf;
  
};

struct vaplus_node  * vaplus_root_node_init();
struct vaplus_node  * vaplus_leaf_node_init();



enum response vaplus_node_append_record(struct vaplus_index * index, struct vaplus_node * node, struct vaplus_record* record);

enum response vaplus_node_create_filename(struct vaplus_index_settings *settings,
				       struct vaplus_node * node,
				       unsigned int index);


enum response vaplus_node_update_statistics(struct vaplus_index * index, struct vaplus_node * node, ts_type * dft_transform);

ts_type calculate_node_min_distance (struct vaplus_index *index, struct vaplus_node *node, ts_type *query, ts_type *query_fft);

ts_type calculate_ts_in_node_distance (struct vaplus_index *index,
				       struct vaplus_node *node,
				       ts_type *query_ts_reordered,
				       int * query_order,
				       unsigned int offset,
				       ts_type bound);

ts_type calculate_node_distance (struct vaplus_index *index, struct vaplus_node *node, ts_type *query_ts_reordered, int *query_order, unsigned int offset, ts_type bsf);
//ts_type calculate_node_distance (struct vaplus_index *index, struct vaplus_node *node, ts_type *query_ts, ts_type *query_ts_reordered, int *query_order, unsigned int offset, ts_type bsf);


ts_type mindist_fft_to_approx (struct vaplus_index *index, unsigned int * cand_approx, ts_type *query_fft,
					 unsigned int * query_approx);

ts_type maxdist_fft_to_approx (struct vaplus_index *index, unsigned int * cand_approx, ts_type *query_fft,
					 unsigned int * query_approx);

ts_type mindist_fft_to_cell (struct vaplus_index * index, unsigned int cand_approx, unsigned int q_approx, ts_type q_fft, unsigned int dim);

ts_type maxdist_fft_to_cell (struct vaplus_index * index, unsigned int cand_approx, unsigned int q_approx, ts_type q_fft, unsigned int dim);

#endif
