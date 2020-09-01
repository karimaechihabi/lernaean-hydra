#include "stats.hpp"

#include <stdio.h>

using namespace std;
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
//extern struct stats_info stats;

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

  get_index_footprint(stats, index_filename);

}

void get_index_footprint(struct stats_info * stats, char * index_filename)
{
  
    struct stat st;
    unsigned int  count_leaves;
    
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

        printf("Index_training_input_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_training_total_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_training_output_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_training_total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_training_cpu_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_training_total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_training_total_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_training_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_training_seq_input_count\t%llu\t%s\t%d\n",
	       stats.idx_training_total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_training_seq_output_count\t%llu\t%s\t%d\n",
	       stats.idx_training_total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_training_rand_input_count\t%llu\t%s\t%d\n",
	       stats.idx_training_total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_training_rand_output_count\t%llu\t%s\t%d\n",
	       stats.idx_training_total_rand_output_count,
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

	printf("Query_file_position\t%u\t%s\t%u\t%u\n",
	       stats.query_file_position,
	       queries,
	       query_num,
	       found_knn	       
	       );	
}

