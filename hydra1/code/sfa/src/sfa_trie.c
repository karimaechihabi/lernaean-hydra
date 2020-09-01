//  
//  sfa_trie.c
//  sfa  C version
//
//  Created by Karima Echihabi on 18/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef VALUES
#include <values.h>
#endif

#include <sys/stat.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/sfa_trie.h"
#include "../include/sfa_node.h"
#include "../include/calc_utils.h"
#include "../include/sfa_file_buffer_manager.h"
#include "../include/sfa_file_buffer.h"

/**
 This function initializes the settings of a sfa trie
 */
struct sfa_trie_settings * sfa_trie_settings_init(const char * root_directory,
						 unsigned int timeseries_size, 
                                                 unsigned int word_length, 
                                                 unsigned int num_symbols,
						 unsigned int max_leaf_size,
						 double buffered_memory_size,
						 boolean is_norm,
	            				 unsigned int  histogram_type,
	            				 unsigned int  lb_dist,						  
						 unsigned int sample_size,
						 boolean is_trie_new)
{

  if (is_trie_new)
  {
    if(chdir(root_directory) == 0)
    {
        fprintf(stderr, "WARNING! Target trie directory already exists. Please delete or choose a new one.\n");
        exit(-1);
    }
  }
    mkdir(root_directory, 0777);
  
    struct sfa_trie_settings *settings = malloc(sizeof(struct sfa_trie_settings));
    if(settings == NULL) {
        fprintf(stderr,"Error in sfa_trie.c: could not allocate memory for trie settings.\n");
        return NULL;
    }


    settings->root_directory = root_directory;
    settings->timeseries_size = timeseries_size;
    settings->max_leaf_size = max_leaf_size;
    settings->buffered_memory_size = buffered_memory_size;

    //settings->word_length = word_length;
    settings->num_symbols = num_symbols;
    settings->fft_size =  fminf(timeseries_size, word_length);
    settings->norm_factor = 1/sqrtf(timeseries_size);
    
    settings->sample_size = sample_size;
    
    settings->is_norm = is_norm;
    
    settings->histogram_type = histogram_type; // 0= equi_depth, 1=equi_frequency/width
    settings->lb_dist = lb_dist; // 0= equi_depth, 1=equi_frequency/width
    
    if (is_norm)
      settings->start_offset = 2;
    else
      settings->start_offset = 0;

    
    /* Each leaf node has a file called: 
       childIndexLevel1_childIndexLevel2_._._._._childIndexLastLevel.ts
       childIndexLevel1_childIndexLevel2_._._._._childIndexLastLevel.ts
       childIndexLevel1_childIndexLevel2_._._._._childIndexLastLevel.ts

       child_index at each level cannot exceed number of symbols, default is 8
                     but we will use max of 256 (max 3 characters)  
       level :  cannot exceed number of fourier coefficients 

       number of punctuation marks (undescrores): total underscores: max_level-1
       null character: 1 

       extension: .ts, .fft, .sfa: max = 4

       The root cannot be a leaf. The smallest trie will contain the root
       and at least one level of children. 

    */

     float child_index_len = 3; //ceil(log10())+1;
     float prefix_len  = ceil(log10(num_symbols))+1;

     settings->max_filename_size = settings->fft_size * (child_index_len+1) +  //index and dot
                                   4 +  //extension
                                   1; //null character
     
    return settings;
}



/**
 This function initializes an isax trie
 @param isax_trie_settings *settings
 @return isax_trie
 */
struct sfa_trie * sfa_trie_init(struct sfa_trie_settings *settings)
{
    struct sfa_trie *trie = malloc(sizeof(struct sfa_trie));
    if(trie == NULL) {
        fprintf(stderr,"Error in sfa_trie.c: Could not allocate memory for trie structure.\n");
        return NULL;
    }

    
    trie->settings = settings;
    trie->total_records = 0;

    sfa_init_stats(trie);
    
    if (!init_file_buffer_manager(trie))
    { 
      fprintf(stderr, "Error in sfa_trie.c:  Could not initialize the \
                       file buffer manager for this trie.\n");
      return NULL;              
    }

    sfa_trie_bins_init(trie);
    
    return trie;
}


/*
  This functions allocates a two dimensional array
  of num_words rows and (num_symbols-1) columns
  The array will contain the discretization
  intervals
*/

enum response sfa_trie_bins_init(struct sfa_trie * trie)
{
  
  trie->bins = NULL;  
  trie->bins = calloc(trie->settings->fft_size,sizeof(ts_type*));
  if(trie == NULL) {
        fprintf(stderr,"Error in sfa_trie.c: Could not allocate memory for trie structure.\n");
        return FAILURE;
  }
  //allocate num_symbols-1 memory slots for each word
  for (int i = 0; i < trie->settings->fft_size; ++i)
  {
    trie->bins[i] = NULL;
    trie->bins[i] = calloc(trie->settings->num_symbols-1,sizeof(ts_type));
    if(trie == NULL) {
        fprintf(stderr,"Error in sfa_trie.c: Could not allocate memory for trie structure.\n");
        return FAILURE;
    }
    for (int j = 0; j < trie->settings->num_symbols-1; ++j)
    {
      trie->bins[i][j] = FLT_MAX;
    }

  }

  fprintf(stderr,"Initialized bins[%d][%d] \n", trie->settings->fft_size, trie->settings->num_symbols-1 );      
  return SUCCESS;

}

enum response sfa_init_stats(struct sfa_trie * trie)
{
    trie->stats = malloc(sizeof(struct stats_info));
    if(trie->stats == NULL) {
        fprintf(stderr,"Error in sfa_trie.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    /*TRIE STATISTICS*/
    trie->stats->idx_building_input_time = 0;  
    trie->stats->idx_building_output_time = 0;
    trie->stats->idx_building_cpu_time = 0;
    trie->stats->idx_building_total_time = 0;

    trie->stats->idx_building_seq_input_count = 0;
    trie->stats->idx_building_seq_output_count = 0;
    trie->stats->idx_building_rand_input_count = 0;
    trie->stats->idx_building_rand_output_count = 0;    

    trie->stats->idx_writing_input_time = 0;  
    trie->stats->idx_writing_output_time = 0;
    trie->stats->idx_writing_cpu_time = 0;
    trie->stats->idx_writing_total_time = 0;

    trie->stats->idx_writing_seq_input_count = 0;
    trie->stats->idx_writing_seq_output_count = 0;
    trie->stats->idx_writing_rand_input_count = 0;
    trie->stats->idx_writing_rand_output_count = 0;

    trie->stats->idx_reading_input_time = 0;  
    trie->stats->idx_reading_output_time = 0;
    trie->stats->idx_reading_cpu_time = 0;
    trie->stats->idx_reading_total_time = 0;
  
    trie->stats->idx_reading_seq_input_count = 0;
    trie->stats->idx_reading_seq_output_count = 0;
    trie->stats->idx_reading_rand_input_count = 0;
    trie->stats->idx_reading_rand_output_count = 0;
    
    trie->stats->idx_total_input_time = 0;
    trie->stats->idx_total_output_time = 0;
    trie->stats->idx_total_cpu_time = 0;
    trie->stats->idx_total_time = 0;

    trie->stats->idx_total_seq_input_count = 0;
    trie->stats->idx_total_seq_output_count = 0;
    trie->stats->idx_total_rand_input_count = 0;
    trie->stats->idx_total_rand_output_count = 0;    

    trie->stats->total_nodes_count = 0;
    trie->stats->leaf_nodes_count = 0;
    trie->stats->empty_leaf_nodes_count = 0;
  
    trie->stats->idx_size_bytes = 0;
    trie->stats->idx_size_blocks = 0;
    
    trie->stats->min_fill_factor = FLT_MAX;
    trie->stats->max_fill_factor = 0;
    trie->stats->sum_fill_factor = 0;
    trie->stats->sum_squares_fill_factor = 0;
    trie->stats->avg_fill_factor = 0;
    trie->stats->sd_fill_factor = 0;

    trie->stats->min_height = FLT_MAX;
    trie->stats->max_height = 0;
    trie->stats->sum_height = 0;
    trie->stats->sum_squares_height = 0;    
    trie->stats->avg_height = 0;
    trie->stats->sd_height = 0;
    
    /*PER QUERY STATISTICS*/    
    trie->stats->query_filter_input_time = 0;
    trie->stats->query_filter_output_time = 0;
    trie->stats->query_filter_load_node_time = 0;
    trie->stats->query_filter_cpu_time = 0;    
    trie->stats->query_filter_total_time = 0;    

    trie->stats->query_filter_seq_input_count = 0;
    trie->stats->query_filter_seq_output_count = 0;
    trie->stats->query_filter_rand_input_count = 0;
    trie->stats->query_filter_rand_output_count = 0;    

    trie->stats->query_filter_loaded_nodes_count = 0;
    trie->stats->query_filter_checked_nodes_count = 0;    
    trie->stats->query_filter_loaded_ts_count = 0;
    trie->stats->query_filter_checked_ts_count = 0;

    trie->stats->query_refine_input_time = 0;
    trie->stats->query_refine_output_time = 0;
    trie->stats->query_refine_load_node_time = 0;
    trie->stats->query_refine_cpu_time = 0;
    trie->stats->query_refine_total_time = 0;    

    trie->stats->query_refine_seq_input_count = 0;
    trie->stats->query_refine_seq_output_count = 0;
    trie->stats->query_refine_rand_input_count = 0;
    trie->stats->query_refine_rand_output_count = 0;    

    trie->stats->query_refine_loaded_nodes_count = 0;
    trie->stats->query_refine_checked_nodes_count = 0;    
    trie->stats->query_refine_loaded_ts_count = 0;
    trie->stats->query_refine_checked_ts_count = 0;

    trie->stats->query_total_input_time = 0;
    trie->stats->query_total_output_time = 0;
    trie->stats->query_total_load_node_time = 0;
    trie->stats->query_total_cpu_time = 0;
    trie->stats->query_total_time = 0;    

    trie->stats->query_total_seq_input_count = 0;
    trie->stats->query_total_seq_output_count = 0;
    trie->stats->query_total_rand_input_count = 0;
    trie->stats->query_total_rand_output_count = 0;
    
    trie->stats->query_total_loaded_nodes_count = 0;
    trie->stats->query_total_checked_nodes_count = 0;    
    trie->stats->query_total_loaded_ts_count = 0;
    trie->stats->query_total_checked_ts_count = 0;
    
    trie->stats->query_exact_distance = 0;
    trie->stats->query_exact_node_filename = NULL;
    trie->stats->query_exact_node_size = 0;    
    trie->stats->query_exact_node_level = 0;

    trie->stats->query_approx_distance = 0;
    trie->stats->query_approx_node_filename = NULL;
    trie->stats->query_approx_node_size = 0;
    trie->stats->query_approx_node_level = 0;

    trie->stats->query_lb_distance = 0;

    trie->stats->query_tlb = 0;        
    trie->stats->query_eff_epsilon = 0;    
    trie->stats->query_pruning_ratio = 0;

    
    /*SUMMARY STATISTICS FOR ALL QUERIES*/        
    trie->stats->queries_refine_input_time = 0;
    trie->stats->queries_refine_output_time = 0;
    trie->stats->queries_refine_load_node_time = 0;
    trie->stats->queries_refine_cpu_time = 0;
    trie->stats->queries_refine_total_time = 0;    

    trie->stats->queries_refine_seq_input_count = 0;
    trie->stats->queries_refine_seq_output_count = 0;
    trie->stats->queries_refine_rand_input_count = 0;
    trie->stats->queries_refine_rand_output_count = 0;        
    
    trie->stats->queries_filter_input_time = 0;
    trie->stats->queries_filter_output_time = 0;
    trie->stats->queries_filter_load_node_time = 0;
    trie->stats->queries_filter_cpu_time = 0;
    trie->stats->queries_filter_total_time = 0;    

    trie->stats->queries_filter_seq_input_count = 0;
    trie->stats->queries_filter_seq_output_count = 0;
    trie->stats->queries_filter_rand_input_count = 0;
    trie->stats->queries_filter_rand_output_count = 0;    

    trie->stats->queries_total_input_time = 0;
    trie->stats->queries_total_output_time = 0;
    trie->stats->queries_total_load_node_time = 0;    
    trie->stats->queries_total_cpu_time = 0;
    trie->stats->queries_total_time = 0;    

    trie->stats->queries_total_seq_input_count = 0;
    trie->stats->queries_total_seq_output_count = 0;
    trie->stats->queries_total_rand_input_count = 0;
    trie->stats->queries_total_rand_output_count = 0;        

    trie->stats->queries_min_eff_epsilon = FLT_MAX;
    trie->stats->queries_max_eff_epsilon = 0;
    trie->stats->queries_sum_eff_epsilon = 0;
    trie->stats->queries_sum_squares_eff_epsilon = 0;
    trie->stats->queries_avg_eff_epsilon = 0;
    trie->stats->queries_sd_eff_epsilon = 0;
    
    trie->stats->queries_min_pruning_ratio =  FLT_MAX;
    trie->stats->queries_max_pruning_ratio = 0;
    trie->stats->queries_sum_pruning_ratio = 0;
    trie->stats->queries_sum_squares_pruning_ratio = 0;
    trie->stats->queries_avg_pruning_ratio = 0;
    trie->stats->queries_sd_pruning_ratio = 0;

    trie->stats->queries_min_tlb =  FLT_MAX;
    trie->stats->queries_max_tlb = 0;
    trie->stats->queries_sum_tlb = 0;
    trie->stats->queries_sum_squares_tlb = 0;
    trie->stats->queries_avg_tlb = 0;
    trie->stats->queries_sd_tlb = 0;    


    trie->stats->tlb_ts_count = 0;
    trie->stats->eff_epsilon_queries_count = 0;

    
//    trie->stats->total_queries_count = 0;
    
    /*COMBINED STATISTICS FOR TRIEING AND QUERY WORKLOAD*/            
    trie->stats->total_input_time = 0;
    trie->stats->total_output_time = 0;
    trie->stats->total_load_node_time = 0;
    trie->stats->total_cpu_time = 0;
    trie->stats->total_time = 0;
    trie->stats->total_time_sanity_check = 0;
    
    trie->stats->total_seq_input_count = 0;
    trie->stats->total_seq_output_count = 0;
    trie->stats->total_rand_input_count = 0;
    trie->stats->total_rand_output_count = 0;
    
    trie->stats->total_parse_time = 0;	
    trie->stats->total_ts_count = 0;	
    
    return SUCCESS;
}

void sfa_get_trie_stats(struct sfa_trie * trie)
{
  trie->stats->total_seq_input_count = trie->stats->idx_building_seq_input_count
                                      + trie->stats->idx_writing_seq_input_count
                                      + trie->stats->idx_reading_seq_input_count;
  trie->stats->total_seq_output_count = trie->stats->idx_building_seq_output_count
                                      + trie->stats->idx_writing_seq_output_count
                                      + trie->stats->idx_reading_seq_output_count;
  trie->stats->total_rand_input_count = trie->stats->idx_building_rand_input_count
                                      + trie->stats->idx_writing_rand_input_count
                                      + trie->stats->idx_reading_rand_input_count;
  trie->stats->total_rand_output_count = trie->stats->idx_building_rand_output_count
                                      + trie->stats->idx_writing_rand_output_count
                                      + trie->stats->idx_reading_rand_output_count;
  
  trie->stats->total_input_time = trie->stats->idx_building_input_time
                                 + trie->stats->idx_writing_input_time
                                  + trie->stats->idx_reading_input_time;
  trie->stats->total_output_time = trie->stats->idx_building_output_time
                                  + trie->stats->idx_writing_output_time
                                  + trie->stats->idx_reading_output_time;
  trie->stats->total_cpu_time    = trie->stats->idx_building_cpu_time
                                  + trie->stats->idx_writing_cpu_time
                                  + trie->stats->idx_reading_cpu_time;

  trie->stats->total_time    = trie->stats->total_input_time
                              + trie->stats->total_output_time
                              + trie->stats->total_cpu_time;

  
  trie->stats->total_parse_time = total_parse_time;

  trie->stats->leaf_nodes_count = leaf_nodes_count;
  trie->stats->empty_leaf_nodes_count = empty_leaf_nodes_count;
  
  trie->stats->total_nodes_count = total_nodes_count - empty_leaf_nodes_count;
  trie->stats->total_ts_count = total_ts_count;

  sfa_get_trie_footprint(trie);

}


void sfa_get_trie_footprint(struct sfa_trie * trie)
{

    const char *filename = malloc(sizeof(char) * (strlen(trie->settings->root_directory) + 9));
    filename = strcpy(filename, trie->settings->root_directory);
    filename = strcat(filename, "root.idx\0");

    struct stat st;
    unsigned int  count_leaves;

    if (stat(filename, &st) == 0)
    {
      trie->stats->idx_size_bytes = (long long) st.st_size;
      trie->stats->idx_size_blocks = (long long) st.st_blksize;
    }

    count_leaves = trie->stats->leaf_nodes_count;
    
    trie->stats->avg_fill_factor =  ((double) trie->stats->sum_fill_factor) / count_leaves  ;
    trie->stats->sum_squares_fill_factor -= (pow( trie->stats->sum_fill_factor,2) / count_leaves);
    trie->stats->sd_fill_factor =  sqrt(((double) trie->stats->sum_squares_fill_factor) / count_leaves);

    trie->stats->avg_height     =  ((double) trie->stats->sum_height) / count_leaves;
    trie->stats->sum_squares_height -= (pow( trie->stats->sum_height,2) / count_leaves);
    trie->stats->sd_height =  sqrt(((double) trie->stats->sum_squares_height) / count_leaves);

    free(filename);
}

  
void sfa_print_trie_stats(struct sfa_trie * trie, char * dataset)
{
        /*
        printf("------------------------ \n");    
        printf("TRIE SUMMARY STATISTICS \n");
        printf("------------------------ \n");
        */	
       //  id = -1 for trie and id = query_id for queries
        int id = -1;
        printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_building_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_building_output_time/1000000,
	       dataset,
	       id);
        printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_building_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_building_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_building_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_building_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_building_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_building_rand_output_count,
	       dataset,
	       id); 

        printf("Index_writing_input_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_writing_input_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_output_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_writing_output_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_cpu_time_secs\t%lf\t%s\t%d\n",
               trie->stats->idx_writing_cpu_time/1000000,
	       dataset,
	       id);
	
        printf("Index_writing_total_time_secs\t%lf\t%s\t%d\n",
               trie->stats->idx_writing_total_time/1000000,
	       dataset,
	       id);	

        printf("Index_writing_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_writing_seq_input_count,
	       dataset,
	       id);

        printf("Index_writing_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_writing_seq_output_count,
	       dataset,
	       id);

        printf("Index_writing_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_writing_rand_input_count,
	       dataset,
	       id);

        printf("Index_writing_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_writing_rand_output_count,
	       dataset,
	       id);	

        printf("Index_reading_input_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_reading_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_reading_output_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_reading_output_time/1000000,
	       dataset,
	       id);
        printf("Index_reading_cpu_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_reading_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_reading_total_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->idx_reading_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_reading_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_reading_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_reading_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_reading_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->idx_reading_rand_output_count,
	       dataset,
	       id);
	
        printf("Index_total_input_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_total_output_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_total_time_secs\t%lf\t%s\t%d\n",
	       trie->stats->total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_total_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_total_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->total_rand_output_count,
	       dataset,
	       id); 

        printf("Internal_nodes_count\t%lu\t%s\t%d\n",
	       (trie->stats->total_nodes_count - trie->stats->leaf_nodes_count),
	       dataset,
	       id);

        printf("Leaf_nodes_count\t%lu\t%s\t%d\n",
	       trie->stats->leaf_nodes_count,
	       dataset,
	       id);

	printf("Empty_leaf_nodes_count\t%lu\t%s\t%d\n",
               trie->stats->empty_leaf_nodes_count,	       
	       dataset,
	       id);

	printf("Total_nodes_count\t%lu\t%s\t%d\n",
	       trie->stats->total_nodes_count,
	       dataset,
	       id);

	double size_MB =  (trie->stats->idx_size_bytes)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);

	printf("Minimum_fill_factor\t%f\t%s\t%d\n",
	       trie->stats->min_fill_factor,	       
	       dataset,
	       id);	

	printf("Maximum_fill_factor\t%f\t%s\t%d\n",
	       trie->stats->max_fill_factor,	       
	       dataset,
	       id);

	printf("Average_fill_factor\t%f\t%s\t%d\n",
	       trie->stats->avg_fill_factor,	       
	       dataset,
	       id);	

	printf("SD_height\t%f\t%s\t%d\n",
	       trie->stats->sd_height,	       
	       dataset,
	       id);		

	printf("Minimum_height\t%u\t%s\t%d\n",
	       trie->stats->min_height,	       
	       dataset,
	       id);	

	printf("Maximum_height\t%u\t%s\t%d\n",
	       trie->stats->max_height,	       
	       dataset,
	       id);

	printf("Average_height\t%f\t%s\t%d\n",
	       trie->stats->avg_height,	       
	       dataset,
	       id);	

	printf("SD_height\t%f\t%s\t%d\n",
	       trie->stats->sd_height,	       
	       dataset,
	       id);			

	printf("Total_ts_count\t%u\t%s\t%d\n",
	       trie->stats->total_ts_count,	       
	       dataset,
	       id);

	for (int i = 0; i < trie->stats->leaves_counter; ++i)	  
	{
 	    double fill_factor = ((double) trie->stats->leaves_sizes[i])/trie->settings->max_leaf_size;
  	    printf("Leaf_report_node_%d \t Height  %d  \t%s\t%d\n",
		   (i+1),
		   trie->stats->leaves_heights[i],
	           dataset,
	           id);
  	    printf("Leaf_report_node_%d \t Fill_Factor  %f \t%s\t%d\n",
		   (i+1),
		   fill_factor,	       
	           dataset,
	           id);	    
	}
	
}


void get_query_stats(struct sfa_trie * trie, ts_type * ts, ts_type * paa, int query_num)
{

  if (trie->stats->total_ts_count != 0)
    {
        trie->stats->query_pruning_ratio = 1.0 - ((double)trie->stats->query_total_checked_ts_count/
						 trie->stats->total_ts_count);
        double ratio = trie->stats->query_pruning_ratio;
        trie->stats->queries_sum_pruning_ratio += ratio;
        trie->stats->queries_sum_squares_pruning_ratio += pow(ratio,2);

        if (ratio < trie->stats->queries_min_pruning_ratio)
        {
  	   trie->stats->queries_min_pruning_ratio = ratio;
        }
        if (ratio > trie->stats->queries_max_pruning_ratio)
        {
  	   trie->stats->queries_max_pruning_ratio = ratio;
        }
    }
    

    if (trie->stats->query_exact_distance != 0)
    {
        trie->stats->query_eff_epsilon =   (trie->stats->query_approx_distance
					    -trie->stats->query_exact_distance
					    )
					    / trie->stats->query_exact_distance;

        double eff_epsilon = trie->stats->query_eff_epsilon;
	
        trie->stats->queries_sum_eff_epsilon += eff_epsilon;

        trie->stats->queries_sum_squares_eff_epsilon += pow(eff_epsilon,2);

	
        if (eff_epsilon < trie->stats->queries_min_eff_epsilon)
        {
  	   trie->stats->queries_min_eff_epsilon = eff_epsilon;
        }
        if (eff_epsilon > trie->stats->queries_min_eff_epsilon)
        {
  	   trie->stats->queries_max_eff_epsilon = eff_epsilon;
        }	
    }
    else
    {  
       ++(trie->stats->eff_epsilon_queries_count);
    }


    if (trie->stats->tlb_ts_count != 0)
    {
 	trie->stats->query_tlb = trie->stats->query_sum_tlb/trie->stats->tlb_ts_count; //the average over all the data set
  
        double tlb  = trie->stats->query_tlb;

        trie->stats->queries_sum_tlb += tlb;
        trie->stats->queries_sum_squares_tlb += pow(tlb,2);	

        if (tlb < trie->stats->queries_min_tlb)
        {
  	   trie->stats->queries_min_tlb = tlb;
        }
        if (tlb > trie->stats->queries_min_tlb)
        {
  	   trie->stats->queries_max_tlb = tlb;
        }			
    }

}


void print_tlb_stats(struct sfa_trie * trie, unsigned int query_num, char * queries)
{

        printf("Query_avg_sfa_tlb\t%lf\t%s\t%u\n",
	       total_sfa_tlb/total_ts_count,	       
	       queries,
	       query_num
	     );
        printf("Query_avg_dft_tlb\t%lf\t%s\t%u\n",
	       total_dft_tlb/total_ts_count,	       
	       queries,
	       query_num
	     );	
	printf("Leaf_nodes_count\t%lu\t%s\t%u\n",
	       leaf_nodes_count,	       
	       queries,
	       query_num);
	
	printf("Total_ts_count\t%lu\t%s\t%u\n",
	       total_ts_count,	       
	       queries,
	       query_num);
	
}

void print_query_stats(struct sfa_trie * trie, unsigned int query_num, char * queries)
{
        /*
        printf("-----------------\n");
        printf("QUERY STATISTICS \n");
        printf("QUERY: %d \n", query_num);
        printf("-----------------\n");
	*/

        printf("Query_filter_input_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_filter_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_output_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_filter_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_filter_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_filter_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_filter_total_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_filter_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_rand_output_count,
	       queries,
	       query_num
	       );
        printf("Query_filter_checked_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_filter_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_checked_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_filter_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_filter_loaded_ts_count,
	       queries,
	       query_num
	       );
	

        printf("Query_refine_input_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_refine_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_output_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_refine_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_refine_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_refine_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_refine_total_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_refine_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_rand_output_count,
	       queries,
	       query_num
	       );

	
        printf("Query_refine_checked_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_refine_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_checked_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_refine_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_refine_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_total_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       trie->stats->query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_rand_output_count,
	       queries,
	       query_num
	       );


        printf("Query_total_checked_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_total_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_checked_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_nodes_count\t%u\t%s\t%d\n",
	       trie->stats->query_total_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_ts_count\t%llu\t%s\t%d\n",
	       trie->stats->query_total_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_approx_distance\t%f\t%s\t%d\n",
	       trie->stats->query_approx_distance,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_filename\t%s\t%s\t%d\n",
	       trie->stats->query_approx_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_approx_node_size\t%u\t%s\t%d\n",
	       trie->stats->query_approx_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_level\t%u\t%s\t%d\n",
	       trie->stats->query_approx_node_level,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       trie->stats->query_exact_distance,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_filename\t%s\t%s\t%d\n",
	       trie->stats->query_exact_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_exact_node_size\t%u\t%s\t%d\n",
	       trie->stats->query_exact_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_level\t%u\t%s\t%d\n",
	       trie->stats->query_exact_node_level,
	       queries,
	       query_num
	       );
        printf("Query_lb_distance\t%f\t%s\t%d\n",
	       trie->stats->query_lb_distance,
	       queries,
	       query_num
	       );
	
        printf("Query_tlb\t%f\t%s\t%d\n",
	       trie->stats->query_tlb,	       
	       queries,
	       query_num
	       );	
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	       trie->stats->query_pruning_ratio,	       
	       queries,
	       query_num
	       );

        printf("Query_eff_epsilon\t%f\t%s\t%d\n",
	       trie->stats->query_eff_epsilon,	       
	       queries,
	       query_num
	       );		

}



void get_queries_stats(struct sfa_trie * trie, unsigned int queries_count)
{

  //total_non_zero_queries_count can be different from total_queries_count
  //since the former only includes queries with non zero distance so
  //we can calculate the effective epsilon
  
  if (queries_count != 0)
  {
    trie->stats->queries_avg_pruning_ratio =  ((double) trie->stats->queries_sum_pruning_ratio)
                                               / queries_count ;
    trie->stats->queries_sum_squares_pruning_ratio -= (pow(trie->stats->queries_sum_pruning_ratio,2)
							/queries_count);
    trie->stats->queries_sd_pruning_ratio =  sqrt(((double) trie->stats->queries_sum_squares_pruning_ratio)
						   / queries_count);
    trie->stats->queries_avg_checked_nodes_count /= queries_count; 
    trie->stats->queries_avg_checked_ts_count /= queries_count;
    trie->stats->queries_avg_loaded_nodes_count /= queries_count; 
    trie->stats->queries_avg_loaded_ts_count /= queries_count;     

    trie->stats->queries_avg_tlb =  ((double) trie->stats->queries_sum_tlb)
                                               / queries_count  ;
    trie->stats->queries_sum_squares_tlb -= (pow(trie->stats->queries_sum_tlb,2)
							/queries_count );
    trie->stats->queries_sd_tlb =  sqrt(((double) trie->stats->queries_sum_squares_tlb)
						   / queries_count);            
  }
  
  
  if ((queries_count - trie->stats->eff_epsilon_queries_count) != 0)
  {
    trie->stats->queries_avg_eff_epsilon =  ((double) trie->stats->queries_sum_eff_epsilon)
                                               / (queries_count - trie->stats->eff_epsilon_queries_count)  ;
    trie->stats->queries_sum_squares_eff_epsilon -= (pow(trie->stats->queries_sum_eff_epsilon,2)
							/(queries_count - trie->stats->eff_epsilon_queries_count) );
    trie->stats->queries_sd_eff_epsilon =  sqrt(((double) trie->stats->queries_sum_squares_eff_epsilon)
						   / (queries_count - trie->stats->eff_epsilon_queries_count));
  }
   
}


void print_queries_stats(struct sfa_trie * trie, int count_queries)
{
  /*
        printf("---------------------------------------- \n");    
        printf("QUERY SUMMARY STATISTICS \n");
        printf("TOTAL QUERIES LOADED %u \n", count_queries );
        printf("TOTAL QUERIES WITH NON-ZERO DISTANCE %u \n", trie->stats->total_queries_count );	
        printf("---------------------------------------- \n");
  */

        printf("Queries_filter_input_time_secs \t  %f \n", trie->stats->queries_filter_input_time/1000000); 
        printf("Queries_filter_output_time_secs \t  %f \n", trie->stats->queries_filter_output_time/1000000); 
        printf("Queries_filter_load_node_time_secs \t  %f \n", trie->stats->queries_filter_load_node_time/1000000);
        printf("Queries_filter_cpu_time_secs \t  %f \n", trie->stats->queries_filter_cpu_time/1000000);
        printf("Queries_filter_total_time_secs \t  %f \n", trie->stats->queries_filter_total_time/1000000);

        printf("Queries_filter_seq_input_count \t  %llu  \n", trie->stats->queries_filter_seq_input_count); 
        printf("Queries_filter_seq_output_count \t  %llu \n", trie->stats->queries_filter_seq_output_count);
        printf("Queries_filter_rand_input_count \t  %llu  \n", trie->stats->queries_filter_rand_input_count); 
        printf("Queries_filter_rand_onput_count \t  %llu \n", trie->stats->queries_filter_rand_output_count);
	
        printf("Queries_refine_input_time_secs \t  %f \n", trie->stats->queries_refine_input_time/1000000); 
        printf("Queries_refine_output_time_secs \t  %f \n", trie->stats->queries_refine_output_time/1000000); 
        printf("Queries_refine_load_node_time_secs \t  %f \n", trie->stats->queries_refine_load_node_time/1000000);
        printf("Queries_refine_cpu_time_secs \t  %f \n", trie->stats->queries_refine_cpu_time/1000000);
        printf("Queries_refine_total_time_secs \t  %f \n", trie->stats->queries_refine_total_time/1000000);

        printf("Queries_refine_seq_input_count \t  %llu  \n", trie->stats->queries_refine_seq_input_count); 
        printf("Queries_refine_seq_output_count \t  %llu \n", trie->stats->queries_refine_seq_output_count);
        printf("Queries_refine_rand_input_count \t  %llu  \n", trie->stats->queries_refine_rand_input_count); 
        printf("Queries_refine_rand_onput_count \t  %llu \n", trie->stats->queries_refine_rand_output_count);

        printf("Queries_total_input_time_secs \t  %f \n", trie->stats->queries_total_input_time/1000000); 
        printf("Queries_total_output_time_secs \t  %f \n", trie->stats->queries_total_output_time/1000000); 
        printf("Queries_total_Load_node_time_secs \t  %f \n", trie->stats->total_load_node_time/1000000);
        printf("Queries_total_cpu_time_secs \t  %f \n", trie->stats->queries_total_cpu_time/1000000);
        printf("Queries_total_Total_time_secs \t  %f \n", trie->stats->queries_total_time/1000000);	

        printf("Queries_total_seq_input_count \t  %llu  \n", trie->stats->queries_total_seq_input_count); 
        printf("Queries_total_seq_output_count \t  %llu \n", trie->stats->queries_total_seq_output_count);
        printf("Queries_total_rand_input_count \t  %llu  \n", trie->stats->queries_total_rand_input_count); 
        printf("Queries_total_rand_onput_count \t  %llu \n", trie->stats->queries_total_rand_output_count);

        printf("Queries_avg_checked_nodes_count \t  %f \n", trie->stats->queries_avg_checked_nodes_count);  
        printf("Queries_avg_checked_ts_count \t  %f \n", trie->stats->queries_avg_checked_ts_count);
        printf("Queries_avg_loaded_nodes_count \t  %f \n", trie->stats->queries_avg_loaded_nodes_count);  
        printf("Queries_avg_loaded_ts_count \t  %f \n", trie->stats->queries_avg_loaded_ts_count);
	
	printf("Queries_min_tlb \t  %f \n", trie->stats->queries_min_tlb);
	printf("Queries_max_tlb \t  %f \n", trie->stats->queries_max_tlb);
	printf("Queries_avg_tlb \t  %f \n", trie->stats->queries_avg_tlb);
	printf("Queries_sd_tlb \t  %f \n", trie->stats->queries_sd_tlb);
	
	printf("Queries_min_pruning_ratio \t  %f \n", trie->stats->queries_min_pruning_ratio);
	printf("Queries_max_pruning_ratio \t  %f \n", trie->stats->queries_max_pruning_ratio);
	printf("Queries_avg_pruning_ratio \t  %f \n", trie->stats->queries_avg_pruning_ratio);
	printf("Queries_sd_pruning_ratio \t  %f \n", trie->stats->queries_sd_pruning_ratio);

	printf("Queries_min_eff_epsilon \t  %f \n", trie->stats->queries_min_eff_epsilon);
	printf("Queries_max_eff_epsilon \t  %f \n", trie->stats->queries_max_eff_epsilon);
	printf("Queries_avg_eff_epsilon \t  %f \n", trie->stats->queries_avg_eff_epsilon);
	printf("Queries_sd_eff_epsilon \t  %f \n", trie->stats->queries_sd_eff_epsilon);
	
        printf("Total Combined trieing and querying times \t  %f \n",
	       (trie->stats->queries_total_time+trie->stats->idx_total_time)/1000000); 

}

void sfa_trie_destroy(struct sfa_trie *trie, struct sfa_node *node, boolean is_trie_new)
{
  
   if (node->level == 0) //root
   {
     if (trie->buffer_manager != NULL)
	 destroy_buffer_manager(trie);
     free(node->min_values);
     free(node->max_values);
     free(node->sfa_word);     
   }
   
   struct sfa_node * curr = node->children;

   if (curr != NULL)
     {
       for (int i = 0; i < trie->settings->num_symbols; ++i)
	 {
	   if(curr->is_leaf)
	     {
	       sfa_trie_destroy(trie, curr, is_trie_new);               
	     }
	   curr= curr->next;
	 }
       
       curr = node->children;
       struct sfa_node *prev;
       
       for (int i = 0; i < trie->settings->num_symbols; ++i)
	 {
	   prev = curr;
	   curr = curr->next;
	   
	   if(prev->filename != NULL)
	     {
	       free(prev->filename);
	     }
	   
	   if(prev->min_values != NULL)
	     {
	       free(prev->min_values);
	     }
	   if(prev->max_values != NULL)
	     {
	       free(prev->max_values);
	     }   
	   
	   if(prev->sfa_word != NULL)
	     {
	       free(prev->sfa_word);
	     }	   
	   
	   if(prev->file_buffer != NULL)
	     {
	       free(prev->file_buffer->buffered_list);
	       prev->file_buffer->buffered_list = NULL;
	       prev->file_buffer->buffered_list_size = 0;
	       free(prev->file_buffer);
	     }
	   free(prev);
	 }
     }
}


void destroy_buffer_manager(struct sfa_trie *trie)
{

  if(trie->buffer_manager != NULL)
  {
    struct sfa_file_map *currP;
    struct sfa_file_map *temp;

    temp = NULL;
    currP = trie->buffer_manager->file_map; 

    while(currP != NULL)
    {
      temp = currP;
      currP = currP->next;      
      free(temp);
    }

    
    for (unsigned long i= 0; i < trie->settings->fft_size; ++i)
    {
      free(trie->bins[i]);
    }
  
    if (trie->bins != NULL)   free(trie->bins);

    free (trie->buffer_manager->sfa_mem_array);
    free (trie->buffer_manager->ts_mem_array);
    free (trie->buffer_manager->dft_mem_array);
    free(trie->buffer_manager);    

  }
 
}

void sfa_trie_insert(struct sfa_trie * trie,
		     int index,		     
		     struct sfa_record * record
		     )
{

  //traverse the index tree to find the appropriate node
  struct sfa_node * node = trie->first_node;

  //choose the correct subtree
  int word_length = trie->settings->fft_size;
  unsigned char key;
  struct sfa_node *child_node;
  unsigned int skip = 0 ;
  unsigned int skip_idx =0;

  if (node->children == NULL)
  {
      //add children
      sfa_node_create_children(trie,node);
  }
  while (!node->is_leaf)
  {
    sfa_node_update_statistics(trie,node,record->dft_transform);
      
    key = record->sfa_word[index];
    //skip = key - 'A';
    skip = (unsigned int)key;    
    
    child_node = node->children;
    skip_idx = 0;
    while (skip_idx < skip)
      {
	child_node = child_node->next;  //&node->children[key-'A'];
	++skip_idx;
      }    
    node = child_node;
    ++index;
  }
        
  if (node->is_leaf)
  {
    //sfa_node_update_statistics(trie,node,record->dft_transform);
    sfa_node_append_record(trie,node, record);
	  
    if (node->node_size >= trie->settings->max_leaf_size)
      {
	// last element was inserted
	if (index >= word_length - 1)
	  {
	    printf("Maximum Leaf Size Reached. Cannot split further\n");
	    printf("In original JAVA version, record appended regardless\n");	       
	    exit(1);
	    /*
	      sfa_node_update_statistics(trie,node,record->dft_transform);     
	      sfa_node_append_record(trie,node, record);        
	    */
	  }
	//split the node and find which child to assign to

	sfa_node_create_children(trie,node);

	if (!get_file_buffer(trie, node))
	  { 
	    fprintf(stderr,"Error in dstree_index.c: could not get the file \
                           buffer for node %s.\n", node->filename);
	    return FAILURE;              
	  }
	
	struct sfa_record * parent_records;
	parent_records = get_all_records_in_node(trie, node);
	++index;
	for (int idx=0; idx <trie->settings->max_leaf_size;++idx)
	  {
	    child_node = node->children;
	    key = parent_records[idx].sfa_word[index];
	    //skip = key - 'A';
	    skip = (unsigned int) key;
	    skip_idx = 0;	       
	    while (skip_idx < skip)
	      {
		child_node = child_node->next;  //&node->children[key-'A'];
		++skip_idx;
	      }
	    //update the stats of the node being split not the children..WHY
	    //sfa_node_update_statistics(trie,node,parent_records[idx].dft_transform);
	    
	    sfa_node_append_record(trie,child_node, &parent_records[idx]);
	  }

	
	for (int i = 0 ; i < trie->settings->max_leaf_size; ++i)
	  {
	    free(parent_records[i].timeseries);
	    free(parent_records[i].dft_transform);
	    free(parent_records[i].sfa_word);		  
	  }
	
	free(parent_records);
	       
	delete_file_buffer(trie,node);
	       

      }
         // split leaf
  } //end if_node_is_leaf

  return SUCCESS;
}

enum response sfa_trie_write(struct sfa_trie *trie)
{
	fprintf(stderr, ">>> Storing trie: %s\n", trie->settings->root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(trie->settings->root_directory) + 9));
	filename = strcpy(filename, trie->settings->root_directory);
	filename = strcat(filename, "root.idx\0");

        COUNT_PARTIAL_RAND_OUTPUT
        COUNT_PARTIAL_OUTPUT_TIME_START	  
	FILE *file = fopen(filename, "wb");
        COUNT_PARTIAL_OUTPUT_TIME_END	  

	free(filename);

        if(file == NULL)
        {   
           fprintf(stderr, "Error in sfa_trie.c: Could not open"
		" the trie file. Reason = %s\n", strerror(errno));
           return FAILURE;
        }	
	
	unsigned int timeseries_size = trie->settings->timeseries_size;
	unsigned int max_leaf_size = trie->settings->max_leaf_size;
	unsigned int fft_size = trie->settings->fft_size;
	unsigned int num_symbols = trie->settings->num_symbols;
	double buffered_memory_size = trie->settings->buffered_memory_size;
	boolean is_norm = trie->settings->is_norm;
	ts_type norm_factor = trie->settings->norm_factor;
	unsigned int start_offset = trie->settings->start_offset;
	//boolean is_trie_new = trie->settings->is_trie_new;
	unsigned int histogram_type = trie->settings->histogram_type;
	unsigned int lb_dist = trie->settings->lb_dist;	
	unsigned int sample_size = trie->settings->sample_size;
	
	// SETTINGS DATA
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT	  
        COUNT_PARTIAL_OUTPUT_TIME_START
        fwrite(&leaf_nodes_count, sizeof(unsigned long), 1, file);
	fwrite(&buffered_memory_size, sizeof(double), 1, file);
	fwrite(&timeseries_size, sizeof(unsigned int), 1, file);
	fwrite(&max_leaf_size, sizeof(unsigned int), 1, file);	
	fwrite(&fft_size, sizeof(unsigned int), 1, file);
	fwrite(&num_symbols, sizeof(unsigned int), 1, file);
	fwrite(&is_norm, sizeof(boolean), 1, file);
	fwrite(&norm_factor, sizeof(unsigned int), 1, file);
	fwrite(&start_offset, sizeof(unsigned int), 1, file);			
	fwrite(&histogram_type, sizeof(unsigned int), 1, file);
	fwrite(&lb_dist, sizeof(unsigned int), 1, file);	
	fwrite(&sample_size, sizeof(unsigned int), 1, file);
	//fwrite(&is_trie_new, sizeof(boolean), 1, file);
        COUNT_PARTIAL_OUTPUT_TIME_END


	//THE BINS TO SET BREAKPOINTS
	//NEEDED FOR QUERIES

	for (int i = 0; i < trie->settings->fft_size; ++i)
	{
	  for (int j=0; j < trie->settings->num_symbols-1; ++j)
	    {
	      	fwrite(&trie->bins[i][j], sizeof(ts_type), 1, file);
	    }
	}	  
	  
	// NODES AND FILE BUFFERS
	sfa_node_write(trie, trie->first_node, file);
        COUNT_PARTIAL_OUTPUT_TIME_START
        fseek(file, 0L, SEEK_SET);
        fwrite(&leaf_nodes_count, sizeof(unsigned long), 1, file);	  	  	  
	fclose(file);
        COUNT_PARTIAL_OUTPUT_TIME_END
	  
	return SUCCESS;
}

struct sfa_trie * sfa_trie_read(const char* root_directory)
{
        if(chdir(root_directory) != 0)
        {
            fprintf(stderr, "The trie directory does not exist. "
                             "Please provide a valid directory.\n");
            exit (-1);
        }

	fprintf(stderr, ">>> Loading trie: %s\n", root_directory);

	const char *filename = malloc(sizeof(char) * (strlen(root_directory) + 9));
	filename = strcpy(filename, root_directory);
	filename = strcat(filename, "root.idx");
	
	//printf ("Trie file: %s\n",filename);
        COUNT_PARTIAL_RAND_INPUT
        COUNT_PARTIAL_INPUT_TIME_START
	FILE *file = fopen(filename, "rb");
        COUNT_PARTIAL_INPUT_TIME_END

	free(filename);
	
        if(file == NULL)
        {   
           fprintf(stderr, "Error in sfa_trie.c: Could not open"
		" the trie file. Reason = %s\n", strerror(errno));
           return FAILURE;
        }

	unsigned long count_leaves = 0;	
	unsigned int timeseries_size = 0;
	unsigned int max_leaf_size = 0;
	unsigned int fft_size = 0;
	unsigned num_symbols = 0;
        double buffered_memory_size = 0;
	boolean is_trie_new = 0;
        boolean is_norm = true;
	unsigned int norm_factor = 0;
	unsigned int start_offset = 0;	
	unsigned int histogram_type = 0;
	unsigned int lb_dist = 0;	
	unsigned int sample_size = 0;
	
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT	  
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT	  
        COUNT_PARTIAL_SEQ_INPUT	  
        COUNT_PARTIAL_INPUT_TIME_START
        fread(&count_leaves, sizeof(unsigned long), 1, file);
	fread(&buffered_memory_size, sizeof(double), 1, file);
	fread(&timeseries_size, sizeof(unsigned int), 1, file);
	fread(&max_leaf_size, sizeof(unsigned int), 1, file);	
	fread(&fft_size, sizeof(unsigned int), 1, file);
	fread(&num_symbols, sizeof(unsigned int), 1, file);
	fread(&is_norm, sizeof(boolean), 1, file);
	fread(&norm_factor, sizeof(unsigned int), 1, file);
	fread(&start_offset, sizeof(unsigned int), 1, file);			
	fread(&histogram_type, sizeof(unsigned int), 1, file);
	fread(&lb_dist, sizeof(unsigned int), 1, file);	
	fread(&sample_size, sizeof(unsigned int), 1, file);
	//fread(&is_trie_new, sizeof(boolean), 1, file);
	
        COUNT_PARTIAL_INPUT_TIME_END

	  struct sfa_trie_settings * trie_settings = sfa_trie_settings_init(root_directory,					 	     								        timeseries_size, 
									  fft_size, 
									  num_symbols,
									  max_leaf_size,
									  buffered_memory_size,
									  is_norm,
									  histogram_type,
									  lb_dist,						    	 								       sample_size,
									  is_trie_new);

    
        struct sfa_trie * trie = sfa_trie_init(trie_settings);

	trie->stats->leaves_heights = calloc(count_leaves, sizeof(int));
	trie->stats->leaves_sizes = calloc(count_leaves, sizeof(int));
	trie->stats->leaves_counter = 0;

	for (int i = 0; i < trie->settings->fft_size; ++i)
	{
	  for (int j=0; j < trie->settings->num_symbols-1; ++j)
	    {
	      	fread(&trie->bins[i][j], sizeof(ts_type), 1, file);
	    }
	}	  

        trie->first_node  = sfa_node_read(trie, file);

        COUNT_PARTIAL_INPUT_TIME_START	
	fclose(file);
        COUNT_PARTIAL_INPUT_TIME_END
	  
	return trie;
}

enum response sfa_update_trie_stats(struct sfa_trie *trie, struct sfa_node *node)
{

  unsigned int height = node->level +1;
  unsigned int threshold = trie->settings->max_leaf_size;
  //at this point all time series are on disk
  unsigned int node_size = node->node_size;

  double node_fill_factor = (node_size * 100.0)/threshold;  
    
  if (node_fill_factor < trie->stats->min_fill_factor)
  {
    trie->stats->min_fill_factor = node_fill_factor;
  }
  if (node_fill_factor > trie->stats->max_fill_factor)
  {
    trie->stats->max_fill_factor = node_fill_factor;
  }
  if (height < trie->stats->min_height)
  {
    trie->stats->min_height = height;    
  }
  if (height > trie->stats->max_height)
  {
    trie->stats->max_height = height;
  }

  trie->stats->sum_fill_factor += node_fill_factor ;
  trie->stats->sum_squares_fill_factor += pow(node_fill_factor,2) ;

  trie->stats->sum_height += height;
  trie->stats->sum_squares_height += pow(height,2) ;
     
  COUNT_TOTAL_TS(node_size)
  
}

  
			

enum response sfa_node_write(struct sfa_trie *trie, struct sfa_node *node, FILE *file)
{
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT	
        COUNT_PARTIAL_SEQ_OUTPUT  
	COUNT_PARTIAL_OUTPUT_TIME_START	  
        fwrite(&(node->is_leaf), sizeof(unsigned char), 1, file);
	fwrite(&(node->node_size), sizeof(unsigned int), 1, file);
        fwrite(&(node->level), sizeof(unsigned int), 1, file);
	fwrite(&(node->prefix_length), sizeof(unsigned int), 1, file);
	//fwrite(node->parent, sizeof(struct sfa_node),1, file);

	for (int i =0; i< node->prefix_length; ++i)
	{
  	      COUNT_PARTIAL_SEQ_OUTPUT
	      fwrite(&(node->sfa_word[i]),
		     sizeof(unsigned char),
		     1,
		     file);
	}	    
		

	if (node->node_size !=0)
	{
	    for (int i =0; i< trie->settings->fft_size; ++i)
	      {
		  COUNT_PARTIAL_SEQ_OUTPUT
		  COUNT_PARTIAL_SEQ_OUTPUT		      
		  fwrite(&(node->min_values[i]),
			 sizeof(ts_type),
			 1,
			 file);
		  fwrite(&(node->max_values[i]),
			 sizeof(ts_type),
			 1,
			 file);			    
	      }	    
	}

        COUNT_PARTIAL_OUTPUT_TIME_END

	if(node->is_leaf) {
		if(node->filename != NULL) {
			int filename_size = strlen(node->filename);
                        COUNT_PARTIAL_SEQ_OUTPUT	
                        COUNT_PARTIAL_SEQ_OUTPUT  
                  	COUNT_PARTIAL_OUTPUT_TIME_START
               	        fwrite(&filename_size, sizeof(int), 1, file);
			fwrite(node->filename, sizeof(char), filename_size, file);

                        COUNT_PARTIAL_SEQ_OUTPUT
                        COUNT_PARTIAL_SEQ_OUTPUT
			 COUNT_PARTIAL_OUTPUT_TIME_END
			
                        COUNT_PARTIAL_OUTPUT_TIME_END
			  
			  /* IF THIS CODE IS EXTENDED TO INCLUDE UPDATES OR DELETES, CALL 
                             flush_buffer_to_disk WITH TRUE*/
			  
			if (node->file_buffer != NULL)
 			    flush_buffer_to_disk(trie,node,false);
                        COUNT_LEAF_NODE
                        //collect stats while traversing the trie
			sfa_update_trie_stats(trie, node);
		}
		else {
			int filename_size = 0;
                        COUNT_PARTIAL_SEQ_OUTPUT  
                  	COUNT_PARTIAL_OUTPUT_TIME_START
			fwrite(&filename_size, sizeof(int), 1, file);
                        COUNT_PARTIAL_OUTPUT_TIME_END						
		}
	}
	else {
	  COUNT_PARTIAL_SEQ_OUTPUT
	    COUNT_PARTIAL_SEQ_OUTPUT
	    COUNT_PARTIAL_SEQ_OUTPUT
	    COUNT_PARTIAL_OUTPUT_TIME_START	  
	    COUNT_PARTIAL_OUTPUT_TIME_END
	      
	    struct sfa_node * cur = node->children;
	    	    
	    while (cur != NULL)
	      {
		sfa_node_write(trie, cur,file);
		cur = cur->next;
	      }

	}
      
	
	return SUCCESS;
}

struct sfa_node * sfa_node_read(struct sfa_trie *trie, FILE *file) {

        struct sfa_node *node = NULL;
        //to initialize node values for leaf and internal nodes
	node = sfa_leaf_node_init(trie->settings);
	
	unsigned char is_leaf = 0;
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT		     
        COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START	
	fread(&is_leaf, sizeof(unsigned char), 1, file);
	fread(&(node->node_size), sizeof(unsigned int), 1, file);
        fread(&(node->level), sizeof(unsigned int), 1, file);
        fread(&(node->prefix_length), sizeof(unsigned int), 1, file);
        //fread(node->parent, sizeof(struct sfa_node), 1, file);

	for (int i =0; i< node->prefix_length; ++i)
	{
  	      COUNT_PARTIAL_SEQ_OUTPUT
	      fread(&(node->sfa_word[i]),
		     sizeof(unsigned char),
		     1,
		     file);
	}	    
		
	
        if (node->node_size != 0 )
	  {
	    for (int i =0; i< trie->settings->fft_size; ++i)
	      {
		COUNT_PARTIAL_SEQ_OUTPUT
		  COUNT_PARTIAL_SEQ_OUTPUT		      
		  fread(&(node->min_values[i]),
			sizeof(ts_type),
			1,
			file);
		fread(&(node->max_values[i]),
		      sizeof(ts_type),
		      1,
		      file);
	      }

	  }
	else
	  {
	    COUNT_EMPTY_LEAF_NODE	                
	  }
	
        COUNT_PARTIAL_INPUT_TIME_END		
 	if(is_leaf) {
		node->is_leaf = 1;
		sfa_file_buffer_init(node);

		int filename_size = 0;

                COUNT_PARTIAL_SEQ_INPUT
	        COUNT_PARTIAL_INPUT_TIME_START			
		fread(&filename_size, sizeof(int), 1, file);
                COUNT_PARTIAL_INPUT_TIME_END				

		node->file_buffer->disk_count = node->node_size;

		if(filename_size > 0)
		{
			node->filename = malloc(sizeof(char) * (filename_size + 1));
			
                        COUNT_PARTIAL_SEQ_INPUT
	                COUNT_PARTIAL_INPUT_TIME_START
			fread(node->filename, sizeof(char), filename_size, file);
                        COUNT_PARTIAL_INPUT_TIME_END							

			node->filename[filename_size] = '\0';

                        COUNT_PARTIAL_SEQ_INPUT
                        COUNT_PARTIAL_SEQ_INPUT
	                COUNT_PARTIAL_INPUT_TIME_START			  


	                COUNT_PARTIAL_INPUT_TIME_END

			if (node->node_size != 0)
			{
			    COUNT_LEAF_NODE
			      
			    trie->stats->leaves_heights[trie->stats->leaves_counter] = node->level + 1;
			    trie->stats->leaves_sizes[trie->stats->leaves_counter] = node->node_size;
			    ++(trie->stats->leaves_counter);
			    sfa_update_trie_stats(trie, node);			  
			}
                 }
		else
		{
			node->filename = NULL;
			node->node_size = 0;
		}
		//get all timeseries for this node into its this file buffer

		//get_all_time_series_in_node(trie,node);
		//node->file_buffer->disk_count = 0;
	}
	else {
	  node->file_buffer = NULL;
	  node->is_leaf = 0;
	  node->filename = NULL;
	  	  
	  node->children = sfa_node_read(trie, file);
	  struct sfa_node * cur = node->children;
	  unsigned long num_children = trie->settings->num_symbols;
	  
	  for (unsigned long i = 0; i < num_children -1 ; ++i )
	    {
	      cur->next =sfa_node_read(trie, file);
	      cur = cur->next;
	    }	  
	    	     
	}
	  
	return node;
}


void sfa_divide_equi_width_hist(struct sfa_trie *trie)
{

  ts_type * cur_coeff_line;
  unsigned int sample_size = trie->settings->sample_size;

  fprintf(stderr, "Using Equi-width histograms\n");
  
  for (int i = 0; i < trie->settings->fft_size; ++i)
  {
       cur_coeff_line = (ts_type *) trie->buffer_manager->current_dft_record;
       //trie->buffer_manager->current_record_index++; no need because we will read from beg after
	   
      ts_type first = cur_coeff_line[0];
      ts_type last = cur_coeff_line[sample_size-1];
      
      ts_type interval_width = (last-first) / (ts_type)(trie->settings->num_symbols);
      for (int j=0; j < trie->settings->num_symbols-1; ++j)
      {
	trie->bins[i][j] = interval_width*(j+1)+first;
      }
      trie->buffer_manager->current_dft_record += sizeof(ts_type) * sample_size;
  }
    //reset current record to point to the start of the array
    trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;  
}

//enum response sfa_divide_equi_depth_hist(struct sfa_trie *trie)
void sfa_divide_equi_depth_hist(struct sfa_trie *trie)
{

  ts_type * cur_coeff_line;
  unsigned int sample_size = trie->settings->sample_size;

  ts_type depth = (ts_type) trie->settings->sample_size /trie->settings->num_symbols;
  fprintf(stderr, "Using Equi-depth histograms\n");
  
  for (int i = 0; i < trie->settings->fft_size; ++i)
  {
      cur_coeff_line = (ts_type *) trie->buffer_manager->current_dft_record;  

	   int pos = 0;
	   int count = 0;
       for (int j=0; j < trie->settings->sample_size; ++j)
	 {
	   ++count;
	   unsigned int ceiling = ceil (depth * (pos+1));
	   if (count > ceiling &&
	       (pos == 0 || trie->bins[i][pos-1] != cur_coeff_line[j]))
	   {
	     trie->bins[i][pos] =  cur_coeff_line[j];
	     pos++;
 	   }


	 }
       trie->buffer_manager->current_dft_record += sizeof(ts_type) * sample_size;
  }
    //reset current record to point to the start of the array
  trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;  
}

void sfa_print_bins(struct sfa_trie *trie)
{
  fprintf(stderr,"[\n");
  for (int i = 0; i < trie->settings->fft_size; ++i)
  {
      fprintf(stderr,"-Inf\t");
      for (int j=0; j < trie->settings->num_symbols-1; ++j)
       {
         ts_type value = roundf(trie->bins[i][j]*100.0)/100.0;
         if (value == FLT_MAX)	  
	   fprintf(stderr,", Inf\t\n");
	 else
	   fprintf(stderr,",\t%g\t",value);
       }
       fprintf(stderr,";\n");
  }
  fprintf(stderr,"]\n");
  
}


void sfa_trie_set_bins(struct sfa_trie *trie, char * ifilename)
{

   unsigned long ts_loaded;
   
    //first build the bins out of a sample of the data
      //read whole sample in memory
   ts_loaded = 0;
   FILE * ifile;
   ifile = fopen (ifilename,"rb");
    unsigned long ts_length = trie->settings->timeseries_size;
    unsigned int sample_size = trie->settings->sample_size;
      int transforms_size = trie->settings->fft_size;
      
    ts_type * ts= NULL;
    ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
    
    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *)fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;
 
    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * transform = NULL;
    transform = fftwf_malloc ( sizeof ( ts_type ) * ts_length);
    
   while (ts_loaded < sample_size) 
   {
           ts_type * ts_orig  = (ts_type *) trie->buffer_manager->current_ts_record;
           fread(ts_orig, sizeof(ts_type), ts_length, ifile);

	   for (int i =0; i< ts_length; ++i)
	     ts[i] =ts_orig[i];
           trie->buffer_manager->current_ts_record += sizeof(ts_type) * ts_length;
	   

            fft_from_ts(trie, ts, ts_out, transform, plan_forward);

  	     ts_type *cur_coeff_line;

    	     trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
             for (int j = 0; j < transforms_size; ++j)	       
	     {
		 cur_coeff_line = (ts_type *) trie->buffer_manager->current_dft_record;
		 ts_type value =(ts_type) roundf(transform[j]*100.0)/100.0;      
		 //we will use the dft memory to temporarily store
		 //the transforms, which will be discarded
		 //once the bins are set. The dft memory array will
		 //then store the transforms for the overlapping
		 //subsequences.
		 //trie->buffer_manager->dft_mem_array[j-offset][i] = value;
		 cur_coeff_line[ts_loaded] = value;      
		 //fprintf(stderr,"order_line[%d][%d] = %g \n", j,i,trie->order_line[j][i]);
		 trie->buffer_manager->current_dft_record += sizeof(ts_type) * sample_size;     
	     }
	     //reset current record to point to the start of the array
	     trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;

	   ++ts_loaded;
   }

   fftwf_destroy_plan (plan_forward);
   fftwf_free (ts);
   fftwf_free (ts_out);
   fftwf_free (transform);
   //free(transform);

   //reset to start from beginning of ts memory buffer
   trie->buffer_manager->current_ts_record = trie->buffer_manager->ts_mem_array;
   trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;

      
   // Train Quantization bins on the sample
      
   //sfa_trie_set_bins(trie);
  
   sfa_trie_fill_order_line(trie);
  
   if (trie->settings->histogram_type == equi_width)
   {
     sfa_divide_equi_width_hist(trie);
   }
   else
   {
     sfa_divide_equi_depth_hist(trie);     
   }

   fclose(ifile);

   sfa_print_bins(trie);
   reset_dft_memory_size(trie);

   trie->buffer_manager->current_ts_record = trie->buffer_manager->ts_mem_array;
   trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
   trie->buffer_manager->current_sfa_record = trie->buffer_manager->sfa_mem_array;
   trie->buffer_manager->current_record_index = 0;


   //return SUCCESS; 
}

void sfa_trie_fill_order_line(struct sfa_trie *trie)
{
  double mean;
  double stdev;
 unsigned int sample_size = trie->settings->sample_size;
  struct sfa_trie_settings * settings = trie->settings; 

  unsigned long ts_length = trie->settings->timeseries_size;
    
  //int word_length = trie->settings->word_length;
  int transforms_size = trie->settings->fft_size;

  //boolean is_lower_bounding = trie->settings->is_lower_bounding;
  //boolean is_norm_mean = trie->settings->is_norm_mean;    

  double norm = 1;
  int offset = 0;    

  fprintf(stderr,"Sample size %lu\n", sample_size);     
    
  //fft_from_ts_chunk(trie);

  ts_type * cur_coeff_line;

  for (int j = 0; j < transforms_size; ++j)
  {
    cur_coeff_line = (ts_type *) trie->buffer_manager->current_dft_record;

    qsort(cur_coeff_line,
	  sample_size,
	  sizeof(ts_type),
	  compare_ts_type);

    trie->buffer_manager->current_dft_record += sizeof(ts_type) * sample_size;
  }
  
  trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;  
 
}

int sfa_trie_get_best_depth(unsigned long count, unsigned long chunk_size) {
    int trie_depth = (int) (roundf(logf(count / chunk_size) / logf(8)));
    fprintf(stderr,"Using trie depth:\t %d (%d buckets)\n", trie_depth, (int) pow(8,trie_depth));
    return trie_depth;
}
