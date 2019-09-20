#ifndef STATS_HPP
#define STATS_HPP

#include <string>

using namespace std;
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

extern struct stats_info full_stats;

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
  
        double preprocessing_total_input_time;  
        double preprocessing_total_output_time;
        double preprocessing_total_cpu_time;
        double preprocessing_total_time;

        unsigned long long preprocessing_total_seq_input_count;
        unsigned long long preprocessing_total_seq_output_count;
        unsigned long long preprocessing_total_rand_input_count;
        unsigned long long preprocessing_total_rand_output_count;

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
        unsigned long long program_byte_size;
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
inline void stats_update_preprocessing_stats(void);
inline void stats_update_query_stats(void);

*/
/*
void print_preprocessing_stats (struct stats_info stats, char * dataset);
void print_query_stats(struct stats_info stats, unsigned int query_num, char * queries);
void print_queries_stats(struct stats_info stats);
*/


void print_preprocessing_stats(struct stats_info stats, char * dataset);
void print_query_stats(struct stats_info full_stats, unsigned int query_num, char * queries);

inline void stats_init_counters (void )
{
   full_stats.counters.total_input_time = 0;
   full_stats.counters.total_output_time = 0;
   full_stats.counters.total_time = 0;
   full_stats.counters.total_parse_time = 0;
   full_stats.counters.load_node_time= 0;
   full_stats.counters.partial_time = 0;
   full_stats.counters.partial_input_time = 0;	
   full_stats.counters.partial_output_time = 0;
   full_stats.counters.partial_load_node_time = 0;
   full_stats.counters.partial_seq_input_count = 0;
   full_stats.counters.partial_seq_output_count = 0;
   full_stats.counters.partial_rand_input_count = 0;
   full_stats.counters.partial_rand_output_count = 0;
   full_stats.counters.total_nodes_count = 0;
   full_stats.counters.leaf_nodes_count = 0;
   full_stats.counters.empty_leaf_nodes_count = 0;
   full_stats.counters.loaded_nodes_count = 0;
   full_stats.counters.loaded_ts_count = 0;			    
   full_stats.counters.checked_ts_count = 0;
   full_stats.counters.checked_nodes_count = 0;			   
   full_stats.counters.total_ts_count = 0;
}

inline void stats_init(void)
{
 
    /*PREPROCESSING_TOTAL STATISTICS*/
    full_stats.preprocessing_total_input_time = 0;  
    full_stats.preprocessing_total_output_time = 0;
    full_stats.preprocessing_total_cpu_time = 0;
    full_stats.preprocessing_total_time = 0;

    full_stats.preprocessing_total_seq_input_count = 0;
    full_stats.preprocessing_total_seq_output_count = 0;
    full_stats.preprocessing_total_rand_input_count = 0;
    full_stats.preprocessing_total_rand_output_count = 0;    
    
    /*PER QUERY STATISTICS*/    
    full_stats.query_total_input_time = 0;
    full_stats.query_total_output_time = 0;
    full_stats.query_total_cpu_time = 0;
    full_stats.query_total_time = 0;    

    full_stats.query_total_seq_input_count = 0;
    full_stats.query_total_seq_output_count = 0;
    full_stats.query_total_rand_input_count = 0;
    full_stats.query_total_rand_output_count = 0;
    
    full_stats.query_exact_distance = 0;
    full_stats.query_exact_distance_sanity_check = 0;
    full_stats.query_exact_offset = 0;
    full_stats.query_accessed_bytes = 0;
    
    /*SUMMARY STATISTICS FOR ALL QUERIES*/        
    full_stats.queries_total_input_time = 0;
    full_stats.queries_total_output_time = 0;
    full_stats.queries_total_cpu_time = 0;
    full_stats.queries_total_time = 0;    

    full_stats.queries_total_seq_input_count = 0;
    full_stats.queries_total_seq_output_count = 0;
    full_stats.queries_total_rand_input_count = 0;
    full_stats.queries_total_rand_output_count = 0;        
    
    /*COMBINED STATISTICS FOR PREPROCESSING AND QUERY WORKLOAD*/            
    full_stats.total_input_time = 0;
    full_stats.total_output_time = 0;
    full_stats.total_cpu_time = 0;
    full_stats.total_time = 0;
    full_stats.total_time_sanity_check = 0;
    
    full_stats.total_seq_input_count = 0;
    full_stats.total_seq_output_count = 0;
    full_stats.total_rand_input_count = 0;
    full_stats.total_rand_output_count = 0;
    
    full_stats.total_ts_count = 0;
    
    full_stats.total_queries_count = 0;
    full_stats.program_byte_size = 0;
    full_stats.dataset_byte_size = 0;
    
    stats_init_counters();
    
}

inline void stats_reset_query_counters(void)
{
   full_stats.counters.loaded_nodes_count = 0;
   full_stats.counters.loaded_ts_count = 0;
   full_stats.counters.checked_nodes_count = 0;
   full_stats.counters.checked_ts_count = 0;
}

inline void stats_reset_partial_counters(void)
{
   full_stats.counters.partial_seq_output_count = 0;
   full_stats.counters.partial_seq_input_count = 0;
   full_stats.counters.partial_rand_output_count = 0;
   full_stats.counters.partial_rand_input_count = 0;
   full_stats.counters.partial_input_time = 0;
   full_stats.counters.partial_output_time = 0;
   full_stats.counters.partial_load_node_time = 0;
   full_stats.counters.partial_time = 0;
  
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

inline void stats_count_total_ts(int i)
{
  full_stats.counters.total_ts_count += i;
}

inline void stats_count_total_nodes(int i)
{
  full_stats.counters.total_nodes_count += i;
}

inline void stats_count_leaf_nodes(int i)
{
  full_stats.counters.leaf_nodes_count += i;
}

inline void stats_count_partial_seq_input(void)
{
   ++full_stats.counters.partial_seq_input_count;
}

 
inline void stats_count_partial_seq_output(void)
{
   ++full_stats.counters.partial_seq_output_count;
}

inline void stats_count_partial_rand_input(void)
{
   ++full_stats.counters.partial_rand_input_count;
}

 
inline void stats_count_partial_rand_output(void)
{
   ++full_stats.counters.partial_rand_output_count;
}

inline void stats_count_input_time_start(void)
{
   gettimeofday(&full_stats.counters.input_time_start, NULL);   
}

inline void stats_count_output_time_start(void)
{
   gettimeofday(&full_stats.counters.output_time_start, NULL);   
}

 inline void stats_count_total_time_start(void)
{
   gettimeofday(&full_stats.counters.total_time_start, NULL);   
}

inline void stats_count_partial_input_time_start(void)
{
   gettimeofday(&full_stats.counters.partial_input_time_start, NULL);   
}

inline void stats_count_partial_output_time_start(void)
{
   gettimeofday(&full_stats.counters.partial_output_time_start, NULL);   
}

 inline void stats_count_partial_time_start(void)
{
   gettimeofday(&full_stats.counters.partial_time_start, NULL);   
} 

inline void stats_count_input_time_end(void)
{
  gettimeofday(&full_stats.counters.current_time, NULL);					
  full_stats.counters.tS = full_stats.counters.input_time_start.tv_sec*1000000 + (full_stats.counters.input_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.total_input_time += (full_stats.counters.tE -full_stats.counters.tS); 
}

inline void stats_count_output_time_end(void)
{
  gettimeofday(&full_stats.counters.current_time, NULL);					
  full_stats.counters.tS = full_stats.counters.output_time_start.tv_sec*1000000 + (full_stats.counters.output_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.total_output_time += (full_stats.counters.tE -full_stats.counters.tS); 
}

inline void stats_count_total_time_end(void)
{
  gettimeofday(&(full_stats.counters.current_time), NULL);					
  full_stats.counters.tS = (full_stats.counters.total_time_start).tv_sec*1000000 + (full_stats.counters.total_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.total_time += (full_stats.counters.tE -full_stats.counters.tS); 
}

inline void stats_count_partial_input_time_end(void)
{
  gettimeofday(&full_stats.counters.current_time, NULL);					
  full_stats.counters.tS = full_stats.counters.partial_input_time_start.tv_sec*1000000 + (full_stats.counters.partial_input_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.partial_input_time += full_stats.counters.tE -full_stats.counters.tS; 
}

inline void stats_count_partial_output_time_end(void)
{
  gettimeofday(&full_stats.counters.current_time, NULL);					
  full_stats.counters.tS = full_stats.counters.partial_output_time_start.tv_sec*1000000 + (full_stats.counters.partial_output_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.partial_output_time += (full_stats.counters.tE -full_stats.counters.tS); 
}

inline void stats_count_partial_time_end(void)
{
  gettimeofday(&full_stats.counters.current_time, NULL);					
  full_stats.counters.tS = full_stats.counters.partial_time_start.tv_sec*1000000 + (full_stats.counters.partial_time_start.tv_usec); 
  full_stats.counters.tE = full_stats.counters.current_time.tv_sec*1000000 + (full_stats.counters.current_time.tv_usec); 
  full_stats.counters.partial_time += (full_stats.counters.tE -full_stats.counters.tS); 
}


inline void stats_update_preprocessing_stats(void)
{
  full_stats.preprocessing_total_time = full_stats.counters.partial_time;
  full_stats.preprocessing_total_input_time = full_stats.counters.partial_input_time;  
  full_stats.preprocessing_total_output_time = full_stats.counters.partial_output_time;
  full_stats.preprocessing_total_cpu_time = full_stats.counters.partial_time -full_stats.counters.partial_input_time - full_stats.counters.partial_output_time;

  full_stats.preprocessing_total_seq_input_count = full_stats.counters.partial_seq_input_count;
  full_stats.preprocessing_total_seq_output_count = full_stats.counters.partial_seq_output_count;
  full_stats.preprocessing_total_rand_input_count = full_stats.counters.partial_rand_input_count;
  full_stats.preprocessing_total_rand_output_count = full_stats.counters.partial_rand_output_count;
  
}

inline void stats_update_query_stats(double dist, unsigned long long bytes_read)
{
     full_stats.query_total_time = full_stats.counters.partial_time;
     full_stats.query_total_input_time = full_stats.counters.partial_input_time;
     full_stats.query_total_output_time = full_stats.counters.partial_output_time;
     full_stats.query_total_cpu_time = full_stats.counters.partial_time -full_stats.counters.partial_input_time - full_stats.counters.partial_output_time;;

     full_stats.query_total_seq_input_count = full_stats.counters.partial_seq_input_count;
     full_stats.query_total_seq_output_count = full_stats.counters.partial_seq_output_count;
     full_stats.query_total_rand_input_count = full_stats.counters.partial_rand_input_count;
     full_stats.query_total_rand_output_count = full_stats.counters.partial_rand_output_count;
     full_stats.query_exact_distance = dist;
     full_stats.query_accessed_bytes = bytes_read;
     
    //full_stats.query_exact_distance_sanity_check = 0;
    //full_stats.query_exact_offset = 0;
    //full_stats.query_accessed_bytes = full_stats.counters.total_bytes_read;
}


#endif

