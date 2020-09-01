//
//  isax_index.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
//
//  Modified by Karima Echihabi on 09/08/17 to improve memory management.
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  

#ifndef isaxlib_isax_index_h
#define isaxlib_isax_index_h
#include "config.h"
#include "globals.h"
#include "isax_node.h"
#include "isax_node_record.h"
#include "sax/ts.h"
//#include "isax_query_engine.h"

typedef struct {
    const char* root_directory;
    int initial_fbl_buffer_size;
    
    // ALWAYS: TIMESERIES_SIZE = TS_VALUES_PER_PAA_SEGMENT * PAA_SEGMENTS
    int timeseries_size;
    int ts_values_per_paa_segment;
    int paa_segments;
    
    // ALWAYS: SAX_ALPHABET_CARDINALITY = 2^SAX_BIT_CARDINALITY
    int sax_bit_cardinality;
    root_mask_type * bit_masks;
    int sax_alphabet_cardinality;
    sax_type *max_sax_cardinalities;
  
    int sax_byte_size;
    int position_byte_size;
    int ts_byte_size;
    
    int full_record_size;
    int partial_record_size;

    
    int max_leaf_size;
    int initial_leaf_buffer_size;
    unsigned long long max_total_buffer_size;
    
    int max_filename_size;
    
    float mindist_sqrt;
    int root_nodes_size;
    
} isax_index_settings;

typedef struct {

    // TODO: FILL THIS THING IN!
    unsigned long long total_buffered_records;
    unsigned long root_nodes;
    unsigned long long total_records;
    struct isax_node *first_node;
    isax_index_settings *settings;
    
    struct first_buffer_layer *fbl;
    struct stats_info * stats;

    unsigned long long total_bytes;
  
} isax_index;

typedef struct {
    float distance;
    isax_node_record record;
} isax_index_result;




struct stats_info {

        double idx_building_input_time;  
        double idx_building_output_time;
        double idx_building_cpu_time;
        double idx_building_total_time;

        double idx_writing_input_time;  
        double idx_writing_output_time;
        double idx_writing_cpu_time;
        double idx_writing_total_time;

        double idx_reading_input_time;  
        double idx_reading_output_time;
        double idx_reading_cpu_time;
        double idx_reading_total_time;
  
        double idx_total_input_time;
        double idx_total_output_time;
        double idx_total_cpu_time;
        double idx_total_time;
  
        unsigned long long idx_building_seq_input_count;
        unsigned long long idx_building_seq_output_count;
        unsigned long long idx_building_rand_input_count;
        unsigned long long idx_building_rand_output_count;

        unsigned long long idx_writing_seq_input_count;
        unsigned long long idx_writing_seq_output_count;
        unsigned long long idx_writing_rand_input_count;
        unsigned long long idx_writing_rand_output_count;

        unsigned long long idx_reading_seq_input_count;
        unsigned long long idx_reading_seq_output_count;
        unsigned long long idx_reading_rand_input_count;
        unsigned long long idx_reading_rand_output_count;
  
        unsigned long long idx_total_seq_input_count;
        unsigned long long idx_total_seq_output_count;
        unsigned long long idx_total_rand_input_count;
        unsigned long long idx_total_rand_output_count;

        double query_filter_input_time;
        double query_filter_output_time;
        double query_filter_load_node_time;
        double query_filter_cpu_time;
        double query_filter_total_time;    

        double queries_filter_input_time;
        double queries_filter_output_time;
        double queries_filter_load_node_time;
        double queries_filter_cpu_time;
        double queries_filter_total_time;

        double query_refine_input_time;
        double query_refine_output_time;
        double query_refine_load_node_time;
        double query_refine_cpu_time;
        double query_refine_total_time;    

        double queries_refine_input_time;
        double queries_refine_output_time;
        double queries_refine_load_node_time;
        double queries_refine_cpu_time;
        double queries_refine_total_time;

        double query_total_input_time;
        double query_total_output_time;
        double query_total_load_node_time;
        double query_total_cpu_time;
        double query_total_time;    

        double queries_total_input_time;
        double queries_total_output_time;
        double queries_total_load_node_time;
        double queries_total_cpu_time;
        double queries_total_time;

        double total_input_time;
        double total_output_time;
        double total_load_node_time;
        double total_cpu_time;
        double total_time;    
        double total_time_sanity_check;
  
        unsigned long long query_refine_seq_input_count; //refine for refinement step
        unsigned long long query_refine_seq_output_count;
        unsigned long long query_refine_rand_input_count;
        unsigned long long query_refine_rand_output_count;

        unsigned long long queries_refine_seq_input_count;
        unsigned long long queries_refine_seq_output_count;
        unsigned long long queries_refine_rand_input_count;
        unsigned long long queries_refine_rand_output_count;

        unsigned long long query_filter_seq_input_count; //filtering step
        unsigned long long query_filter_seq_output_count;
        unsigned long long query_filter_rand_input_count;
        unsigned long long query_filter_rand_output_count;

        unsigned long long queries_filter_seq_input_count;
        unsigned long long queries_filter_seq_output_count;
        unsigned long long queries_filter_rand_input_count;
        unsigned long long queries_filter_rand_output_count;

        unsigned long long query_total_seq_input_count;
        unsigned long long query_total_seq_output_count;
        unsigned long long query_total_rand_input_count;
        unsigned long long query_total_rand_output_count;

        unsigned long long queries_total_seq_input_count;
        unsigned long long queries_total_seq_output_count;
        unsigned long long queries_total_rand_input_count;
        unsigned long long queries_total_rand_output_count;

        unsigned long long total_seq_input_count;
        unsigned long long total_seq_output_count;
        unsigned long long total_rand_input_count;
        unsigned long long total_rand_output_count;
  
        unsigned int total_nodes_count;
        unsigned int leaf_nodes_count;
        unsigned int empty_leaf_nodes_count;  
  
        long long idx_size_bytes;
        long long idx_size_blocks; 

        int *  leaves_sizes;
        double min_fill_factor;
        double max_fill_factor;
        double sum_fill_factor;
        double sum_squares_fill_factor;
        double avg_fill_factor;
        double sd_fill_factor;

        int *  leaves_heights;
        unsigned int min_height;
        unsigned int max_height;
        unsigned int sum_height;
        unsigned int sum_squares_height;  
        double avg_height;
        double sd_height;
  

        int leaves_counter;
   
        //file_position_type loaded_records_count;
  /*
        unsigned long long query_refine_ts_disk_count;
        unsigned long long query_refine_ts_mem_count;
        unsigned long long query_filter_ts_disk_count;
        unsigned long long query_filter_ts_mem_count;
  */
  
        unsigned int query_filter_loaded_nodes_count;
        unsigned int query_filter_checked_nodes_count;
	unsigned long long query_filter_loaded_ts_count;
        unsigned long long query_filter_checked_ts_count;

        unsigned int query_refine_loaded_nodes_count;
        unsigned int query_refine_checked_nodes_count;
	unsigned long long query_refine_loaded_ts_count;
        unsigned long long query_refine_checked_ts_count;

        unsigned int query_total_loaded_nodes_count;
        unsigned int query_total_checked_nodes_count;
	unsigned long long query_total_loaded_ts_count;
        unsigned long long query_total_checked_ts_count;
  
        unsigned long long total_ts_count;

        double query_approx_distance;
        char * query_approx_node_filename;  
        unsigned int query_approx_node_size;
        unsigned int query_approx_node_level;  

        double query_exact_distance;
        char * query_exact_node_filename;
        unsigned int query_exact_node_size;
        unsigned int query_exact_node_level;

        double query_lb_distance;  
  
        double query_eff_epsilon;
        double query_pruning_ratio;
        double query_tlb;
        double query_sum_tlb;
  
        double queries_min_pruning_ratio;
        double queries_max_pruning_ratio;
        double queries_sum_pruning_ratio;
        double queries_sum_squares_pruning_ratio;
        double queries_avg_pruning_ratio;
        double queries_sd_pruning_ratio;
  
        double queries_min_eff_epsilon;
        double queries_max_eff_epsilon;
        double queries_sum_eff_epsilon;
        double queries_sum_squares_eff_epsilon;
        double queries_avg_eff_epsilon;
        double queries_sd_eff_epsilon;

        double queries_min_tlb;
        double queries_max_tlb;
        double queries_sum_tlb;
        double queries_sum_squares_tlb;
        double queries_avg_tlb;
        double queries_sd_tlb;
  
        double queries_total_checked_ts_count;
        double queries_total_checked_nodes_count;  
        double queries_avg_checked_ts_count;
        double queries_avg_checked_nodes_count;
        double queries_avg_loaded_ts_count;
        double queries_avg_loaded_nodes_count;  
  
        unsigned long long tlb_ts_count; //this could be different from the total_ts_count. It includes only the ts for whiche d(Q,S) != 0
        unsigned long long eff_epsilon_queries_count; //the number of queries for which  d(Q,S) != 0

        double total_parse_time;
    
}; 

//TODO: Put sanity check for variables (cardinalities etc.)

isax_index * isax_index_init(isax_index_settings *settings);
isax_index_result * isax_index_search(isax_index *index, ts_type *ts);
isax_index_settings * isax_index_settings_init (const char * root_directory,
                                                int timeseries_size, 
                                                int paa_segments, 
                                                int sax_bit_cardinality,
                                                int max_leaf_size,
                                                int initial_leaf_buffer_size,
                                                unsigned long long  max_total_buffer_size, 
                                                int initial_fbl_buffer_size,
						int use_index);
enum response add_record_to_node(isax_index *index, isax_node *node, 
                                 isax_node_record *record,
                                 const char leaf_size_check);
enum response isax_fbl_index_insert(isax_index *index, isax_node_record *record);
enum response isax_index_insert(isax_index *index, isax_node_record *record);
enum response flush_subtree_leaf_buffers (isax_index *index, isax_node *node);
enum response flush_all_leaf_buffers(isax_index *index, const char clear_input_data);
enum response create_node_filename(isax_index *index,
                                   isax_node *node,
                                   isax_node_record *record);
void isax_index_clear_node_buffers(isax_index *index, isax_node *node, 
                                   const char clear_children,
                                   const char clear_input_data);
void isax_index_finalize(isax_index *index, isax_node *node, int *already_finalized);
void isax_index_destroy(isax_index *index, int use_index, isax_node *node);

ts_type calculate_node_distance (isax_index *index, isax_node *node,
				 ts_type *query_ts_reordered, int *query_order,
				 unsigned int offset, ts_type bsf,file_position_type *bsf_position);
//float calculate_node_distance (isax_index *index, isax_node *node, ts_type *query,
//                               file_position_type *bsf_position);
void isax_index_serialize_structure(isax_index *index, const char *filename);

isax_index * index_read(const char* root_directory);
void index_write(isax_index *index);
isax_node *node_read(isax_index *index, FILE *file);
void node_write(isax_index *index, isax_node *node, FILE *file);



void get_index_footprint(isax_index * index);

void isax_index_collect_stats(isax_index *index, isax_node *node);


void isax_index_collect_subtree_stats(isax_index *index, isax_node *node);
void update_index_stats(isax_index *index, isax_node *node);
void init_stats(isax_index * index);
//void get_query_stats(isax_index * index, ts_type * ts, ts_type * paa, int query_num);

void print_index_stats(isax_index * index, char * dataset);
//void print_query_stats(isax_index * index, unsigned int query_num, char * queries);

void get_queries_stats(isax_index * index, unsigned int queries_count);
void print_tlb_stats(isax_index * index, unsigned int query_num, char * queries);

#endif
