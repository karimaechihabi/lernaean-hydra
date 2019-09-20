#ifndef sfalib_sfa_trie_h
#define sfalib_sfa_trie_h

#include "../config.h"
#include "../globals.h"
#include "calc_utils.h"
#include "sfa_node.h"
#include "ts.h"


struct sfa_trie_settings {
    const char* root_directory;  
    unsigned int timeseries_size;
    unsigned int max_leaf_size;    
    unsigned int max_filename_size;
    unsigned int fft_size;  
    unsigned int num_symbols;
    double buffered_memory_size;  
    boolean is_norm;
    ts_type norm_factor;  
    unsigned int start_offset;
    unsigned int histogram_type;
    unsigned int lb_dist;  
    unsigned long sample_size;
    unsigned int minimal_depth;
};

struct sfa_record {
  ts_type * timeseries;
  ts_type * dft_transform;
  unsigned char * sfa_word;
};

struct stats_info {


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


struct  sfa_trie{
    unsigned long long total_records;
    struct sfa_node *first_node;
    //struct sfa_node first_node;
    struct sfa_trie_settings *settings;
    struct sfa_file_buffer_manager * buffer_manager;
    struct stats_info * stats;
    ts_type ** bins;  
};

struct sfa_trie * sfa_trie_init(struct sfa_trie_settings *settings);
struct sfa_trie * sfa_trie_read(const char* root_directory); 
enum response sfa_trie_write(struct sfa_trie *trie);
struct sfa_node * sfa_node_read(struct sfa_trie *trie, FILE *file);
enum response sfa_node_write(struct sfa_trie *trie, struct sfa_node *node, FILE *file);

void sfa_trie_insert(struct sfa_trie * trie,
		     //		     struct sfa_node * node,
		     int index,		     
		     struct sfa_record * record
		     );

struct sfa_trie_settings * sfa_trie_settings_init(const char * root_directory,
						 unsigned int timeseries_size, 
                                                 unsigned int fft_size, 
                                                 unsigned int num_symbols,
						 unsigned int max_leaf_size,
						 double buffered_memory_size,
						 boolean is_norm,
	            				 unsigned int  histogram_type,
	            				 unsigned int  lb_dist,						  
						 unsigned int sample_size,
					         boolean is_trie_new);

void sfa_trie_destroy(struct sfa_trie *trie, struct sfa_node *node, boolean is_trie_new);
void destroy_buffer_manager(struct sfa_trie *trie);
enum response append_ts_to_buffered_list(struct sfa_trie * trie, struct sfa_node * node, ts_type * timeseries);
void sfa_print_stats (struct sfa_trie *trie);
enum response sfa_update_trie_stats(struct sfa_trie *trie, struct sfa_node *node);
enum response sfa_init_stats(struct sfa_trie * trie);
enum response sfa_trie_bins_init(struct sfa_trie * trie);
void sfa_trie_set_bins(struct sfa_trie *trie, char * ifilename);
void print_tlb_stats(struct sfa_trie * trie, unsigned int query_num, char * queries);

#endif
