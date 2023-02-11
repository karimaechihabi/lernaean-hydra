//
//  dstree_index.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#ifndef dstreelib_dstree_index_h
#define dstreelib_dstree_index_h
#include "../config.h"
#include "../globals.h"
#include "calc_utils.h"
#include "dstree_node.h"
#include "ts.h"


struct dstree_index_settings {
    const char* root_directory;  
    double buffered_memory_size;
    unsigned int timeseries_size;
    unsigned int init_segments;    
    unsigned int max_leaf_size;    
    unsigned int max_filename_size;
};

struct stats_info {


	double idx_traverse_tree_total_time ;	
	double idx_traverse_tree_input_time;
	double idx_traverse_tree_output_time;
	double idx_traverse_tree_cpu_time;

	unsigned long idx_traverse_tree_seq_input_count;
	unsigned long idx_traverse_tree_seq_output_count;
	unsigned long idx_traverse_tree_rand_input_count ;
	unsigned long idx_traverse_tree_rand_output_count ;

	double idx_append_ts_to_leaf_total_time ;	
	double idx_append_ts_to_leaf_input_time;
	double idx_append_ts_to_leaf_output_time;
	double idx_append_ts_to_leaf_cpu_time;

	unsigned long idx_append_ts_to_leaf_seq_input_count;
	unsigned long idx_append_ts_to_leaf_seq_output_count;
	unsigned long idx_append_ts_to_leaf_rand_input_count ;
	unsigned long idx_append_ts_to_leaf_rand_output_count ;

	double idx_evaluate_split_policies_total_time ;	
	double idx_evaluate_split_policies_input_time;
	double idx_evaluate_split_policies_output_time;
	double idx_evaluate_split_policies_cpu_time;

	unsigned long idx_evaluate_split_policies_seq_input_count;
	unsigned long idx_evaluate_split_policies_seq_output_count;
	unsigned long idx_evaluate_split_policies_rand_input_count ;
	unsigned long idx_evaluate_split_policies_rand_output_count ;

	double idx_split_node_total_time ;	
	double idx_split_node_input_time;
	double idx_split_node_output_time;
	double idx_split_node_cpu_time;

	unsigned long idx_split_node_seq_input_count;
	unsigned long idx_split_node_seq_output_count;
	unsigned long idx_split_node_rand_input_count ;
	unsigned long idx_split_node_rand_output_count ;

        double idx_building_input_time;  
        double idx_building_output_time;
        double idx_building_cpu_time;
        double idx_building_total_time;

        unsigned long long idx_building_seq_input_count;
        unsigned long long idx_building_seq_output_count;
        unsigned long long idx_building_rand_input_count;
        unsigned long long idx_building_rand_output_count;

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
  
        unsigned int query_loaded_nodes_count;
        unsigned int query_checked_nodes_count;
	unsigned long long query_loaded_ts_count;
        unsigned long long query_checked_ts_count;
  
        unsigned long long total_ts_count;


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
        double queries_total_loaded_ts_count;
        double queries_total_loaded_nodes_count;  
        double queries_avg_loaded_ts_count;
        double queries_avg_loaded_nodes_count;

        unsigned long long tlb_ts_count; //this could be different from the total_ts_count. It includes only the ts for whiche d(Q,S) != 0
        unsigned long long eff_epsilon_queries_count; //the number of queries for which  d(Q,S) != 0

  
        unsigned int total_queries_count; //this could be different from the input queries to eliminate cases d(Q,S) = 0

        double total_parse_time;
 };


struct  dstree_index{
    unsigned long long total_records;
    struct dstree_node *first_node;
    struct dstree_index_settings *settings;
    struct dstree_file_buffer_manager * buffer_manager;
    struct stats_info * stats;
};
struct dstree_index * dstree_index_init(struct dstree_index_settings *settings);
struct dstree_index * dstree_index_read(const char* root_directory); 
enum response dstree_index_write(struct dstree_index *index);
struct dstree_node * dstree_node_read(struct dstree_index *index, FILE *file);
enum response dstree_node_write(struct dstree_index *index, struct dstree_node *node, FILE *file);


struct dstree_index_settings * dstree_index_settings_init(const char * root_directory,
  						 unsigned int timeseries_size, 
                                                 unsigned int init_segments, 
                                                 unsigned int max_leaf_size,
						 double buffered_memory_size,
						 boolean is_index_new);
void dstree_index_destroy(struct dstree_index *index, struct dstree_node *node, boolean is_index_new);
void destroy_buffer_manager(struct dstree_index *index);
enum response dstree_index_insert(struct dstree_index *index,  ts_type * timeseries);
enum response append_ts_to_buffered_list(struct dstree_index * index, struct dstree_node * node, ts_type * timeseries);

void dstree_print_stats (struct dstree_index *index);
enum response dstree_update_index_stats(struct dstree_index *index, struct dstree_node *node);
enum response dstree_init_stats(struct dstree_index * index);
void print_tlb_stats(struct dstree_index * index, unsigned int query_num, char * queries);

  
//struct dstree_ts_buffer * copy_ts_in_mem(struct dstree_index * index, struct dstree_node * node);

#endif
