#include "../include/stats.h"

#include <stdio.h>

#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern struct stats_info stats;

void print_query_stats(unsigned int query_num, char * queries)
{
  /*
    printf("-----------------\n");
    printf("QUERY STATISTICS \n");
    printf("QUERY: %d \n", query_num);
    printf("-----------------\n");
  */

  printf("Query_filter_input_time_secs\t%lf\t%s\t%u\n",
	 stats.query_filter_input_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_filter_output_time_secs\t%lf\t%s\t%u\n",
	 stats.query_filter_output_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\n",
	 stats.query_filter_load_node_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\n",
	 stats.query_filter_cpu_time/1000000,	       
	 queries,
	 query_num
	 );
	
  printf("Query_filter_total_time_secs\t%lf\t%s\t%u\n",
	 stats.query_filter_total_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_filter_seq_input_count\t%llu\t%s\t%d\n",
	 stats.query_filter_seq_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_filter_seq_output_count\t%llu\t%s\t%d\n",
	 stats.query_filter_seq_output_count,
	 queries,
	 query_num
	 );
	
  printf("Query_filter_rand_input_count\t%llu\t%s\t%d\n",
	 stats.query_filter_rand_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_filter_rand_output_count\t%llu\t%s\t%d\n",
	 stats.query_filter_rand_output_count,
	 queries,
	 query_num
	 );
  printf("Query_filter_checked_nodes_count\t%u\t%s\t%d\n",
	 stats.query_filter_checked_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_filter_checked_ts_count\t%llu\t%s\t%d\n",
	 stats.query_filter_checked_ts_count,
	 queries,
	 query_num
	 );	

  printf("Query_filter_loaded_nodes_count\t%u\t%s\t%d\n",
	 stats.query_filter_loaded_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_filter_loaded_ts_count\t%llu\t%s\t%d\n",
	 stats.query_filter_loaded_ts_count,
	 queries,
	 query_num
	 );
	

  printf("Query_refine_input_time_secs\t%lf\t%s\t%u\n",
	 stats.query_refine_input_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_refine_output_time_secs\t%lf\t%s\t%u\n",
	 stats.query_refine_output_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\n",
	 stats.query_refine_load_node_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\n",
	 stats.query_refine_cpu_time/1000000,	       
	 queries,
	 query_num
	 );
	
  printf("Query_refine_total_time_secs\t%lf\t%s\t%u\n",
	 stats.query_refine_total_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_refine_seq_input_count\t%llu\t%s\t%d\n",
	 stats.query_refine_seq_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_refine_seq_output_count\t%llu\t%s\t%d\n",
	 stats.query_refine_seq_output_count,
	 queries,
	 query_num
	 );
	
  printf("Query_refine_rand_input_count\t%llu\t%s\t%d\n",
	 stats.query_refine_rand_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_refine_rand_output_count\t%llu\t%s\t%d\n",
	 stats.query_refine_rand_output_count,
	 queries,
	 query_num
	 );

	
  printf("Query_refine_checked_nodes_count\t%u\t%s\t%d\n",
	 stats.query_refine_checked_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_refine_checked_ts_count\t%llu\t%s\t%d\n",
	 stats.query_refine_checked_ts_count,
	 queries,
	 query_num
	 );	

  printf("Query_refine_loaded_nodes_count\t%u\t%s\t%d\n",
	 stats.query_refine_loaded_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_refine_loaded_ts_count\t%llu\t%s\t%d\n",
	 stats.query_refine_loaded_ts_count,
	 queries,
	 query_num
	 );

  printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	 stats.query_total_input_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	 stats.query_total_output_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\n",
	 stats.query_total_load_node_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	 stats.query_total_cpu_time/1000000,	       
	 queries,
	 query_num
	 );
	
  printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	 stats.query_total_time/1000000,	       
	 queries,
	 query_num
	 );

  printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	 stats.query_total_seq_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	 stats.query_total_seq_output_count,
	 queries,
	 query_num
	 );
	
  printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	 stats.query_total_rand_input_count,
	 queries,
	 query_num
	 );
	
  printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	 stats.query_total_rand_output_count,
	 queries,
	 query_num
	 );


  printf("Query_total_checked_nodes_count\t%u\t%s\t%d\n",
	 stats.query_total_checked_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_total_checked_ts_count\t%llu\t%s\t%d\n",
	 stats.query_total_checked_ts_count,
	 queries,
	 query_num
	 );	

  printf("Query_total_loaded_nodes_count\t%u\t%s\t%d\n",
	 stats.query_total_loaded_nodes_count,
	 queries,
	 query_num
	 );	

  printf("Query_total_loaded_ts_count\t%llu\t%s\t%d\n",
	 stats.query_total_loaded_ts_count,
	 queries,
	 query_num
	 );

  printf("Query_approx_distance\t%f\t%s\t%d\n",
	 stats.query_approx_distance,
	 queries,
	 query_num
	 );	

  printf("Query_approx_node_filename\t%s\t%s\t%d\n",
	 stats.query_approx_node_filename,
	 queries,
	 query_num
	 );
	
  printf("Query_approx_node_size\t%u\t%s\t%d\n",
	 stats.query_approx_node_size,
	 queries,
	 query_num
	 );	

  printf("Query_approx_node_level\t%u\t%s\t%d\n",
	 stats.query_approx_node_level,
	 queries,
	 query_num
	 );

  printf("Query_exact_distance\t%f\t%s\t%d\n",
	 stats.query_exact_distance,
	 queries,
	 query_num
	 );	

  printf("Query_exact_node_filename\t%s\t%s\t%d\n",
	 stats.query_exact_node_filename,
	 queries,
	 query_num
	 );
	
  printf("Query_exact_node_size\t%u\t%s\t%d\n",
	 stats.query_exact_node_size,
	 queries,
	 query_num
	 );	

  printf("Query_exact_node_level\t%u\t%s\t%d\n",
	 stats.query_exact_node_level,
	 queries,
	 query_num
	 );
  printf("Query_lb_distance\t%f\t%s\t%d\n",
	 stats.query_lb_distance,
	 queries,
	 query_num
	 );
	
  printf("Query_tlb\t%f\t%s\t%d\n",
	 stats.query_tlb,	       
	 queries,
	 query_num
	 );	
	
  printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	 stats.query_pruning_ratio,	       
	 queries,
	 query_num
	 );

  printf("Query_eff_epsilon\t%f\t%s\t%d\n",
	 stats.query_eff_epsilon,	       
	 queries,
	 query_num
	 );		

}

void print_knn_query_stats(unsigned int query_num, unsigned int found_knn, char * queries)
{
  /*
    printf("-----------------\n");
    printf("QUERY STATISTICS \n");
    printf("QUERY: %d \n", query_num);
    printf("-----------------\n");
  */

  printf("Query_filter_input_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_filter_input_time/1000000,	       
	 queries,
	 query_num,
	 found_knn
	 );

  printf("Query_filter_output_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_filter_output_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_filter_load_node_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_filter_cpu_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_filter_total_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_filter_total_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_filter_seq_input_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_seq_input_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_filter_seq_output_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_seq_output_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_filter_rand_input_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_rand_input_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_filter_rand_output_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_rand_output_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
  printf("Query_filter_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_filter_checked_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_filter_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_checked_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_filter_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_filter_loaded_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_filter_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_filter_loaded_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	

  printf("Query_refine_input_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_refine_input_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_refine_output_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_refine_output_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_refine_load_node_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_refine_cpu_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_refine_total_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_refine_total_time/1000000,	       
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_refine_seq_input_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_seq_input_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_refine_seq_output_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_seq_output_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_refine_rand_input_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_rand_input_count,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_refine_rand_output_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_rand_output_count,
	 queries,
	 query_num,
	 found_knn	 
	 );

	
  printf("Query_refine_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_refine_checked_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_refine_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_checked_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_refine_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_refine_loaded_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_refine_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_refine_loaded_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );

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

  printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	 stats.query_total_load_node_time/1000000,	       
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


  printf("Query_total_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_total_checked_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_total_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_total_checked_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_total_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	 stats.query_total_loaded_nodes_count,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_total_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	 stats.query_total_loaded_ts_count,
	 queries,
	 query_num,
	 found_knn	 
	 );

  printf("Query_approx_distance\t%f\t%s\t%u\t%u\n",
	 stats.query_approx_distance,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_approx_node_filename\t%s\t%s\t%u\t%u\n",
	 stats.query_approx_node_filename,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_approx_node_size\t%u\t%s\t%u\t%u\n",
	 stats.query_approx_node_size,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_approx_node_level\t%u\t%s\t%u\t%u\n",
	 stats.query_approx_node_level,
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

  printf("Query_exact_node_filename\t%s\t%s\t%u\t%u\n",
	 stats.query_exact_node_filename,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_exact_node_size\t%u\t%s\t%u\t%u\n",
	 stats.query_exact_node_size,
	 queries,
	 query_num,
	 found_knn	 
	 );	

  printf("Query_exact_node_level\t%u\t%s\t%u\t%u\n",
	 stats.query_exact_node_level,
	 queries,
	 query_num,
	 found_knn	 
	 );
  printf("Query_lb_distance\t%f\t%s\t%u\t%u\n",
	 stats.query_lb_distance,
	 queries,
	 query_num,
	 found_knn	 
	 );
	
  printf("Query_tlb\t%f\t%s\t%u\t%u\n",
	 stats.query_tlb,	       
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

  printf("Query_eff_epsilon\t%f\t%s\t%u\t%u\n",
	 stats.query_eff_epsilon,	       
	 queries,
	 query_num,
	 found_knn	 
	 );		

}



void get_index_stats(char * index_filename)
{
  stats.total_seq_input_count = stats.idx_building_seq_input_count
    + stats.idx_writing_seq_input_count
    + stats.idx_reading_seq_input_count;

  stats.total_seq_output_count = stats.idx_building_seq_output_count
    + stats.idx_writing_seq_output_count
    + stats.idx_reading_seq_output_count;

  stats.total_rand_input_count = stats.idx_building_rand_input_count
    + stats.idx_writing_rand_input_count
    + stats.idx_reading_rand_input_count;

  stats.total_rand_output_count = stats.idx_building_rand_output_count
    + stats.idx_writing_rand_output_count
    + stats.idx_reading_rand_output_count;

  
  stats.total_input_time = stats.idx_building_input_time
    + stats.idx_writing_input_time
    + stats.idx_reading_input_time;

  stats.total_output_time = stats.idx_building_output_time
    + stats.idx_writing_output_time
    + stats.idx_reading_output_time;

  stats.total_cpu_time    = stats.idx_building_cpu_time
    + stats.idx_writing_cpu_time
    + stats.idx_reading_cpu_time;


  stats.total_time    = stats.total_input_time
    + stats.total_output_time
    + stats.total_cpu_time;

  stats.total_parse_time = total_parse_time;


  stats.leaf_nodes_count = leaf_nodes_count;
  stats.empty_leaf_nodes_count = empty_leaf_nodes_count;
  

  stats.total_nodes_count = total_nodes_count;
  stats.total_ts_count = total_ts_count;

  get_index_footprint(index_filename);

}


void get_index_footprint(char * index_filename)
{

  // const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
  //filename = strcpy(filename, index->settings->root_directory);
  //filename = strcat(filename, "/index.idx");

  struct stat st;
  unsigned int  count_leaves;

  if (stat(index_filename, &st) == 0)
    {
      stats.idx_size_bytes = (long long) st.st_size;
      stats.idx_size_blocks = (long long) st.st_blksize;
    }

  count_leaves = stats.leaf_nodes_count;
    
}

  
void print_index_stats(char * dataset, unsigned int max_leaf_size)
{
  int id = -1;
  printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_building_input_time/1000000,
	 dataset,
	 id); 
  printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_building_output_time/1000000,
	 dataset,
	 id);
  printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_building_cpu_time/1000000,
	 dataset,
	 id);	
  printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_building_total_time/1000000,
	 dataset,
	 id);
	
  printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	 stats.idx_building_seq_input_count,
	 dataset,
	 id);
	
  printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	 stats.idx_building_seq_output_count,
	 dataset,
	 id);
	
  printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	 stats.idx_building_rand_input_count,
	 dataset,
	 id);
	
  printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	 stats.idx_building_rand_output_count,
	 dataset,
	 id); 

  printf("Index_writing_input_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_writing_input_time/1000000,
	 dataset,
	 id);

  printf("Index_writing_output_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_writing_output_time/1000000,
	 dataset,
	 id);

  printf("Index_writing_cpu_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_writing_cpu_time/1000000,
	 dataset,
	 id);
	
  printf("Index_writing_total_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_writing_total_time/1000000,
	 dataset,
	 id);	

  printf("Index_writing_seq_input_count\t%llu\t%s\t%d\n",
	 stats.idx_writing_seq_input_count,
	 dataset,
	 id);

  printf("Index_writing_seq_output_count\t%llu\t%s\t%d\n",
	 stats.idx_writing_seq_output_count,
	 dataset,
	 id);

  printf("Index_writing_rand_input_count\t%llu\t%s\t%d\n",
	 stats.idx_writing_rand_input_count,
	 dataset,
	 id);

  printf("Index_writing_rand_output_count\t%llu\t%s\t%d\n",
	 stats.idx_writing_rand_output_count,
	 dataset,
	 id);	

  printf("Index_reading_input_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_reading_input_time/1000000,
	 dataset,
	 id
	 ); 
  printf("Index_reading_output_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_reading_output_time/1000000,
	 dataset,
	 id);
  printf("Index_reading_cpu_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_reading_cpu_time/1000000,
	 dataset,
	 id);	
  printf("Index_reading_total_time_secs\t%lf\t%s\t%d\n",
	 stats.idx_reading_total_time/1000000,
	 dataset,
	 id);
	
  printf("Index_reading_seq_input_count\t%llu\t%s\t%d\n",
	 stats.idx_reading_seq_input_count,
	 dataset,
	 id);
	
  printf("Index_reading_seq_output_count\t%llu\t%s\t%d\n",
	 stats.idx_reading_seq_output_count,
	 dataset,
	 id);
	
  printf("Index_reading_rand_input_count\t%llu\t%s\t%d\n",
	 stats.idx_reading_rand_input_count,
	 dataset,
	 id);
	
  printf("Index_reading_rand_output_count\t%llu\t%s\t%d\n",
	 stats.idx_reading_rand_output_count,
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

  printf("Internal_nodes_count\t%lu\t%s\t%d\n",
	 (stats.total_nodes_count - stats.leaf_nodes_count),
	 dataset,
	 id);

  printf("Leaf_nodes_count\t%lu\t%s\t%d\n",
	 stats.leaf_nodes_count,
	 dataset,
	 id);

  printf("Empty_leaf_nodes_count\t%lu\t%s\t%d\n",
	 stats.empty_leaf_nodes_count,	       
	 dataset,
	 id);

  printf("Total_nodes_count\t%lu\t%s\t%d\n",
	 stats.total_nodes_count,
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

  if (max_leaf_size != 0)
  {
    for (int i = 0; i < stats.leaves_counter; ++i)	  
      {
	double fill_factor = ((double) stats.leaves_sizes[i])/max_leaf_size;
	printf("Leaf_report_node_%d \t Height  %d  \t%s\t%d\n",
	       (i+1),
	       stats.leaves_heights[i],
	       dataset,
	       id);
	printf("Leaf_report_node_%d \t Fill_Factor  %f \t%s\t%d\n",
	       (i+1),
	       fill_factor,	       
	       dataset,
	       id);	    
      }	
  }
}

/*
void update_index_stats(isax_index *index, isax_node *node)
{

  unsigned int height = node->level +1;
  unsigned int threshold = index->settings->max_leaf_size;
  //at this point all time series are on disk
  unsigned int node_size = node->leaf_size;

  double node_fill_factor = (node_size * 100.0)/threshold;  
    
  if (node_fill_factor < stats.min_fill_factor)
    {
      stats.min_fill_factor = node_fill_factor;
    }
  if (node_fill_factor > stats.max_fill_factor)
    {
      stats.max_fill_factor = node_fill_factor;
    }
  if (height < stats.min_height)
    {
      stats.min_height = height;    
    }
  if (height > stats.max_height)
    {
      stats.max_height = height;
    }
  
  if(node_size == 0)
    {
      COUNT_EMPTY_LEAF_NODE
	}
  stats.sum_fill_factor += node_fill_factor ;
  stats.sum_squares_fill_factor += pow(node_fill_factor,2) ;

  stats.sum_height += height;
  stats.sum_squares_height += pow(height,2) ;
     
  COUNT_TOTAL_TS(node_size)
  
}
*/

void print_tlb_stats(unsigned int query_num, char * queries)
{

  printf("Query_avg_data_tlb\t%lf\t%s\t%u\n",
	 total_dft_tlb/total_ts_count,	       
	 queries,
	 query_num
	 );	
  printf("Total_ts_count\t%lu\t%s\t%u\n",
	 total_ts_count,	       
	 queries,
	 query_num);
	
}


void stats_init(void)
{
    /*INDEX STATISTICS*/
    stats.idx_building_input_time = 0;  
    stats.idx_building_output_time = 0;
    stats.idx_building_cpu_time = 0;
    stats.idx_building_total_time = 0;

    stats.idx_building_seq_input_count = 0;
    stats.idx_building_seq_output_count = 0;
    stats.idx_building_rand_input_count = 0;
    stats.idx_building_rand_output_count = 0;    

    stats.idx_writing_input_time = 0;  
    stats.idx_writing_output_time = 0;
    stats.idx_writing_cpu_time = 0;
    stats.idx_writing_total_time = 0;

    stats.idx_writing_seq_input_count = 0;
    stats.idx_writing_seq_output_count = 0;
    stats.idx_writing_rand_input_count = 0;
    stats.idx_writing_rand_output_count = 0;

    stats.idx_reading_input_time = 0;  
    stats.idx_reading_output_time = 0;
    stats.idx_reading_cpu_time = 0;
    stats.idx_reading_total_time = 0;
  
    stats.idx_reading_seq_input_count = 0;
    stats.idx_reading_seq_output_count = 0;
    stats.idx_reading_rand_input_count = 0;
    stats.idx_reading_rand_output_count = 0;
    
    stats.idx_total_input_time = 0;
    stats.idx_total_output_time = 0;
    stats.idx_total_cpu_time = 0;
    stats.idx_total_time = 0;

    stats.idx_total_seq_input_count = 0;
    stats.idx_total_seq_output_count = 0;
    stats.idx_total_rand_input_count = 0;
    stats.idx_total_rand_output_count = 0;    

    stats.total_nodes_count = 0;
    stats.leaf_nodes_count = 0;
    stats.empty_leaf_nodes_count = 0;
  
    stats.idx_size_bytes = 0;
    stats.idx_size_blocks = 0;
    
    
    /*PER QUERY STATISTICS*/    
    stats.query_filter_input_time = 0;
    stats.query_filter_output_time = 0;
    stats.query_filter_load_node_time = 0;
    stats.query_filter_cpu_time = 0;    
    stats.query_filter_total_time = 0;    

    stats.query_filter_seq_input_count = 0;
    stats.query_filter_seq_output_count = 0;
    stats.query_filter_rand_input_count = 0;
    stats.query_filter_rand_output_count = 0;    

    stats.query_filter_loaded_nodes_count = 0;
    stats.query_filter_checked_nodes_count = 0;    
    stats.query_filter_loaded_ts_count = 0;
    stats.query_filter_checked_ts_count = 0;

    stats.query_refine_input_time = 0;
    stats.query_refine_output_time = 0;
    stats.query_refine_load_node_time = 0;
    stats.query_refine_cpu_time = 0;
    stats.query_refine_total_time = 0;    

    stats.query_refine_seq_input_count = 0;
    stats.query_refine_seq_output_count = 0;
    stats.query_refine_rand_input_count = 0;
    stats.query_refine_rand_output_count = 0;    

    stats.query_refine_loaded_nodes_count = 0;
    stats.query_refine_checked_nodes_count = 0;    
    stats.query_refine_loaded_ts_count = 0;
    stats.query_refine_checked_ts_count = 0;

    stats.query_total_input_time = 0;
    stats.query_total_output_time = 0;
    stats.query_total_load_node_time = 0;
    stats.query_total_cpu_time = 0;
    stats.query_total_time = 0;    

    stats.query_total_seq_input_count = 0;
    stats.query_total_seq_output_count = 0;
    stats.query_total_rand_input_count = 0;
    stats.query_total_rand_output_count = 0;
    
    stats.query_total_loaded_nodes_count = 0;
    stats.query_total_checked_nodes_count = 0;    
    stats.query_total_loaded_ts_count = 0;
    stats.query_total_checked_ts_count = 0;

    
    stats.query_exact_distance = 0;
    stats.query_exact_node_filename = NULL;
    stats.query_exact_node_size = 0;    
    stats.query_exact_node_level = 0;

    stats.query_approx_distance = 0;
    stats.query_approx_node_filename = NULL;
    stats.query_approx_node_size = 0;
    stats.query_approx_node_level = 0;

    stats.query_lb_distance = 0;

    stats.query_tlb = 0;        
    stats.query_eff_epsilon = 0;    
    stats.query_pruning_ratio = 0;

    
    
    stats.tlb_ts_count = 0;
    stats.eff_epsilon_queries_count = 0;

    
    /*COMBINED STATISTICS FOR INDEXING AND QUERY WORKLOAD*/            
    stats.total_input_time = 0;
    stats.total_output_time = 0;
    stats.total_load_node_time = 0;
    stats.total_cpu_time = 0;
    stats.total_time = 0;
    stats.total_time_sanity_check = 0;
    
    stats.total_seq_input_count = 0;
    stats.total_seq_output_count = 0;
    stats.total_rand_input_count = 0;
    stats.total_rand_output_count = 0;
    
    stats.total_parse_time = 0;	
    stats.total_ts_count = 0;	
    
}

void stats_update_idx_reading_stats(void)
{
  stats.idx_reading_total_time = partial_time;
  stats.idx_reading_input_time = partial_input_time;  
  stats.idx_reading_output_time = partial_output_time;
  stats.idx_reading_cpu_time = partial_time -partial_input_time - partial_output_time;

  stats.idx_reading_seq_input_count = partial_seq_input_count;
  stats.idx_reading_seq_output_count = partial_seq_output_count;
  stats.idx_reading_rand_input_count = partial_rand_input_count;
  stats.idx_reading_rand_output_count = partial_rand_output_count;

}

void stats_update_idx_writing_stats(void)
{
  stats.idx_writing_total_time = partial_time;
  stats.idx_writing_input_time = partial_input_time;  
  stats.idx_writing_output_time = partial_output_time;
  stats.idx_writing_cpu_time = partial_time -partial_input_time - partial_output_time;

  stats.idx_writing_seq_input_count = partial_seq_input_count;
  stats.idx_writing_seq_output_count = partial_seq_output_count;
  stats.idx_writing_rand_input_count = partial_rand_input_count;
  stats.idx_writing_rand_output_count = partial_rand_output_count;

}

void stats_update_idx_building_stats(void)
{
  stats.idx_building_total_time = partial_time;
  stats.idx_building_input_time = partial_input_time;  
  stats.idx_building_output_time = partial_output_time;
  stats.idx_building_cpu_time = partial_time -partial_input_time - partial_output_time;

  stats.idx_building_seq_input_count = partial_seq_input_count;
  stats.idx_building_seq_output_count = partial_seq_output_count;
  stats.idx_building_rand_input_count = partial_rand_input_count;
  stats.idx_building_rand_output_count = partial_rand_output_count;
  stats.total_ts_count = total_ts_count;
}

void stats_update_query_exact_answer(ts_type distance)
{
  stats.query_refine_total_time  = partial_time;	
        
  stats.query_refine_input_time  = partial_input_time;
  stats.query_refine_output_time = partial_output_time;
  stats.query_refine_load_node_time = partial_load_node_time;    
  stats.query_refine_cpu_time    = partial_time
    - partial_input_time
    - partial_output_time;
  stats.query_refine_seq_input_count   = partial_seq_input_count;
  stats.query_refine_seq_output_count  = partial_seq_output_count;
  stats.query_refine_rand_input_count  = partial_rand_input_count;
  stats.query_refine_rand_output_count = partial_rand_output_count;

  stats.query_total_time  = partial_time
    +  stats.query_filter_total_time;
  stats.query_total_input_time  = partial_input_time
    +  stats.query_filter_input_time;
  stats.query_total_output_time  = partial_output_time
    +  stats.query_filter_output_time;
  stats.query_total_load_node_time  = partial_load_node_time
    +  stats.query_filter_load_node_time;
  stats.query_total_cpu_time  =  stats.query_total_time
    -  stats.query_total_input_time
    -  stats.query_total_output_time;
    
  stats.query_total_seq_input_count   = partial_seq_input_count
    + stats.query_filter_seq_input_count;
  stats.query_total_seq_output_count   = partial_seq_output_count
    + stats.query_filter_seq_output_count;
  stats.query_total_rand_input_count   = partial_rand_input_count
    + stats.query_filter_rand_input_count;
  stats.query_total_rand_output_count   = partial_rand_output_count
    + stats.query_filter_rand_output_count;
    
  //stats.query_refine_loaded_nodes_count = loaded_nodes_count;
  stats.query_refine_loaded_ts_count = loaded_ts_count;
  //stats.query_refine_checked_nodes_count = checked_nodes_count;
  stats.query_refine_checked_ts_count = loaded_ts_count;
  
  stats.query_total_loaded_nodes_count = loaded_nodes_count 
    + stats.query_filter_loaded_nodes_count;
  stats.query_total_loaded_ts_count = loaded_ts_count
    + stats.query_filter_loaded_ts_count;
  stats.query_total_checked_nodes_count = checked_nodes_count
    + stats.query_filter_checked_nodes_count;
  stats.query_total_checked_ts_count = loaded_ts_count
    + stats.query_filter_checked_ts_count;


  stats.query_exact_distance = sqrtf(distance);


  if (stats.total_ts_count != 0)
    {
      stats.query_pruning_ratio = 1.0 - ((double)stats.query_total_checked_ts_count/
					 stats.total_ts_count);
    }
    

  if (stats.query_exact_distance != 0)
    {
      stats.query_eff_epsilon =   (stats.query_approx_distance
				   -stats.query_exact_distance
				   )/ stats.query_exact_distance;
    }
  
}


void stats_update_knn_query_approx_answer(ts_type distance, boolean user_bsf)
{

  if (user_bsf)
    {
      stats.query_filter_total_time  = 0;	
		  
      stats.query_filter_input_time  = 0;
      stats.query_filter_output_time = 0;
      stats.query_filter_load_node_time = 0;
      stats.query_filter_cpu_time    = 0;
      stats.query_filter_seq_input_count   = 0;
      stats.query_filter_seq_output_count  = 0;
      stats.query_filter_rand_input_count  = 0;
      stats.query_filter_rand_output_count = 0;

      stats.query_filter_loaded_nodes_count = 0;
      stats.query_filter_loaded_ts_count = 0;
      stats.query_filter_checked_nodes_count = 0;
      stats.query_filter_checked_ts_count = 0;
	    
      stats.query_approx_distance = sqrtf(distance);
      stats.query_approx_node_filename = NULL;
      stats.query_approx_node_size = NULL;
      stats.query_approx_node_level = 0;
    }
  else
    {
      stats.query_filter_total_time  = partial_time;	
    
      stats.query_filter_input_time  = partial_input_time;
      stats.query_filter_output_time = partial_output_time;
      stats.query_filter_load_node_time = partial_load_node_time;
      stats.query_filter_cpu_time    = partial_time-partial_input_time-partial_output_time;
      stats.query_filter_seq_input_count   = partial_seq_input_count;
      stats.query_filter_seq_output_count  = partial_seq_output_count;
      stats.query_filter_rand_input_count  = partial_rand_input_count;
      stats.query_filter_rand_output_count = partial_rand_output_count;

      stats.query_filter_loaded_nodes_count = loaded_nodes_count;
      stats.query_filter_loaded_ts_count = loaded_ts_count;
      stats.query_filter_checked_nodes_count = checked_nodes_count;
      stats.query_filter_checked_ts_count = checked_ts_count;

      stats.queries_filter_total_time  +=  stats.query_filter_total_time;
    
      stats.queries_filter_input_time  +=  stats.query_filter_input_time;
      stats.queries_filter_output_time +=  stats.query_filter_output_time;
      stats.queries_filter_load_node_time += stats.query_filter_load_node_time;
      stats.queries_filter_cpu_time    += stats.query_filter_cpu_time;
 
      stats.queries_filter_seq_input_count   += stats.query_filter_seq_input_count;
      stats.queries_filter_seq_output_count  += stats.query_filter_seq_output_count;
      stats.queries_filter_rand_input_count  += stats.query_filter_rand_input_count;
      stats.queries_filter_rand_output_count += stats.query_filter_rand_output_count;
    
      stats.query_approx_distance = sqrtf(distance);

      /*
      if (distance == 0) {
	stats_update_query_exact_answer(distance);
	//stats.query_exact_node_filename = approximate_result.node->filename;
	//stats.query_exact_node_size = approximate_result.node->leaf_size;;
	//stats.query_exact_node_level = approximate_result.node->level; 
      }
      */

    }

}

void stats_update_query_approx_answer(ts_type distance, boolean user_bsf)
{

  if (user_bsf)
    {
      stats.query_filter_total_time  = 0;	
		  
      stats.query_filter_input_time  = 0;
      stats.query_filter_output_time = 0;
      stats.query_filter_load_node_time = 0;
      stats.query_filter_cpu_time    = 0;
      stats.query_filter_seq_input_count   = 0;
      stats.query_filter_seq_output_count  = 0;
      stats.query_filter_rand_input_count  = 0;
      stats.query_filter_rand_output_count = 0;

      stats.query_filter_loaded_nodes_count = 0;
      stats.query_filter_loaded_ts_count = 0;
      stats.query_filter_checked_nodes_count = 0;
      stats.query_filter_checked_ts_count = 0;
	    
      stats.query_approx_distance = sqrtf(distance);
      stats.query_approx_node_filename = NULL;
      stats.query_approx_node_size = NULL;
      stats.query_approx_node_level = 0;
    }
  else
    {
      stats.query_filter_total_time  = partial_time;	
    
      stats.query_filter_input_time  = partial_input_time;
      stats.query_filter_output_time = partial_output_time;
      stats.query_filter_load_node_time = partial_load_node_time;
      stats.query_filter_cpu_time    = partial_time-partial_input_time-partial_output_time;
      stats.query_filter_seq_input_count   = partial_seq_input_count;
      stats.query_filter_seq_output_count  = partial_seq_output_count;
      stats.query_filter_rand_input_count  = partial_rand_input_count;
      stats.query_filter_rand_output_count = partial_rand_output_count;

      stats.query_filter_loaded_nodes_count = loaded_nodes_count;
      stats.query_filter_loaded_ts_count = loaded_ts_count;
      stats.query_filter_checked_nodes_count = checked_nodes_count;
      stats.query_filter_checked_ts_count = checked_ts_count;

      stats.queries_filter_total_time  +=  stats.query_filter_total_time;
    
      stats.queries_filter_input_time  +=  stats.query_filter_input_time;
      stats.queries_filter_output_time +=  stats.query_filter_output_time;
      stats.queries_filter_load_node_time += stats.query_filter_load_node_time;
      stats.queries_filter_cpu_time    += stats.query_filter_cpu_time;
 
      stats.queries_filter_seq_input_count   += stats.query_filter_seq_input_count;
      stats.queries_filter_seq_output_count  += stats.query_filter_seq_output_count;
      stats.queries_filter_rand_input_count  += stats.query_filter_rand_input_count;
      stats.queries_filter_rand_output_count += stats.query_filter_rand_output_count;
    
      stats.query_approx_distance = sqrtf(distance);

      /*
      if (distance == 0) {
	stats_update_query_exact_answer(distance);
	//stats.query_exact_node_filename = approximate_result.node->filename;
	//stats.query_exact_node_size = approximate_result.node->leaf_size;;
	//stats.query_exact_node_level = approximate_result.node->level; 
      }
      */

    }

}

void stats_update_knn_query_exact_answer(unsigned int query_id,
					 unsigned int found_knn,
					 ts_type distance)
{
  stats.query_refine_total_time  = partial_time;	
        
  stats.query_refine_input_time  = partial_input_time;
  stats.query_refine_output_time = partial_output_time;
  stats.query_refine_load_node_time = partial_load_node_time;    
  stats.query_refine_cpu_time    = partial_time
    - partial_input_time
    - partial_output_time;
  stats.query_refine_seq_input_count   = partial_seq_input_count;
  stats.query_refine_seq_output_count  = partial_seq_output_count;
  stats.query_refine_rand_input_count  = partial_rand_input_count;
  stats.query_refine_rand_output_count = partial_rand_output_count;

  //stats.query_refine_loaded_nodes_count = loaded_nodes_count;
  stats.query_refine_loaded_ts_count = loaded_ts_count;
  //stats.query_refine_checked_nodes_count = checked_nodes_count;
  stats.query_refine_checked_ts_count = loaded_ts_count;

  if (found_knn == 1)
  {
    stats.query_total_time  = partial_time
      +  stats.query_filter_total_time;
    stats.query_total_input_time  = partial_input_time
      +  stats.query_filter_input_time;
    stats.query_total_output_time  = partial_output_time
      +  stats.query_filter_output_time;
    stats.query_total_load_node_time  = partial_load_node_time
      +  stats.query_filter_load_node_time;
    stats.query_total_cpu_time  =  stats.query_total_time
      -  stats.query_total_input_time
      -  stats.query_total_output_time;
    
    stats.query_total_seq_input_count   = partial_seq_input_count
      + stats.query_filter_seq_input_count;
    stats.query_total_seq_output_count   = partial_seq_output_count
      + stats.query_filter_seq_output_count;
    stats.query_total_rand_input_count   = partial_rand_input_count
      + stats.query_filter_rand_input_count;
    stats.query_total_rand_output_count   = partial_rand_output_count
      + stats.query_filter_rand_output_count;
  
  
    stats.query_total_loaded_nodes_count = loaded_nodes_count 
      + stats.query_filter_loaded_nodes_count;
    stats.query_total_loaded_ts_count = loaded_ts_count
      + stats.query_filter_loaded_ts_count;
    stats.query_total_checked_nodes_count = checked_nodes_count
      + stats.query_filter_checked_nodes_count;
    stats.query_total_checked_ts_count = loaded_ts_count
      + stats.query_filter_checked_ts_count;
  }

  else
  {
    stats.query_total_time  = partial_time;
    stats.query_total_input_time  = partial_input_time;
    stats.query_total_output_time  = partial_output_time;
    stats.query_total_load_node_time  = partial_load_node_time;
    stats.query_total_cpu_time  =  stats.query_total_time
      -  stats.query_total_input_time
      -  stats.query_total_output_time;
    
    stats.query_total_seq_input_count   = partial_seq_input_count;
    stats.query_total_seq_output_count   = partial_seq_output_count;
    stats.query_total_rand_input_count   = partial_rand_input_count;
    stats.query_total_rand_output_count   = partial_rand_output_count;
  
  
    stats.query_total_loaded_nodes_count = loaded_nodes_count; 
    stats.query_total_loaded_ts_count = loaded_ts_count;
    stats.query_total_checked_nodes_count = checked_nodes_count;
    stats.query_total_checked_ts_count = loaded_ts_count;
   
  }
  
  stats.query_exact_distance = sqrtf(distance);

  
}
