#ifndef STATS_H
#define STATS_H

//#include <string>
//#include "../isax_index.h"
//#include "../isax_query_engine.h"
#include "../globals.h"

//using namespace std;
#include <time.h>
#include <sys/time.h>

extern struct stats_info stats;

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

  int *  leaves_heights;  


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
  
  
  unsigned long long tlb_ts_count; //this could be different from the total_ts_count. It includes only the ts for whiche d(Q,S) != 0
  unsigned long long eff_epsilon_queries_count; //the number of queries for which  d(Q,S) != 0

  double total_parse_time;
    
}; 

void get_query_stats(ts_type * ts, ts_type * paa, int query_num);
void print_query_stats(unsigned int query_num, char * queries);
void print_index_stats(char * dataset, unsigned int max_leaf_size);
void get_index_footprint();

void stats_init(void);
void stats_update_idx_reading_stats(void);
void stats_update_idx_writing_stats(void);
void stats_update_idx_building_stats(void);
void stats_update_knn_query_exact_answer(unsigned int query_id,
					 unsigned int found_knn,
					 ts_type distance);
void stats_update_knn_query_approx_answer(ts_type distance, boolean user_bsf);
void stats_update_query_exact_answer(ts_type distance);  
void stats_update_query_approx_answer(ts_type distance, boolean user_bsf);


#endif

