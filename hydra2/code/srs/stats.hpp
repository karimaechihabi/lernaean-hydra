#ifndef STATS_HPP
#define STATS_HPP

#include <string>
#include "stats.hpp"

using namespace std;
#include <time.h>
#include <sys/time.h>

typedef float ts_type;
typedef unsigned long long file_position_type;
typedef bool boolean;

extern struct stats_info stats;

struct stats_counters {
        double tS;
        double tE;

        struct timeval total_time_start;
        struct timeval parse_time_start;
        struct timeval input_time_start;
        struct timeval output_time_start;
        struct timeval current_time;
        struct timeval fetch_start;
        struct timeval fetch_check_start;
        double total_input_time;
        double total_output_time;
        double total_parse_time;
        double total_time;


        struct timeval partial_time_start;
        struct timeval partial_input_time_start;
        struct timeval partial_output_time_start;

        double partial_time;
        double partial_input_time;
        double partial_output_time;

        unsigned long long partial_seq_input_count;
        unsigned long long partial_seq_output_count;
        unsigned long long partial_rand_input_count;
        unsigned long long partial_rand_output_count;

        unsigned long checked_ts_count;
        unsigned long total_ts_count;
        unsigned long total_queries_count;


};


struct stats_info {

        struct stats_counters counters;
  
        double idx_building_total_input_time;  
        double idx_building_total_output_time;
        double idx_building_total_cpu_time;
        double idx_building_total_time;

        unsigned long long idx_building_total_seq_input_count;
        unsigned long long idx_building_total_seq_output_count;
        unsigned long long idx_building_total_rand_input_count;
        unsigned long long idx_building_total_rand_output_count;

        double query_total_input_time;
        double query_total_output_time;
        double query_total_cpu_time;
        double query_total_time;    

        unsigned long long query_total_seq_input_count;
        unsigned long long query_total_seq_output_count;
        unsigned long long query_total_rand_input_count;
        unsigned long long query_total_rand_output_count;

        double query_exact_distance;
        double query_exact_distance_sanity_check;
        unsigned long long  query_exact_offset;
        unsigned long long query_checked_ts_count;
        double query_pruning_ratio;
  
  
        double total_input_time;
        double total_output_time;
        double total_cpu_time;
        double total_time;    
        double total_time_sanity_check;
  
        unsigned long long total_seq_input_count;
        unsigned long long total_seq_output_count;
        unsigned long long total_rand_input_count;
        unsigned long long total_rand_output_count;
  
        unsigned long long total_ts_count;  
        unsigned int total_queries_count; //this could be different from the input queries to eliminate cases d(Q,S) = 0
        unsigned long long program_byte_size;
        unsigned long long dataset_byte_size;

        long long idx_size_bytes;
        long long idx_size_blocks;
  
};

/*
inline void stats_init(void);
inline void stats_init_counters (void );
inline void stats_reset_query_counters(void);
inline void stats_reset_partial_counters(void);
inline void stats_count_partial_seq_input(void);
inline void stats_count_partial_seq_output(void);
inline void stats_count_partial_rand_input(void);
inline void stats_count_partial_rand_output(void);
inline void stats_count_input_time_start(void);
inline void stats_count_output_time_start(void);
inline void stats_count_total_time_start(void);
inline void stats_count_partial_input_time_start(void);
inline void stats_count_partial_output_time_start(void);
inline void stats_count_partial_time_start(void);
inline void stats_count_input_time_end(void);
inline void stats_count_output_time_end(void);
inline void stats_count_total_time_end(void);
inline void stats_count_partial_input_time_end(void);
inline void stats_count_partial_output_time_end(void);
inline void stats_count_partial_time_end(void);
inline void stats_update_idx_building_stats(void);
inline void stats_update_query_stats(void);

*/

void get_index_stats(struct stats_info * stats, char * filename);
void print_query_stats(struct stats_info stats, char * queries, unsigned int query_num, unsigned int found_knn);
void print_index_stats(struct stats_info  stats, char * dataset, unsigned int max_leaf_size);
void get_index_footprint(struct stats_info * stats, char * index_filename);
void get_query_stats(struct stats_info * stats);
  
inline void stats_init_counters (void )
{
   stats.counters.total_input_time = 0;
   stats.counters.total_output_time = 0;
   stats.counters.total_time = 0;
   stats.counters.partial_time = 0;
   stats.counters.partial_input_time = 0;	
   stats.counters.partial_output_time = 0;
   stats.counters.partial_seq_input_count = 0;
   stats.counters.partial_seq_output_count = 0;
   stats.counters.partial_rand_input_count = 0;
   stats.counters.partial_rand_output_count = 0;
   stats.counters.checked_ts_count = 0;
   stats.counters.total_ts_count = 0;
}

inline void stats_init(void)
{
 
    /*IDX_BUILDING_TOTAL STATISTICS*/
    stats.idx_building_total_input_time = 0;  
    stats.idx_building_total_output_time = 0;
    stats.idx_building_total_cpu_time = 0;
    stats.idx_building_total_time = 0;

    stats.idx_building_total_seq_input_count = 0;
    stats.idx_building_total_seq_output_count = 0;
    stats.idx_building_total_rand_input_count = 0;
    stats.idx_building_total_rand_output_count = 0;    
    
    /*PER QUERY STATISTICS*/    
    stats.query_total_input_time = 0;
    stats.query_total_output_time = 0;
    stats.query_total_cpu_time = 0;
    stats.query_total_time = 0;    

    stats.query_total_seq_input_count = 0;
    stats.query_total_seq_output_count = 0;
    stats.query_total_rand_input_count = 0;
    stats.query_total_rand_output_count = 0;
    
    stats.query_exact_distance = 0;
    stats.query_exact_distance_sanity_check = 0;
    stats.query_exact_offset = 0;
    stats.query_checked_ts_count = 0;
    
    
    /*COMBINED STATISTICS FOR IDX_BUILDING AND QUERY WORKLOAD*/            
    stats.total_input_time = 0;
    stats.total_output_time = 0;
    stats.total_cpu_time = 0;
    stats.total_time = 0;
    stats.total_time_sanity_check = 0;
    
    stats.total_seq_input_count = 0;
    stats.total_seq_output_count = 0;
    stats.total_rand_input_count = 0;
    stats.total_rand_output_count = 0;
    
    stats.total_ts_count = 0;
    
    stats.total_queries_count = 0;
    stats.program_byte_size = 0;
    stats.dataset_byte_size = 0;
    
    stats_init_counters();
    
}

inline void stats_reset_query_counters(void)
{
   stats.counters.checked_ts_count = 0;
}

inline void stats_reset_partial_counters(void)
{
   stats.counters.partial_seq_output_count = 0;
   stats.counters.partial_seq_input_count = 0;
   stats.counters.partial_rand_output_count = 0;
   stats.counters.partial_rand_input_count = 0;
   stats.counters.partial_input_time = 0;
   stats.counters.partial_output_time = 0;
   stats.counters.partial_time = 0;
  
}
  /*
        #define COUNT_NEW_NODE ++total_nodes_count; 
        #define COUNT_LEAF_NODE ++leaf_nodes_count;
        #define COUNT_EMPTY_LEAF_NODE ++empty_leaf_nodes_count;
        #define COUNT_TOTAL_TS(num_ts) total_ts_count+=num_ts; //actual ts inserted in index

        #define COUNT_CHECKED_NODE ++checked_nodes_count;
        #define COUNT_LOADED_NODE ++loaded_nodes_count;
        #define COUNT_LOADED_TS(num_ts) loaded_ts_count +=num_ts; //ts loaded to answer query
        #define COUNT_CHECKED_TS(num_ts) checked_ts_count +=num_ts; //ts loaded to answer query
  */

inline void stats_count_partial_seq_input(int x)
{
   stats.counters.partial_seq_input_count += x ;
}

 
inline void stats_count_partial_seq_output(int x)
{
   stats.counters.partial_seq_output_count += x;
}

inline void stats_count_partial_rand_input(int x)
{
   stats.counters.partial_rand_input_count += x;
}

 
inline void stats_count_partial_rand_output(int x)
{
   stats.counters.partial_rand_output_count += x;
}

inline void stats_count_total_ts_count(int x)
{
   stats.counters.total_ts_count += x;
}

inline void stats_count_input_time_start(void)
{
   gettimeofday(&stats.counters.input_time_start, NULL);   
}

inline void stats_count_output_time_start(void)
{
   gettimeofday(&stats.counters.output_time_start, NULL);   
}

 inline void stats_count_total_time_start(void)
{
   gettimeofday(&stats.counters.total_time_start, NULL);   
}

inline void stats_count_partial_input_time_start(void)
{
   gettimeofday(&stats.counters.partial_input_time_start, NULL);   
}

inline void stats_count_partial_output_time_start(void)
{
   gettimeofday(&stats.counters.partial_output_time_start, NULL);   
}

 inline void stats_count_partial_time_start(void)
{
   gettimeofday(&stats.counters.partial_time_start, NULL);   
} 

inline void stats_count_input_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.input_time_start.tv_sec*1000000 + (stats.counters.input_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_input_time += (stats.counters.tE -stats.counters.tS); 
}

inline void stats_count_output_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.output_time_start.tv_sec*1000000 + (stats.counters.output_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_output_time += (stats.counters.tE -stats.counters.tS); 
}

inline void stats_count_total_time_end(void)
{
  gettimeofday(&(stats.counters.current_time), NULL);					
  stats.counters.tS = (stats.counters.total_time_start).tv_sec*1000000 + (stats.counters.total_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_time += (stats.counters.tE -stats.counters.tS); 
}

inline void stats_count_partial_input_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_input_time_start.tv_sec*1000000 + (stats.counters.partial_input_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_input_time += stats.counters.tE -stats.counters.tS; 
}

inline void stats_count_partial_output_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_output_time_start.tv_sec*1000000 + (stats.counters.partial_output_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_output_time += (stats.counters.tE -stats.counters.tS); 
}

inline void stats_count_partial_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_time_start.tv_sec*1000000 + (stats.counters.partial_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_time += (stats.counters.tE -stats.counters.tS); 
}


inline void stats_update_idx_building_stats(void)
{
  stats.idx_building_total_time = stats.counters.partial_time;
  stats.idx_building_total_input_time = stats.counters.partial_input_time;  
  stats.idx_building_total_output_time = stats.counters.partial_output_time;
  stats.idx_building_total_cpu_time = stats.counters.partial_time -stats.counters.partial_input_time - stats.counters.partial_output_time;

  stats.idx_building_total_seq_input_count = stats.counters.partial_seq_input_count;
  stats.idx_building_total_seq_output_count = stats.counters.partial_seq_output_count;
  stats.idx_building_total_rand_input_count = stats.counters.partial_rand_input_count;
  stats.idx_building_total_rand_output_count = stats.counters.partial_rand_output_count;
  stats.total_ts_count = stats.counters.total_ts_count;
}

inline void stats_update_query_stats(double dist, unsigned long long bytes_read)
{
     stats.query_total_time = stats.counters.partial_time;
     stats.query_total_input_time = stats.counters.partial_input_time;
     stats.query_total_output_time = stats.counters.partial_output_time;
     stats.query_total_cpu_time = stats.counters.partial_time -stats.counters.partial_input_time - stats.counters.partial_output_time;;

     stats.query_total_seq_input_count = stats.counters.partial_seq_input_count;
     stats.query_total_seq_output_count = stats.counters.partial_seq_output_count;
     stats.query_total_rand_input_count = stats.counters.partial_rand_input_count;
     stats.query_total_rand_output_count = stats.counters.partial_rand_output_count;
     stats.query_exact_distance = dist;
     stats.query_checked_ts_count = stats.counters.checked_ts_count;
     
     //stats.query_accessed_bytes = bytes_read;
     
    //stats.query_exact_distance_sanity_check = 0;
    //stats.query_exact_offset = 0;
    //stats.query_accessed_bytes = stats.counters.total_bytes_read;
}


#endif

