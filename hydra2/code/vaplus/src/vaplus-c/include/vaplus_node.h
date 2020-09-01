//
//  vaplus_node.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef vapluslib_vaplus_node_h
#define vapluslib_vaplus_node_h


#include "../config.h"
#include "../globals.h"    
//#include "vaplus_node_split.h"
#include "vaplus_file_buffer.h"


struct vaplus_node {
    /*    
    struct node_segment_split_policy * node_segment_split_policies;
    short * node_points;
    short * hs_node_points;
    struct segment_sketch * node_segment_sketches;
    struct segment_sketch * hs_node_segment_sketches;
    struct node_split_policy * split_policy; 

    struct vaplus_node *left_child;
    struct vaplus_node *right_child;

    mean_stdev_range range;  
 
    int num_node_segment_split_policies;  
    short num_node_points;  //number of vertical split points
    short num_hs_node_points;  //number of horizontal split points

    int max_segment_length; 
    int max_value_length; 
  
    boolean is_left;
    */
    /*VAPLUS*/

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


/*
enum response node_init_segments(struct vaplus_node * node, short * split_points, int segment_size);

enum response append_ts_to_node(struct vaplus_index * index, struct vaplus_node * node, ts_type * timeseries);  
//enum response update_node_statistics(struct vaplus_node * node, ts_type * timeseries);
*/

enum response vaplus_node_append_record(struct vaplus_index * index, struct vaplus_node * node, struct vaplus_record* record);

enum response vaplus_node_create_filename(struct vaplus_index_settings *settings,
				       struct vaplus_node * node,
				       unsigned int index);


enum response vaplus_node_update_statistics(struct vaplus_index * index, struct vaplus_node * node, ts_type * dft_transform);

//ts_type calculate_ts_in_node_distance (struct vaplus_index *index, struct vaplus_node *node, ts_type *query);
//ts_type calculate_node_distance (struct vaplus_index *index, struct vaplus_node *node, ts_type *query);
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
