#include "../src/cpp/flann/stats.h"

#include <stdio.h>

//using namespace std;
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>

#include <string.h>
extern struct stats_info stats;

void get_index_stats(struct stats_info *  stats, char * index_filename)
{
  stats->total_seq_input_count = stats->idx_building_total_seq_input_count;
  stats->total_seq_output_count = stats->idx_building_total_seq_output_count;
  stats->total_rand_input_count = stats->idx_building_total_rand_input_count;
  stats->total_rand_output_count = stats->idx_building_total_rand_output_count;
  
  stats->total_input_time = stats->idx_building_total_input_time;
  stats->total_output_time = stats->idx_building_total_output_time;
  stats->total_cpu_time    = stats->idx_building_total_cpu_time;
 
  stats->total_time    = stats->total_input_time;
  //stats->total_time_sanity_check = total_time;
   
  stats->total_ts_count = stats->total_ts_count;

  //get_index_footprint(stats, index_filename);

}

void get_index_footprint(struct stats_info * stats, char * index_filename)
{
  
 
    struct stat st;
    
    if (stat(index_filename, &st) == 0)
    {
      stats->idx_size_bytes = (long long) st.st_size;
      stats->idx_size_blocks = (long long) st.st_blksize;
    }

}

void print_index_stats(struct stats_info stats, char * dataset, unsigned int max_leaf_size)
{
      //  id = -1 for index and id = query_id for queries
        int id = -1;
        printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_total_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	       stats.idx_building_total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	       stats.idx_building_total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	       stats.idx_building_total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	       stats.idx_building_total_rand_output_count,
	       dataset,
	       id); 
        printf("Index_total_input_time_secs\t%lf\t%s\t%d\n",
	       stats.total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_total_output_time_secs\t%lf\t%s\t%d\n",
	       stats.total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       stats.total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_total_time_secs\t%lf\t%s\t%d\n",
	       stats.total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_total_seq_input_count\t%llu\t%s\t%d\n",
	       stats.total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_total_seq_output_count\t%llu\t%s\t%d\n",
	       stats.total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_input_count\t%llu\t%s\t%d\n",
	       stats.total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_output_count\t%llu\t%s\t%d\n",
	       stats.total_rand_output_count,
	       dataset,
	       id);
	
	double size_MB =  (stats.idx_size_bytes)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);

	printf("Total_ts_count\t%u\t%s\t%d\n",
	       stats.total_ts_count,	       
	       dataset,
	       id);

          
}


void get_query_stats(struct stats_info * stats)
{

  if (stats->total_ts_count != 0)
    {
        stats->query_pruning_ratio = 1.0 - ((double)stats->query_checked_ts_count/
						 stats->total_ts_count);
    }
    
}



void print_query_stats(struct stats_info stats, char * queries, unsigned int query_num, unsigned found_knn)
{
        printf("Query_total_input_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats.query_total_input_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats.query_total_output_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats.query_total_cpu_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats.query_total_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%u\t%u\n",
	       stats.query_total_seq_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%u\t%u\n",
	       stats.query_total_seq_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%u\t%u\n",
	       stats.query_total_rand_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%u\t%u\n",
	       stats.query_total_rand_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );


        printf("Query_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	       stats.query_checked_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_exact_distance\t%f\t%s\t%u\t%u\n",
	       stats.query_exact_distance,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_pruning_ratio_level\t%f\t%s\t%u\t%u\n",
	       stats.query_pruning_ratio,	       
	       queries,
	       query_num,
	       found_knn	       
	       );

	printf("Query_exact_file_position\t%u\t%s\t%u\t%u\n",
	       stats.query_file_position,
	       queries,
	       query_num,
	       found_knn	       
	       );	
}

void stats_init_counters (void )
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

 void stats_init(void)
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

  void stats_reset_query_counters(void)
{
   stats.counters.checked_ts_count = 0;
}

  void stats_reset_partial_counters(void)
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

  void stats_count_partial_seq_input(int x)
{
   stats.counters.partial_seq_input_count += x ;
}

 
  void stats_count_partial_seq_output(int x)
{
   stats.counters.partial_seq_output_count += x;
}

  void stats_count_partial_rand_input(int x)
{
   stats.counters.partial_rand_input_count += x;
}

 
  void stats_count_partial_rand_output(int x)
{
   stats.counters.partial_rand_output_count += x;
}

  void stats_count_total_ts_count(int x)
{
   stats.counters.total_ts_count += x;
}

  void stats_count_input_time_start(void)
{
   gettimeofday(&stats.counters.input_time_start, NULL);   
}

  void stats_count_output_time_start(void)
{
   gettimeofday(&stats.counters.output_time_start, NULL);   
}

   void stats_count_total_time_start(void)
{
   gettimeofday(&stats.counters.total_time_start, NULL);   
}

  void stats_count_partial_input_time_start(void)
{
   gettimeofday(&stats.counters.partial_input_time_start, NULL);   
}

  void stats_count_partial_output_time_start(void)
{
   gettimeofday(&stats.counters.partial_output_time_start, NULL);   
}

   void stats_count_partial_time_start(void)
{
   gettimeofday(&stats.counters.partial_time_start, NULL);   
} 

  void stats_count_input_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.input_time_start.tv_sec*1000000 + (stats.counters.input_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_input_time += (stats.counters.tE -stats.counters.tS); 
}

  void stats_count_output_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.output_time_start.tv_sec*1000000 + (stats.counters.output_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_output_time += (stats.counters.tE -stats.counters.tS); 
}

  void stats_count_total_time_end(void)
{
  gettimeofday(&(stats.counters.current_time), NULL);					
  stats.counters.tS = (stats.counters.total_time_start).tv_sec*1000000 + (stats.counters.total_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.total_time += (stats.counters.tE -stats.counters.tS); 
}

  void stats_count_partial_input_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_input_time_start.tv_sec*1000000 + (stats.counters.partial_input_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_input_time += stats.counters.tE -stats.counters.tS; 
}

  void stats_count_partial_output_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_output_time_start.tv_sec*1000000 + (stats.counters.partial_output_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_output_time += (stats.counters.tE -stats.counters.tS); 
}

  void stats_count_partial_time_end(void)
{
  gettimeofday(&stats.counters.current_time, NULL);					
  stats.counters.tS = stats.counters.partial_time_start.tv_sec*1000000 + (stats.counters.partial_time_start.tv_usec); 
  stats.counters.tE = stats.counters.current_time.tv_sec*1000000 + (stats.counters.current_time.tv_usec); 
  stats.counters.partial_time += (stats.counters.tE -stats.counters.tS); 
}


  void stats_update_idx_building_stats(void)
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

void stats_update_query_stats(double dist, unsigned long long bytes_read, unsigned int file_pos)
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
     stats.query_file_position = file_pos;
     
     //stats.query_accessed_bytes = bytes_read;
     
    //stats.query_exact_distance_sanity_check = 0;
    stats.query_exact_offset = 0;
    //stats.query_accessed_bytes = stats.counters.total_bytes_read;
}
