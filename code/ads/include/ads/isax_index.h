//
//  isax_index.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isaxlib_isax_index_h
#define isaxlib_isax_index_h
#include "../../config.h"
#include "../../globals.h"
#include "isax_node.h"
#include "isax_node_record.h"
#include "sax/ts.h"

typedef struct {
    unsigned long mem_tree_structure;
    unsigned long mem_data; 
    unsigned long mem_summaries;
    unsigned long disk_data_full;
    unsigned long disk_data_partial;
} meminfo;

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



typedef struct {
    char new_index;

    char * raw_filename;
    const char* root_directory;
    int initial_fbl_buffer_size;
	sax_type *max_sax_cardinalities;
    
    // ALWAYS: TIMESERIES_SIZE = TS_VALUES_PER_PAA_SEGMENT * PAA_SEGMENTS
    int timeseries_size;
    int ts_values_per_paa_segment;
    int paa_segments;
	
	int tight_bound;
	int aggressive_check;
    
    int sax_byte_size;
    int position_byte_size;
    int ts_byte_size;
    
    int full_record_size;
    int partial_record_size;
    
    // ALWAYS: SAX_ALPHABET_CARDINALITY = 2^SAX_BIT_CARDINALITY
    int sax_bit_cardinality;
    root_mask_type * bit_masks;
    int sax_alphabet_cardinality;
    
    int max_leaf_size;
    int min_leaf_size;
    int initial_leaf_buffer_size;
    unsigned long long max_total_buffer_size;
    unsigned long long max_total_full_buffer_size;
    
    int max_filename_size;
    
    float mindist_sqrt;
    int root_nodes_size;
    
    int total_loaded_leaves;
    
} isax_index_settings;

typedef struct {
    meminfo memory_info;

    FILE *sax_file;
    sax_type *sax_cache;
    unsigned long sax_cache_size;

    unsigned long long allocated_memory;
    unsigned long root_nodes;
    unsigned long long total_records;
    unsigned long long loaded_records;
    
    int * locations;
    struct isax_node *first_node;
    isax_index_settings *settings;
    
    char has_wedges;

    struct first_buffer_layer *fbl;

    ts_type *answer;

    struct stats_info * stats;
} isax_index;




//TODO: Put sanity check for variables (cardinalities etc.)

isax_index * isax_index_init(isax_index_settings *settings);
isax_index_settings * isax_index_settings_init (const char * root_directory,
                                                int timeseries_size, 
                                                int paa_segments, 
                                                int sax_bit_cardinality,
                                                int max_leaf_size,
                                                int min_leaf_size,
                                                int initial_leaf_buffer_size,
                                                unsigned long long max_total_buffer_size, 
                                                int initial_fbl_buffer_size,
                                                int total_loaded_leaves,
                                                int tight_bound, 
                                                int aggressive_check, 
                                                int new_index);
void print_settings(isax_index_settings *settings);

isax_node * add_record_to_node(isax_index *index, isax_node *node, 
                                 isax_node_record *record,
                                 const char leaf_size_check);
root_mask_type isax_fbl_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos);
enum response isax_index_insert(isax_index *index, sax_type *sax, 
                                file_position_type *pos);
enum response flush_subtree_leaf_buffers (isax_index *index, isax_node *node);
enum response flush_all_leaf_buffers(isax_index *index, enum buffer_cleaning_mode buffer_clean_mode);
enum response create_node_filename(isax_index *index,
                                   isax_node *node,
                                   isax_node_record *record);
void isax_index_clear_node_buffers(isax_index *index, isax_node *node, 
                                   enum node_cleaning_mode node_cleaning_mode,
                                   enum buffer_cleaning_mode buffer_clean_mode);
void isax_index_destroy(isax_index *index, isax_node *node);
int comp(const void * a, const void * b);
void load_random_leaf(isax_index *index);
ts_type calculate_node_distance (isax_index *index, isax_node *node,
				 ts_type *query_ts_reordered, int *query_order,
				 unsigned int offset, ts_type bsf);

//float calculate_node_distance (isax_index *index, isax_node *node, ts_type *query, float bsf);
//float isax_index_load_node(isax_index *index, isax_node *c_node, ts_type * query, float bsf);
float isax_index_load_node(isax_index *index, isax_node *c_node, ts_type * query_ts_reordered, int offset,float bsf, int * query_order);

void complete_index(isax_index *index, int ts_num);
void complete_index_leafs(isax_index *index);
void complete_subtree_leafs(isax_index *index, isax_node *node);

int find_siblings(isax_node *c_node, isax_node **nodes_to_load, 
                  int *number_of_leaves, int *offset);
float calculate_minimum_distance (isax_index *index, isax_node *node, ts_type *raw_query, ts_type *query);
void cache_sax_file(isax_index *index);

void dump_query_answer(isax_index *index, unsigned int query_num);



void index_write(isax_index *index);
void node_write(isax_index *index, isax_node *node, FILE *file);
isax_index * index_read(const char * root_directory);
isax_node * node_read(isax_index *index, FILE *file);

//// WEDGES FUNCTIONALITY ////
void create_wedges(isax_index *index, isax_node *node);
void clear_wedges(isax_index *index, isax_node *node);
int compare_file_positions (const void * a, const void * b);
void print_mem_info(isax_index *index);
meminfo get_memory_utilization_info(isax_index *index);
#endif
