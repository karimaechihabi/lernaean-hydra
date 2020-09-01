#ifndef STATS_H
#define STATS_H

#include <string>

using namespace std;
#include <time.h>
#include <sys/time.h>

extern struct stats_info stats;

struct stats_counters {
        double tS;
        double tE;

        struct timeval total_time_start;
        struct timeval parse_time_start;
        struct timeval input_time_start;
        struct timeval output_time_start;
        struct timeval load_node_start;
        struct timeval current_time;
        struct timeval fetch_start;
        struct timeval fetch_check_start;
        double total_input_time;
        double total_output_time;
        double total_parse_time;
        double load_node_time;
        double total_time;


        struct timeval partial_time_start;
        struct timeval partial_input_time_start;
        struct timeval partial_output_time_start;
        struct timeval partial_load_node_time_start;         

        double partial_time;
        double partial_input_time;
        double partial_output_time;
        double partial_load_node_time;

        unsigned long long partial_seq_input_count;
        unsigned long long partial_seq_output_count;
        unsigned long long partial_rand_input_count;
        unsigned long long partial_rand_output_count;

        unsigned long total_nodes_count;
        unsigned long leaf_nodes_count;
        unsigned long empty_leaf_nodes_count;
        unsigned long loaded_nodes_count;
        unsigned long checked_nodes_count;
        unsigned long loaded_ts_count;
        unsigned long checked_ts_count;
        unsigned long total_ts_count;
        unsigned long total_queries_count;


};


struct stats_info {

        struct stats_counters counters;
  
        double idx_total_input_time;  
        double idx_total_output_time;
        double idx_total_cpu_time;
        double idx_total_time;

        unsigned long long idx_total_seq_input_count;
        unsigned long long idx_total_seq_output_count;
        unsigned long long idx_total_rand_input_count;
        unsigned long long idx_total_rand_output_count;

        double idx_building_input_time;  
        double idx_building_output_time;
        double idx_building_cpu_time;
        double idx_building_total_time;

        unsigned long long idx_building_seq_input_count;
        unsigned long long idx_building_seq_output_count;
        unsigned long long idx_building_rand_input_count;
        unsigned long long idx_building_rand_output_count;

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
        unsigned long long query_accessed_bytes;

        double queries_total_input_time;
        double queries_total_output_time;
        double queries_total_cpu_time;
        double queries_total_time;    

        unsigned long long queries_total_seq_input_count;
        unsigned long long queries_total_seq_output_count;
        unsigned long long queries_total_rand_input_count;
        unsigned long long queries_total_rand_output_count;
    
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
        unsigned long long query_pruned_ts;
        unsigned long long dataset_byte_size;
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
inline void stats_update_idx_stats(void);
inline void stats_update_query_stats(void);

*/
void print_index_stats (struct stats_info stats, const char * dataset);
void print_query_stats(struct stats_info stats, unsigned int query_num, const char * queries);
void print_queries_stats(struct stats_info stats);



inline void stats_init_counters (void )
{
   stats.counters.total_input_time = 0;
   stats.counters.total_output_time = 0;
   stats.counters.total_time = 0;
   stats.counters.total_parse_time = 0;
   stats.counters.load_node_time= 0;
   stats.counters.partial_time = 0;
   stats.counters.partial_input_time = 0;	
   stats.counters.partial_output_time = 0;
   stats.counters.partial_load_node_time = 0;
   stats.counters.partial_seq_input_count = 0;
   stats.counters.partial_seq_output_count = 0;
   stats.counters.partial_rand_input_count = 0;
   stats.counters.partial_rand_output_count = 0;
   stats.counters.total_nodes_count = 0;
   stats.counters.leaf_nodes_count = 0;
   stats.counters.empty_leaf_nodes_count = 0;
   stats.counters.loaded_nodes_count = 0;
   stats.counters.loaded_ts_count = 0;			    
   stats.counters.checked_ts_count = 0;
   stats.counters.checked_nodes_count = 0;			   
   stats.counters.total_ts_count = 0;
}

inline void stats_init(void)
{
 
    /*IDX_TOTAL STATISTICS*/
    stats.idx_total_input_time = 0;  
    stats.idx_total_output_time = 0;
    stats.idx_total_cpu_time = 0;
    stats.idx_total_time = 0;

    stats.idx_total_seq_input_count = 0;
    stats.idx_total_seq_output_count = 0;
    stats.idx_total_rand_input_count = 0;
    stats.idx_total_rand_output_count = 0;    
    
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
    stats.query_accessed_bytes = 0;
    
    /*SUMMARY STATISTICS FOR ALL QUERIES*/        
    stats.queries_total_input_time = 0;
    stats.queries_total_output_time = 0;
    stats.queries_total_cpu_time = 0;
    stats.queries_total_time = 0;    

    stats.queries_total_seq_input_count = 0;
    stats.queries_total_seq_output_count = 0;
    stats.queries_total_rand_input_count = 0;
    stats.queries_total_rand_output_count = 0;        
    
    /*COMBINED STATISTICS FOR IDX AND QUERY WORKLOAD*/            
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
    stats.query_pruned_ts = 0;
    stats.dataset_byte_size = 0;
    
    stats_init_counters();
    
}

inline void stats_reset_query_counters(void)
{
   stats.counters.loaded_nodes_count = 0;
   stats.counters.loaded_ts_count = 0;
   stats.counters.checked_nodes_count = 0;
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
   stats.counters.partial_load_node_time = 0;
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

inline void stats_count_partial_seq_input(void)
{
   ++stats.counters.partial_seq_input_count;
}

 
inline void stats_count_partial_seq_output(void)
{
   ++stats.counters.partial_seq_output_count;
}

inline void stats_count_partial_rand_input(void)
{
   ++stats.counters.partial_rand_input_count;
}

 
inline void stats_count_partial_rand_output(void)
{
   ++stats.counters.partial_rand_output_count;
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


inline void stats_update_index_stats(void)
{
  stats.idx_building_total_time = stats.counters.partial_time;
  stats.idx_building_input_time = stats.counters.partial_input_time;  
  stats.idx_building_output_time = stats.counters.partial_output_time;
  stats.idx_building_cpu_time = stats.counters.partial_time -stats.counters.partial_input_time - stats.counters.partial_output_time;

  stats.idx_building_seq_input_count = stats.counters.partial_seq_input_count;
  stats.idx_building_seq_output_count = stats.counters.partial_seq_output_count;
  stats.idx_building_rand_input_count = stats.counters.partial_rand_input_count;
  stats.idx_building_rand_output_count = stats.counters.partial_rand_output_count;
  
}

inline void stats_update_query_stats(double dist, unsigned long long pruned_ts)
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
     stats.query_pruned_ts = pruned_ts;
     
    //stats.query_exact_distance_sanity_check = 0;
    //stats.query_exact_offset = 0;
    //stats.query_accessed_bytes = stats.counters.total_bytes_read;
}


#endif

