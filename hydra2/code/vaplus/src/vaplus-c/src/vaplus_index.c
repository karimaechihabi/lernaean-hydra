//  
//  vaplus_index.c
//  vaplus  C version
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
#include "../include/vaplus_index.h"
#include "../include/vaplus_node.h"
#include "../include/calc_utils.h"
//#include "../include/vaplus_node_split.h"
#include "../include/vaplus_file_buffer_manager.h"
#include "../include/vaplus_file_buffer.h"

/**
 This function initializes the settings of a vaplus index
 */
struct vaplus_index_settings * vaplus_index_settings_init(const char * root_directory,
						 unsigned int timeseries_size, 
                                                 unsigned int word_length, 
                                                 unsigned int num_bits,
						 double buffered_memory_size,
						 boolean is_norm,
						 unsigned int dataset_size,
						 boolean is_index_new)
{

  if (is_index_new)
  {
    if(chdir(root_directory) == 0)
    {
        fprintf(stderr, "WARNING! Target index directory already exists. Please delete or choose a new one.\n");
        exit(-1);
    }
  }
    mkdir(root_directory, 0777);
  
    struct vaplus_index_settings *settings = malloc(sizeof(struct vaplus_index_settings));
    if(settings == NULL) {
        fprintf(stderr,"Error in vaplus_index.c: could not allocate memory for index settings.\n");
        return NULL;
    }

    settings->ts_byte_size = (sizeof(ts_type) * timeseries_size);
    settings->raw_filename = NULL;


    settings->root_directory = root_directory;
    settings->timeseries_size = timeseries_size;
    settings->buffered_memory_size = buffered_memory_size;

    //settings->word_length = word_length;
    settings->num_bits = num_bits;
    settings->fft_size =  fminf(timeseries_size, word_length);
    
    settings->norm_factor = 1/sqrtf(timeseries_size);
    
    settings->dataset_size = dataset_size;
    
    settings->is_norm = is_norm;
    settings->is_new = is_index_new;
    
    if (is_norm)
      settings->start_offset = 2;
    else
      settings->start_offset = 0;

    
     float child_index_len = 3; //ceil(log10())+1;
     //Modify this to 2^bits
     float prefix_len  = ceil(log10(num_bits))+1;

     settings->max_filename_size = settings->fft_size * (child_index_len+1) +  //index and dot
                                   4 +  //extension
                                   1; //null character
     
    return settings;
}



/**
 This function initializes an isax index
 @param isax_index_settings *settings
 @return isax_index
 */
struct vaplus_index * vaplus_index_init(struct vaplus_index_settings *settings)
{
    struct vaplus_index *index = malloc(sizeof(struct vaplus_index));
    if(index == NULL) {
        fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for index structure.\n");
        return NULL;
    }

    
    index->settings = settings;
    index->total_records = 0;
    index->cells = NULL;  
    index->cells = calloc(index->settings->fft_size,sizeof(unsigned int));
  
    if (!init_file_buffer_manager(index))
    { 
      fprintf(stderr, "Error in vaplus_index.c:  Could not initialize the \
                       file buffer manager for this index.\n");
      return NULL;              
    }
    
    return index;
}



/*
  This functions allocates a two dimensional array
  of num_words rows and (num_symbols-1) columns
  The array will contain the discretization
  intervals
*/


enum response vaplus_index_bins_init(struct vaplus_index * index)
{

  //First identify number of bits per dimension for non-uniform bit allocation

  vaplus_index_bins_calculate_non_uniform(index);  
  
  index->bins = NULL;  
  index->bins = calloc(index->settings->fft_size,sizeof(ts_type*));
  if(index->bins == NULL) {
        fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for index structure.\n");
        return FAILURE;
  }
  //allocate num_symbols-1 memory slots for each word
  for (int i = 0; i < index->settings->fft_size; ++i)
  {
    index->bins[i] = NULL;
    //index->bins[i] = calloc(index->settings->num_symbols-1,sizeof(ts_type));

    //each dimension has a specific number of cells
    if (index->cells[i] > 1 )
    {
       index->bins[i] = calloc(index->cells[i]+1,sizeof(ts_type));
    
       if(index->bins[i] == NULL) {
          fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for index structure.\n");
          return FAILURE;
       }
       for (int j = 0; j < index->cells[i]+1; ++j)
       {
	   index->bins[i][j] = 0;
       }
       fprintf(stderr,"Initialized bins[%d][%d] \n", index->settings->fft_size, index->cells[i]+1);             
    }


  }


  return SUCCESS;

}

/*
  This functions allocates the total budget of bits per dimension
  in a non-uniform fashion according to the quantization theory
  heuristic derived from the paper: Vector Quantization and Signal 
  Compression by Gersho.

  The number of bits allocated to each dimension depends on the 
  variance of DFT values. More bits are allocated for dimensions with
  higher variance.

*/

enum response vaplus_index_bins_calculate_non_uniform(struct vaplus_index * index)
{

  int transforms_size = index->settings->fft_size;
  unsigned int dataset_size = index->settings->dataset_size;
  
  ts_type * energies = calloc(transforms_size, sizeof(ts_type));
  int bit_budget = transforms_size * index->settings->num_bits;
  int cur_dimension = 0;
  ts_type * cur_coeff_line;


  //Calculate the energy for each dimension
  
  index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
  for (int j = 0; j < transforms_size; ++j)	       
  {
      cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
      //fprintf(stderr,"order_line[%d][%d] = %g \n", j,i,index->order_line[j][i]);
      energies[j] = calc_variance(cur_coeff_line,0,dataset_size);
      index->cells[j] = 1;
	
      index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }

  while(bit_budget > 0 )
  {
    //get the dimension with the maximum energy
    
     cur_dimension = get_index_max_value(energies,transforms_size);

     //use heuristic to double the number of cells for
     //current dimension and decrease its energy by
     //a factor of 4
     
     index->cells[cur_dimension] *= 2;
     energies[cur_dimension] /= 4;

     if (index->cells[cur_dimension] > index->settings->dataset_size || index->cells[cur_dimension] > UINT_MAX/2)
     {
            index->cells[cur_dimension] /= 2;
            printf("exceed bit allocation\n");

     }
     else
     {
       --bit_budget;
     }
     
  }

   //SANITY CHECK
    unsigned int total_cells_used = 0;

    for (int j = 0; j < transforms_size; ++j)	       
    {
      total_cells_used += index->cells[j];	
   }

   printf("total cells used is %u\n", total_cells_used);

   index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
   free(energies);
}

/*
enum response vaplus_init_stats(struct vaplus_index * index)
{
    index->stats = malloc(sizeof(struct stats_info));
    if(index->stats == NULL) {
        fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    index->stats->idx_building_input_time = 0;  
    index->stats->idx_building_output_time = 0;
    index->stats->idx_building_cpu_time = 0;
    index->stats->idx_building_total_time = 0;

    index->stats->idx_building_seq_input_count = 0;
    index->stats->idx_building_seq_output_count = 0;
    index->stats->idx_building_rand_input_count = 0;
    index->stats->idx_building_rand_output_count = 0;    

    index->stats->idx_writing_input_time = 0;  
    index->stats->idx_writing_output_time = 0;
    index->stats->idx_writing_cpu_time = 0;
    index->stats->idx_writing_total_time = 0;

    index->stats->idx_writing_seq_input_count = 0;
    index->stats->idx_writing_seq_output_count = 0;
    index->stats->idx_writing_rand_input_count = 0;
    index->stats->idx_writing_rand_output_count = 0;

    index->stats->idx_reading_input_time = 0;  
    index->stats->idx_reading_output_time = 0;
    index->stats->idx_reading_cpu_time = 0;
    index->stats->idx_reading_total_time = 0;
  
    index->stats->idx_reading_seq_input_count = 0;
    index->stats->idx_reading_seq_output_count = 0;
    index->stats->idx_reading_rand_input_count = 0;
    index->stats->idx_reading_rand_output_count = 0;
    
    index->stats->idx_total_input_time = 0;
    index->stats->idx_total_output_time = 0;
    index->stats->idx_total_cpu_time = 0;
    index->stats->idx_total_time = 0;

    index->stats->idx_total_seq_input_count = 0;
    index->stats->idx_total_seq_output_count = 0;
    index->stats->idx_total_rand_input_count = 0;
    index->stats->idx_total_rand_output_count = 0;    

    index->stats->total_nodes_count = 0;
    index->stats->leaf_nodes_count = 0;
    index->stats->empty_leaf_nodes_count = 0;
  
    index->stats->idx_size_bytes = 0;
    index->stats->idx_size_blocks = 0;
    
    index->stats->min_fill_factor = FLT_MAX;
    index->stats->max_fill_factor = 0;
    index->stats->sum_fill_factor = 0;
    index->stats->sum_squares_fill_factor = 0;
    index->stats->avg_fill_factor = 0;
    index->stats->sd_fill_factor = 0;

    index->stats->min_height = FLT_MAX;
    index->stats->max_height = 0;
    index->stats->sum_height = 0;
    index->stats->sum_squares_height = 0;    
    index->stats->avg_height = 0;
    index->stats->sd_height = 0;

    
    index->stats->query_filter_input_time = 0;
    index->stats->query_filter_output_time = 0;
    index->stats->query_filter_load_node_time = 0;
    index->stats->query_filter_cpu_time = 0;    
    index->stats->query_filter_total_time = 0;    

    index->stats->query_filter_seq_input_count = 0;
    index->stats->query_filter_seq_output_count = 0;
    index->stats->query_filter_rand_input_count = 0;
    index->stats->query_filter_rand_output_count = 0;    

    index->stats->query_filter_loaded_nodes_count = 0;
    index->stats->query_filter_checked_nodes_count = 0;    
    index->stats->query_filter_loaded_ts_count = 0;
    index->stats->query_filter_checked_ts_count = 0;

    index->stats->query_refine_input_time = 0;
    index->stats->query_refine_output_time = 0;
    index->stats->query_refine_load_node_time = 0;
    index->stats->query_refine_cpu_time = 0;
    index->stats->query_refine_total_time = 0;    

    index->stats->query_refine_seq_input_count = 0;
    index->stats->query_refine_seq_output_count = 0;
    index->stats->query_refine_rand_input_count = 0;
    index->stats->query_refine_rand_output_count = 0;    

    index->stats->query_refine_loaded_nodes_count = 0;
    index->stats->query_refine_checked_nodes_count = 0;    
    index->stats->query_refine_loaded_ts_count = 0;
    index->stats->query_refine_checked_ts_count = 0;

    index->stats->query_total_input_time = 0;
    index->stats->query_total_output_time = 0;
    index->stats->query_total_load_node_time = 0;
    index->stats->query_total_cpu_time = 0;
    index->stats->query_total_time = 0;    

    index->stats->query_total_seq_input_count = 0;
    index->stats->query_total_seq_output_count = 0;
    index->stats->query_total_rand_input_count = 0;
    index->stats->query_total_rand_output_count = 0;
    
    index->stats->query_total_loaded_nodes_count = 0;
    index->stats->query_total_checked_nodes_count = 0;    
    index->stats->query_total_loaded_ts_count = 0;
    index->stats->query_total_checked_ts_count = 0;
    
    index->stats->query_exact_distance = 0;
    index->stats->query_exact_node_filename = NULL;
    index->stats->query_exact_node_size = 0;    
    index->stats->query_exact_node_level = 0;

    index->stats->query_approx_distance = 0;
    index->stats->query_approx_node_filename = NULL;
    index->stats->query_approx_node_size = 0;
    index->stats->query_approx_node_level = 0;

    index->stats->query_lb_distance = 0;

    index->stats->query_tlb = 0;        
    index->stats->query_eff_epsilon = 0;    
    index->stats->query_pruning_ratio = 0;

    
    index->stats->queries_refine_input_time = 0;
    index->stats->queries_refine_output_time = 0;
    index->stats->queries_refine_load_node_time = 0;
    index->stats->queries_refine_cpu_time = 0;
    index->stats->queries_refine_total_time = 0;    

    index->stats->queries_refine_seq_input_count = 0;
    index->stats->queries_refine_seq_output_count = 0;
    index->stats->queries_refine_rand_input_count = 0;
    index->stats->queries_refine_rand_output_count = 0;        
    
    index->stats->queries_filter_input_time = 0;
    index->stats->queries_filter_output_time = 0;
    index->stats->queries_filter_load_node_time = 0;
    index->stats->queries_filter_cpu_time = 0;
    index->stats->queries_filter_total_time = 0;    

    index->stats->queries_filter_seq_input_count = 0;
    index->stats->queries_filter_seq_output_count = 0;
    index->stats->queries_filter_rand_input_count = 0;
    index->stats->queries_filter_rand_output_count = 0;    

    index->stats->queries_total_input_time = 0;
    index->stats->queries_total_output_time = 0;
    index->stats->queries_total_load_node_time = 0;    
    index->stats->queries_total_cpu_time = 0;
    index->stats->queries_total_time = 0;    

    index->stats->queries_total_seq_input_count = 0;
    index->stats->queries_total_seq_output_count = 0;
    index->stats->queries_total_rand_input_count = 0;
    index->stats->queries_total_rand_output_count = 0;        

    index->stats->queries_min_eff_epsilon = FLT_MAX;
    index->stats->queries_max_eff_epsilon = 0;
    index->stats->queries_sum_eff_epsilon = 0;
    index->stats->queries_sum_squares_eff_epsilon = 0;
    index->stats->queries_avg_eff_epsilon = 0;
    index->stats->queries_sd_eff_epsilon = 0;
    
    index->stats->queries_min_pruning_ratio =  FLT_MAX;
    index->stats->queries_max_pruning_ratio = 0;
    index->stats->queries_sum_pruning_ratio = 0;
    index->stats->queries_sum_squares_pruning_ratio = 0;
    index->stats->queries_avg_pruning_ratio = 0;
    index->stats->queries_sd_pruning_ratio = 0;

    index->stats->queries_min_tlb =  FLT_MAX;
    index->stats->queries_max_tlb = 0;
    index->stats->queries_sum_tlb = 0;
    index->stats->queries_sum_squares_tlb = 0;
    index->stats->queries_avg_tlb = 0;
    index->stats->queries_sd_tlb = 0;    


    index->stats->tlb_ts_count = 0;
    index->stats->eff_epsilon_queries_count = 0;

    
//    index->stats->total_queries_count = 0;
    
    index->stats->total_input_time = 0;
    index->stats->total_output_time = 0;
    index->stats->total_load_node_time = 0;
    index->stats->total_cpu_time = 0;
    index->stats->total_time = 0;
    index->stats->total_time_sanity_check = 0;
    
    index->stats->total_seq_input_count = 0;
    index->stats->total_seq_output_count = 0;
    index->stats->total_rand_input_count = 0;
    index->stats->total_rand_output_count = 0;
    
    index->stats->total_parse_time = 0;	
    index->stats->total_ts_count = 0;	
    
    return SUCCESS;
}

void vaplus_get_index_stats(struct vaplus_index * index)
{
  index->stats->total_seq_input_count = index->stats->idx_building_seq_input_count
                                      + index->stats->idx_writing_seq_input_count
                                      + index->stats->idx_reading_seq_input_count;
                                      //    + index->stats->queries_total_seq_input_count;
  index->stats->total_seq_output_count = index->stats->idx_building_seq_output_count
                                      + index->stats->idx_writing_seq_output_count
                                      + index->stats->idx_reading_seq_output_count;
                                      //+ index->stats->queries_total_seq_output_count;
  index->stats->total_rand_input_count = index->stats->idx_building_rand_input_count
                                      + index->stats->idx_writing_rand_input_count
                                      + index->stats->idx_reading_rand_input_count;
                                      // + index->stats->queries_total_rand_input_count;
  index->stats->total_rand_output_count = index->stats->idx_building_rand_output_count
                                      + index->stats->idx_writing_rand_output_count
                                      + index->stats->idx_reading_rand_output_count;
                                         //+ index->stats->queries_total_rand_output_count;
  
  index->stats->total_input_time = index->stats->idx_building_input_time
                                 + index->stats->idx_writing_input_time
                                  + index->stats->idx_reading_input_time;
                                 //   + index->stats->queries_total_input_time;
  index->stats->total_output_time = index->stats->idx_building_output_time
                                  + index->stats->idx_writing_output_time
                                  + index->stats->idx_reading_output_time;
                                  //    + index->stats->queries_total_output_time;  
  index->stats->total_cpu_time    = index->stats->idx_building_cpu_time
                                  + index->stats->idx_writing_cpu_time
                                  + index->stats->idx_reading_cpu_time;
                                 //    + index->stats->queries_total_cpu_time;

  index->stats->total_time    = index->stats->total_input_time
                              + index->stats->total_output_time
                              + index->stats->total_cpu_time;

  //index->stats->total_time_sanity_check = total_time;
  
  //index->stats->load_node_time = load_node_time;
  index->stats->total_parse_time = total_parse_time;

  //index->stats->loaded_nodes_count = loaded_nodes_count;
  index->stats->leaf_nodes_count = leaf_nodes_count;
  index->stats->empty_leaf_nodes_count = empty_leaf_nodes_count;
  
  //index->stats->checked_nodes_count = checked_nodes_count;
  index->stats->total_nodes_count = total_nodes_count - empty_leaf_nodes_count;
  index->stats->total_ts_count = total_ts_count;

  //vaplus_get_index_footprint(index);

}


void vaplus_get_index_footprint(struct vaplus_index * index)
{

    const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 9));
    filename = strcpy(filename, index->settings->root_directory);
    filename = strcat(filename, "root.idx\0");

    struct stat st;
    unsigned int  count_leaves;

    if (stat(filename, &st) == 0)
    {
      index->stats->idx_size_bytes = (long long) st.st_size;
      index->stats->idx_size_blocks = (long long) st.st_blksize;
    }

    count_leaves = index->stats->leaf_nodes_count;
    
    index->stats->avg_fill_factor =  ((double) index->stats->sum_fill_factor) / count_leaves  ;
    index->stats->sum_squares_fill_factor -= (pow( index->stats->sum_fill_factor,2) / count_leaves);
    index->stats->sd_fill_factor =  sqrt(((double) index->stats->sum_squares_fill_factor) / count_leaves);

    index->stats->avg_height     =  ((double) index->stats->sum_height) / count_leaves;
    index->stats->sum_squares_height -= (pow( index->stats->sum_height,2) / count_leaves);
    index->stats->sd_height =  sqrt(((double) index->stats->sum_squares_height) / count_leaves);

    free(filename);
}

  
void vaplus_print_index_stats(struct vaplus_index * index, char * dataset)
{
      //  id = -1 for index and id = query_id for queries
        int id = -1;
        printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_output_time/1000000,
	       dataset,
	       id);
        printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_rand_output_count,
	       dataset,
	       id); 

        printf("Index_writing_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_writing_input_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_writing_output_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_cpu_time_secs\t%lf\t%s\t%d\n",
               index->stats->idx_writing_cpu_time/1000000,
	       dataset,
	       id);
	
        printf("Index_writing_total_time_secs\t%lf\t%s\t%d\n",
               index->stats->idx_writing_total_time/1000000,
	       dataset,
	       id);	

        printf("Index_writing_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_seq_input_count,
	       dataset,
	       id);

        printf("Index_writing_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_seq_output_count,
	       dataset,
	       id);

        printf("Index_writing_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_rand_input_count,
	       dataset,
	       id);

        printf("Index_writing_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_rand_output_count,
	       dataset,
	       id);	

        printf("Index_reading_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_reading_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_output_time/1000000,
	       dataset,
	       id);
        printf("Index_reading_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_reading_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_reading_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_rand_output_count,
	       dataset,
	       id);
	
        printf("Index_total_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_total_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_total_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_total_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->total_rand_output_count,
	       dataset,
	       id); 

        printf("Internal_nodes_count\t%lu\t%s\t%d\n",
	       (index->stats->total_nodes_count - index->stats->leaf_nodes_count),
	       dataset,
	       id);

        printf("Leaf_nodes_count\t%lu\t%s\t%d\n",
	       index->stats->leaf_nodes_count,
	       dataset,
	       id);

	printf("Empty_leaf_nodes_count\t%lu\t%s\t%d\n",
               index->stats->empty_leaf_nodes_count,	       
	       dataset,
	       id);

	printf("Total_nodes_count\t%lu\t%s\t%d\n",
	       index->stats->total_nodes_count,
	       dataset,
	       id);

	double size_MB =  (index->stats->idx_size_bytes)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);

	printf("Minimum_fill_factor\t%f\t%s\t%d\n",
	       index->stats->min_fill_factor,	       
	       dataset,
	       id);	

	printf("Maximum_fill_factor\t%f\t%s\t%d\n",
	       index->stats->max_fill_factor,	       
	       dataset,
	       id);

	printf("Average_fill_factor\t%f\t%s\t%d\n",
	       index->stats->avg_fill_factor,	       
	       dataset,
	       id);	

	printf("SD_height\t%f\t%s\t%d\n",
	       index->stats->sd_height,	       
	       dataset,
	       id);		

	printf("Minimum_height\t%u\t%s\t%d\n",
	       index->stats->min_height,	       
	       dataset,
	       id);	

	printf("Maximum_height\t%u\t%s\t%d\n",
	       index->stats->max_height,	       
	       dataset,
	       id);

	printf("Average_height\t%f\t%s\t%d\n",
	       index->stats->avg_height,	       
	       dataset,
	       id);	

	printf("SD_height\t%f\t%s\t%d\n",
	       index->stats->sd_height,	       
	       dataset,
	       id);			

	printf("Total_ts_count\t%u\t%s\t%d\n",
	       index->stats->total_ts_count,	       
	       dataset,
	       id);
        
	//for (int i = 0; i < index->stats->leaves_counter; ++i)	  
	//{
 	//    double fill_factor = ((double) index->stats->leaves_sizes[i])/index->settings->max_leaf_size;
  	//    printf("Leaf_report_node_%d \t Height  %d  \t%s\t%d\n",
	//	   (i+1),
	//	   index->stats->leaves_heights[i],
	//           dataset,
	//           id);
  	//    printf("Leaf_report_node_%d \t Fill_Factor  %f \t%s\t%d\n",
	//		   (i+1),
	//	   fill_factor,	       
	//           dataset,
	//           id);	    
	//}
         
	
}


void get_query_stats(struct vaplus_index * index, ts_type * ts, ts_type * paa, int query_num)
{

  if (index->stats->total_ts_count != 0)
    {
        index->stats->query_pruning_ratio = 1.0 - ((double)index->stats->query_refine_checked_ts_count/
						 index->stats->total_ts_count);
        double ratio = index->stats->query_pruning_ratio;
        index->stats->queries_sum_pruning_ratio += ratio;
        index->stats->queries_sum_squares_pruning_ratio += pow(ratio,2);

        if (ratio < index->stats->queries_min_pruning_ratio)
        {
  	   index->stats->queries_min_pruning_ratio = ratio;
        }
        if (ratio > index->stats->queries_max_pruning_ratio)
        {
  	   index->stats->queries_max_pruning_ratio = ratio;
        }
    }
    

    if (index->stats->query_exact_distance != 0)
    {
        index->stats->query_eff_epsilon =   (index->stats->query_approx_distance
					    -index->stats->query_exact_distance
					    )
					    / index->stats->query_exact_distance;

        double eff_epsilon = index->stats->query_eff_epsilon;
	
        index->stats->queries_sum_eff_epsilon += eff_epsilon;

        index->stats->queries_sum_squares_eff_epsilon += pow(eff_epsilon,2);

	
        if (eff_epsilon < index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_min_eff_epsilon = eff_epsilon;
        }
        if (eff_epsilon > index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_max_eff_epsilon = eff_epsilon;
        }	
    }
    else
    {  
       ++(index->stats->eff_epsilon_queries_count);
    }


    if (index->stats->tlb_ts_count != 0)
    {
 	index->stats->query_tlb = index->stats->query_sum_tlb/index->stats->tlb_ts_count; //the average over all the data set
  
        double tlb  = index->stats->query_tlb;

        index->stats->queries_sum_tlb += tlb;
        index->stats->queries_sum_squares_tlb += pow(tlb,2);	

        if (tlb < index->stats->queries_min_tlb)
        {
  	   index->stats->queries_min_tlb = tlb;
        }
        if (tlb > index->stats->queries_min_tlb)
        {
  	   index->stats->queries_max_tlb = tlb;
        }			
    }
 
} 
*/
/*


void vaplus_get_query_stats(struct vaplus_index * index, int query_num)
{

  if (total_ts_count != 0)
    {
        index->stats->query_pruning_ratio = 1.0 - ((double)index->stats->query_checked_ts_count/
						 index->stats->total_ts_count);
        double ratio = index->stats->query_pruning_ratio;
        index->stats->queries_sum_pruning_ratio += ratio;
        index->stats->queries_sum_squares_pruning_ratio += pow(ratio,2);

        if (ratio < index->stats->queries_min_pruning_ratio)
        {
  	   index->stats->queries_min_pruning_ratio = ratio;
        }
        if (ratio > index->stats->queries_max_pruning_ratio)
        {
  	   index->stats->queries_max_pruning_ratio = ratio;
        }
    }
    
    if (index->stats->query_exact_distance != 0)
    {
        index->stats->query_tlb = index->stats->query_lb_distance / index->stats->query_exact_distance;		  
        index->stats->query_eff_epsilon =   (index->stats->query_approx_distance
					    -index->stats->query_exact_distance
					    )
					    / index->stats->query_exact_distance;

        double eff_epsilon = index->stats->query_eff_epsilon;
        double tlb  = index->stats->query_tlb;
	
        index->stats->queries_sum_eff_epsilon += eff_epsilon;
        index->stats->queries_sum_squares_eff_epsilon += pow(eff_epsilon,2);

        index->stats->queries_sum_tlb += tlb;
        index->stats->queries_sum_squares_tlb += pow(tlb,2);	
	
        if (eff_epsilon < index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_min_eff_epsilon = eff_epsilon;
        }
        if (eff_epsilon > index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_max_eff_epsilon = eff_epsilon;
        }	

        if (tlb < index->stats->queries_min_tlb)
        {
  	   index->stats->queries_min_tlb = tlb;
        }
        if (tlb > index->stats->queries_min_tlb)
        {
  	   index->stats->queries_max_tlb = tlb;
        }		
	
	++index->stats->total_queries_count;
	
    }


}
*/
/*
void vaplus_print_query_stats(struct vaplus_index * index, unsigned int query_num, char * queries)
{
        printf("Query_filter_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_filter_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_output_count,
	       queries,
	       query_num
	       );
	

        printf("Query_refine_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_refine_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_output_count,
	       queries,
	       query_num
	       );

        printf("Query_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_approx_distance\t%f\t%s\t%d\n",
	       index->stats->query_approx_distance,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_approx_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_approx_node_size\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_level\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_level,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       index->stats->query_exact_distance,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_exact_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_exact_node_size\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_level\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_level,
	       queries,
	       query_num
	       );
	
        printf("Query_lb_distance\t%f\t%s\t%d\n",
	       index->stats->query_lb_distance,
	       queries,
	       query_num
	       );
	
        printf("Query_tlb\t%f\t%s\t%d\n",
	       index->stats->query_tlb,	       
	       queries,
	       query_num
	       );	
	
        printf("Query_pruning_ratio\t%f\t%s\t%d\n",
	       index->stats->query_pruning_ratio,	       
	       queries,
	       query_num
	       );

        printf("Query_eff_epsilon\t%f\t%s\t%d\n",
	       index->stats->query_eff_epsilon,	       
	       queries,
	       query_num
	       );		

}



void print_tlb_stats(struct vaplus_index * index, unsigned int query_num, char * queries)
{

        printf("Query_avg_vaplus_tlb\t%lf\t%s\t%u\n",
	       total_vaplus_tlb/total_ts_count,	       
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

void print_query_stats(struct vaplus_index * index, unsigned int query_num, char * queries)
{

        printf("Query_filter_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_filter_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_output_count,
	       queries,
	       query_num
	       );
        printf("Query_filter_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_filter_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_filter_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_loaded_ts_count,
	       queries,
	       query_num
	       );
	

        printf("Query_refine_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_refine_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_output_count,
	       queries,
	       query_num
	       );

	
        printf("Query_refine_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_refine_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_refine_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_loaded_ts_count,
	       queries,
	       query_num
	       );
 
        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_output_count,
	       queries,
	       query_num
	       );


        printf("Query_total_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_total_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_total_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_approx_distance\t%f\t%s\t%d\n",
	       index->stats->query_approx_distance,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_approx_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_approx_node_size\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_level\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_level,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       index->stats->query_exact_distance,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_exact_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_exact_node_size\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_level\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_level,
	       queries,
	       query_num
	       );
        printf("Query_lb_distance\t%f\t%s\t%d\n",
	       index->stats->query_lb_distance,
	       queries,
	       query_num
	       );
	
        printf("Query_tlb\t%f\t%s\t%d\n",
	       index->stats->query_tlb,	       
	       queries,
	       query_num
	       );	
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	       index->stats->query_pruning_ratio,	       
	       queries,
	       query_num
	       );

        printf("Query_eff_epsilon\t%f\t%s\t%d\n",
	       index->stats->query_eff_epsilon,	       
	       queries,
	       query_num
	       );		

}

*/


void vaplus_index_destroy(struct vaplus_index *index, struct vaplus_node *node, boolean is_index_new)
{
     if (index->buffer_manager != NULL)
	 destroy_buffer_manager(index);

     
     free(index->settings->raw_filename);


  /*
   if (node->level == 0) //root
   {
     if (index->buffer_manager != NULL)
	 destroy_buffer_manager(index);
     free(node->min_values);
     free(node->max_values);
     free(node->vaplus_word);     
   }
   
   struct vaplus_node * curr = node->children;
*/
   /*
   if (curr != NULL)
     {
       for (int i = 0; i < index->settings->num_symbols; ++i)
	 {
	   if(curr->is_leaf)
	     {
	       vaplus_index_destroy(index, curr, is_index_new);               
	     }
	   curr= curr->next;
	 }
       
       curr = node->children;
       struct vaplus_node *prev;
       
       for (int i = 0; i < index->settings->num_symbols; ++i)
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
	   
	   if(prev->vaplus_word != NULL)
	     {
	       free(prev->vaplus_word);
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
*/
}


void destroy_buffer_manager(struct vaplus_index *index)
{
    for (unsigned int i= 0; i < index->settings->fft_size; ++i)
    {
      free(index->bins[i]);
    }
    
    if (index->bins != NULL)   free(index->bins);

    if (index->cells != NULL)   free(index->cells);    

    free (index->buffer_manager->approx_mem_array);
    //free (index->buffer_manager->ts_mem_array);
    if (index->settings->is_new) 
      free (index->buffer_manager->dft_mem_array);

    free(index->buffer_manager);    

 
}
struct vaplus_index * vaplus_index_read(const char* root_directory)
{
        if(chdir(root_directory) != 0)
        {
            fprintf(stderr, "The index directory does not exist. "
                             "Please provide a valid directory.\n");
            exit (-1);
        }

	fprintf(stderr, ">>> Loading index: %s\n", root_directory);

	const char *filename = malloc(sizeof(char) * (strlen(root_directory) + 9));
	filename = strcpy(filename, root_directory);
	filename = strcat(filename, "root.idx");
	//filename = strcat(filename, "root.idx\0");

	
	//printf ("Index file: %s\n",filename);
        COUNT_PARTIAL_RAND_INPUT
        COUNT_PARTIAL_INPUT_TIME_START
	FILE *file = fopen(filename, "rb");
        COUNT_PARTIAL_INPUT_TIME_END

	free(filename);
	
        if(file == NULL)
        {   
           fprintf(stderr, "Error in vaplus_index.c: Could not open"
		" the index file. Reason = %s\n", strerror(errno));
           return FAILURE;
        }

	  
        unsigned int num_cells;


	int raw_filename_size = 0;
	char *raw_filename = NULL;
	unsigned int timeseries_size = 0;
	unsigned int transforms_size = 0;
	unsigned int num_bits = 0;
        double buffered_memory_size = 0;
	boolean is_index_new = 0;
        boolean is_norm = true;
	ts_type norm_factor = 0;
	unsigned int start_offset = 0;	
	unsigned int dataset_size = 0;

        unsigned int * cur_approx_line;
	
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
	fread(&raw_filename_size, sizeof(int), 1, file);
	raw_filename = malloc(sizeof(char) * (raw_filename_size+1));
	fread(raw_filename, sizeof(char), raw_filename_size, file);
	raw_filename[raw_filename_size] = '\0';
	fread(&buffered_memory_size, sizeof(double), 1, file);
	fread(&timeseries_size, sizeof(unsigned int), 1, file);
	fread(&transforms_size, sizeof(unsigned int), 1, file);
	fread(&num_bits, sizeof(unsigned int), 1, file);
	fread(&is_norm, sizeof(boolean), 1, file);
	fread(&norm_factor, sizeof(ts_type), 1, file);
	fread(&start_offset, sizeof(unsigned int), 1, file);			
	fread(&dataset_size, sizeof(unsigned int), 1, file);
	//fread(&is_index_new, sizeof(boolean), 1, file);
	
        COUNT_PARTIAL_INPUT_TIME_END

        struct vaplus_index_settings * index_settings = vaplus_index_settings_init(root_directory,
                                                                    timeseries_size,   
                                                                    transforms_size,
								    num_bits,
								    buffered_memory_size,
								    is_norm,
								    dataset_size,
								    is_index_new);  

        if (index_settings == NULL)
        { 
         fprintf(stderr, "Error main.c:  Could not initialize the index settings.\n");
         return -1;              
        }
       

    	index_settings->raw_filename = malloc(sizeof(char) * 256);
	strcpy(index_settings->raw_filename, raw_filename);
	free(raw_filename);

        struct vaplus_index * index = vaplus_index_init(index_settings);


	//THE BINS TO SET BREAKPOINTS
	//NEEDED FOR QUERIES
	for (int i = 0; i < transforms_size; ++i)
	{
 	  fread(&(index->cells[i]), sizeof(unsigned int), 1, file);
	}
	

        index->bins = NULL;  
        index->bins = calloc(transforms_size,sizeof(ts_type*));
        if(index->bins == NULL) {
          fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for index structure.\n");
          return FAILURE;
        }
       //allocate num_symbols-1 memory slots for each word
       for (int i = 0; i < transforms_size; ++i)
       {
         index->bins[i] = NULL;
         if (index->cells[i] > 1 )
         {
            index->bins[i] = calloc(index->cells[i]+1,sizeof(ts_type));
    
            if(index->bins[i] == NULL) {
             fprintf(stderr,"Error in vaplus_index.c: Could not allocate memory for index structure.\n");
             return FAILURE;
            }

         }
       }

	for (int i = 0; i < transforms_size; ++i)
	{
	  unsigned int num_cells = index->cells[i];
	  for (int j=0; j < num_cells + 1; ++j)
	    {
	      	fread(&(index->bins[i][j]), sizeof(ts_type), 1, file);
	    }
            fprintf(stderr,"Initialized bins[%d][%d] \n", index->settings->fft_size, index->cells[i]+1);             

	}	  

        index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;  
        //unsigned int * approx_record = calloc(transforms_size, sizeof(unsigned int));

	for (unsigned int i = 0; i < dataset_size; ++i)
	{
          cur_approx_line = (unsigned int *) index->buffer_manager->current_approx_record;    
          fread(cur_approx_line, sizeof(unsigned int), transforms_size, file);
          index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;
          COUNT_TOTAL_TS(1)
	}	  


        index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;  

	// NODES AND FILE BUFFERS
	//vaplus_node_write(index, index->first_node, file);
        COUNT_PARTIAL_OUTPUT_TIME_START


	fclose(file);
        COUNT_PARTIAL_OUTPUT_TIME_END

	return index;
}

/*
enum response vaplus_update_index_stats(struct vaplus_index *index, struct vaplus_node *node)
{

  unsigned int height = node->level +1;
  unsigned int threshold = index->settings->max_leaf_size;
  //at this point all time series are on disk
  unsigned int node_size = node->node_size;

  double node_fill_factor = (node_size * 100.0)/threshold;  
    
  if (node_fill_factor < index->stats->min_fill_factor)
  {
    index->stats->min_fill_factor = node_fill_factor;
  }
  if (node_fill_factor > index->stats->max_fill_factor)
  {
    index->stats->max_fill_factor = node_fill_factor;
  }
  if (height < index->stats->min_height)
  {
    index->stats->min_height = height;    
  }
  if (height > index->stats->max_height)
  {
    index->stats->max_height = height;
  }

  index->stats->sum_fill_factor += node_fill_factor ;
  index->stats->sum_squares_fill_factor += pow(node_fill_factor,2) ;

  index->stats->sum_height += height;
  index->stats->sum_squares_height += pow(height,2) ;
     
  COUNT_TOTAL_TS(node_size)
  
}

*/
enum response vaplus_index_write(struct vaplus_index *index)
{

	fprintf(stderr, ">>> Storing index: %s\n", index->settings->root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 9));
	filename = strcpy(filename, index->settings->root_directory);
	filename = strcat(filename, "root.idx\0");

        COUNT_PARTIAL_RAND_OUTPUT
        COUNT_PARTIAL_OUTPUT_TIME_START	  
	FILE *file = fopen(filename, "wb");
        COUNT_PARTIAL_OUTPUT_TIME_END	  

	free(filename);

        if(file == NULL)
        {   
           fprintf(stderr, "Error in vaplus_index.c: Could not open"
		" the index file. Reason = %s\n", strerror(errno));
           return FAILURE;
        }	
	
	int raw_filename_size = strlen(index->settings->raw_filename);
	unsigned int timeseries_size = index->settings->timeseries_size;
	unsigned int transforms_size = index->settings->fft_size;
	unsigned int num_bits = index->settings->num_bits;
	double buffered_memory_size = index->settings->buffered_memory_size;
	boolean is_norm = index->settings->is_norm;
	ts_type norm_factor = index->settings->norm_factor;
	unsigned int start_offset = index->settings->start_offset;
	//boolean is_index_new = index->settings->is_index_new;
	unsigned int dataset_size = index->settings->dataset_size;
	   
        ts_type * cur_coeff_line;
        unsigned int * cur_approx_line;
  
        unsigned int num_cells;
  
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
	fwrite(&raw_filename_size, sizeof(int), 1, file);
	fwrite(index->settings->raw_filename, sizeof(char), raw_filename_size, file);
	fwrite(&buffered_memory_size, sizeof(double), 1, file);
	fwrite(&timeseries_size, sizeof(unsigned int), 1, file);
	fwrite(&transforms_size, sizeof(unsigned int), 1, file);
	fwrite(&num_bits, sizeof(unsigned int), 1, file);
	fwrite(&is_norm, sizeof(boolean), 1, file);
	fwrite(&norm_factor, sizeof(ts_type), 1, file);
	fwrite(&start_offset, sizeof(unsigned int), 1, file);			
	fwrite(&dataset_size, sizeof(unsigned int), 1, file);
	//fwrite(&is_index_new, sizeof(boolean), 1, file);
        COUNT_PARTIAL_OUTPUT_TIME_END


	//THE BINS TO SET BREAKPOINTS
	//NEEDED FOR QUERIES
	for (int i = 0; i < transforms_size; ++i)
	{
 	  fwrite(&index->cells[i], sizeof(unsigned int), 1, file);
	}
	
	
	for (int i = 0; i < transforms_size; ++i)
	{
	  unsigned int num_cells = index->cells[i];
	  for (int j=0; j < num_cells + 1; ++j)
	    {
	      	fwrite(&index->bins[i][j], sizeof(ts_type), 1, file);                
	    }
	}	  
	  
        /*
	for (int i = 0; i < transforms_size; ++i)
	{
          cur_approx_line = (unsigned int *) index->buffer_manager->current_approx_record;    
          fwrite(cur_approx_line, sizeof(unsigned int), dataset_size, file);
          index->buffer_manager->current_approx_record += sizeof(unsigned int) * dataset_size;
	}
        */	  

        unsigned int * approx_record = calloc(transforms_size, sizeof(unsigned int));

	for (int i = 0; i < dataset_size; ++i)
	{
          //restart from the beginning of the array
          index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;  
          for (int j = 0; j < transforms_size; ++j)
          {
              cur_approx_line = (unsigned int *) index->buffer_manager->current_approx_record;   
              approx_record[j] = cur_approx_line[i];
              index->buffer_manager->current_approx_record += sizeof(unsigned int) * dataset_size;
          }
        int coucou;
        if (i == 723755)
           coucou = 0;

          fwrite(approx_record, sizeof(unsigned int), transforms_size, file);
	}	  

        index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;  
        free (approx_record);

	// NODES AND FILE BUFFERS
	//vaplus_node_write(index, index->first_node, file);
        COUNT_PARTIAL_OUTPUT_TIME_START


	fclose(file);
        COUNT_PARTIAL_OUTPUT_TIME_END
	  
	return SUCCESS;

 
}

			
/*
enum response vaplus_node_write(struct vaplus_index *index, struct vaplus_node *node, FILE *file)
{
        COUNT_PARTIAL_SEQ_OUTPUT
        COUNT_PARTIAL_SEQ_OUTPUT	
        COUNT_PARTIAL_SEQ_OUTPUT  
	COUNT_PARTIAL_OUTPUT_TIME_START	  
	fwrite(&(node->is_leaf), sizeof(unsigned char), 1, file);
	fwrite(&(node->node_size), sizeof(unsigned int), 1, file);
	fwrite(&(node->level), sizeof(unsigned int), 1, file);
	fwrite(&(node->prefix_length), sizeof(unsigned int), 1, file);
	fwrite(node->parent, sizeof(struct vaplus_node),1, file);

	for (int i =0; i< node->prefix_length; ++i)
	{
  	      COUNT_PARTIAL_SEQ_OUTPUT
	      fwrite(&(node->vaplus_word[i]),
		     sizeof(unsigned char),
		     1,
		     file);
	}	    
		

	if (node->node_size !=0)
	{
	    for (int i =0; i< index->settings->fft_size; ++i)
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
			  
			if (node->file_buffer != NULL)
 			    flush_buffer_to_disk(index,node,false);
                        COUNT_LEAF_NODE
                        //collect stats while traversing the index
			vaplus_update_index_stats(index, node);
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
	      
	    struct vaplus_node * cur = node->children;
	    	    
	    while (cur != NULL)
	      {
		vaplus_node_write(index, cur,file);
		cur = cur->next;
	      }

	}
      
	
	return SUCCESS;
}

struct vaplus_node * vaplus_node_read(struct vaplus_index *index, FILE *file) {

        struct vaplus_node *node = NULL;
        //to initialize node values for leaf and internal nodes
	node = vaplus_leaf_node_init(index->settings);
	
	unsigned char is_leaf = 0;
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT		     
        COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START	
	fread(&is_leaf, sizeof(unsigned char), 1, file);
	fread(&(node->node_size), sizeof(unsigned int), 1, file);
        fread(&(node->level), sizeof(unsigned int), 1, file);
        fread(&(node->prefix_length), sizeof(unsigned int), 1, file);
        //fread(node->parent, sizeof(struct vaplus_node), 1, file);

	for (int i =0; i< node->prefix_length; ++i)
	{
  	      COUNT_PARTIAL_SEQ_OUTPUT
	      fread(&(node->vaplus_word[i]),
		     sizeof(unsigned char),
		     1,
		     file);
	}	    
		
	
        if (node->node_size != 0 )
	  {
	    for (int i =0; i< index->settings->fft_size; ++i)
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
		vaplus_file_buffer_init(node);

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
			      
			    index->stats->leaves_heights[index->stats->leaves_counter] = node->level + 1;
			    index->stats->leaves_sizes[index->stats->leaves_counter] = node->node_size;
			    ++(index->stats->leaves_counter);
			    vaplus_update_index_stats(index, node);			  
			}
                 }
		else
		{
			node->filename = NULL;
			node->node_size = 0;
		}
		//get all timeseries for this node into its this file buffer

		//get_all_time_series_in_node(index,node);
		//node->file_buffer->disk_count = 0;
	}
	else {
	  node->file_buffer = NULL;
	  node->is_leaf = 0;
	  node->filename = NULL;
	  	  
	  node->children = vaplus_node_read(index, file);
	  struct vaplus_node * cur = node->children;
	  unsigned long num_children = index->settings->num_symbols;
	  
	  for (unsigned long i = 0; i < num_children -1 ; ++i )
	    {
	      cur->next =vaplus_node_read(index, file);
	      cur = cur->next;
	    }	  
	    	     
	}
	  
	return node;
}

*/
/*
void vaplus_divide_equi_width_hist(struct vaplus_index *index)
{

  ts_type * cur_coeff_line;
  unsigned int dataset_size = index->settings->dataset_size;

  fprintf(stderr, "Using Equi-width histograms\n");
  
  for (int i = 0; i < index->settings->fft_size; ++i)
  {
       cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
       //index->buffer_manager->current_record_index++; no need because we will read from beg after
	   
      ts_type first = cur_coeff_line[0];
      ts_type last = cur_coeff_line[dataset_size-1];
      
      ts_type interval_width = (last-first) / (ts_type)(index->settings->num_symbols);
      for (int j=0; j < index->settings->num_symbols-1; ++j)
      {
	index->bins[i][j] = interval_width*(j+1)+first;
      }
      index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }
    //reset current record to point to the start of the array
    index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;  
}

//enum response vaplus_divide_equi_depth_hist(struct vaplus_index *index)
void vaplus_divide_equi_depth_hist(struct vaplus_index *index)
{

  ts_type * cur_coeff_line;
  unsigned int dataset_size = index->settings->dataset_size;

  ts_type depth = (ts_type) index->settings->dataset_size /index->settings->num_symbols;
  fprintf(stderr, "Using Equi-depth histograms\n");
  
  for (int i = 0; i < index->settings->fft_size; ++i)
  {
      cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;  

      int pos = 0;
      int count = 0;
      for (int j=0; j < index->settings->dataset_size; ++j)
	{
	  ++count;
	  unsigned int ceiling = ceil (depth * (pos+1));
	  if (count > ceiling &&
	      (pos == 0 || index->bins[i][pos-1] != cur_coeff_line[j]))
	    {
	      index->bins[i][pos] =  cur_coeff_line[j];
	      pos++;
	    }


	}
      index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }
    //reset current record to point to the start of the array
  index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;  
}
*/
/*
void vaplus_print_bins(struct vaplus_index *index)
{
  fprintf(stderr,"[\n");
  for (int i = 0; i < index->settings->fft_size; ++i)
  {
      fprintf(stderr,"-Inf\t");
      for (int j=0; j < index->settings->num_symbols-1; ++j)
       {
         ts_type value = roundf(index->bins[i][j]*100.0)/100.0;
         if (value == FLT_MAX)	  
	   fprintf(stderr,", Inf\t\n");
	 else
	   fprintf(stderr,",\t%g\t",value);
       }
       fprintf(stderr,";\n");
  }
  fprintf(stderr,"]\n");
  
}
*/

void vaplus_print_bins(struct vaplus_index *index)
{

  
  fprintf(stderr,"[\n");
  for (int i = 0; i < index->settings->fft_size; ++i)
  {
      unsigned int  num_cells = index->cells[i];
       for (unsigned int j = 0; j < num_cells; j++)
       {
           ts_type value = roundf(index->bins[i][j]*100.0)/100.0;
	   fprintf(stderr,",\t%g\t",value);
       }
       fprintf(stderr,";\n");
  }
  fprintf(stderr,"]\n");  
}



void vaplus_index_set_bins(struct vaplus_index *index, char * ifilename)
{

   unsigned int ts_loaded;
   
    //first build the bins out of a sample of the data
      //read whole sample in memory
   ts_loaded = 0;
   FILE * ifile;
   COUNT_PARTIAL_INPUT_TIME_START
   ifile = fopen (ifilename,"rb");
   COUNT_PARTIAL_INPUT_TIME_END

   index->settings->raw_filename = malloc(sizeof(char) * 256);
   strcpy(index->settings->raw_filename, ifilename);

    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int dataset_size = index->settings->dataset_size;
    int transforms_size = index->settings->fft_size;
      
    ts_type * ts_orig= NULL;
    ts_orig = (ts_type*) malloc ( sizeof ( ts_type ) * ts_length); 

    ts_type * ts= NULL;
    ts = (ts_type*) fftwf_malloc ( sizeof ( ts_type ) * ts_length); 

    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *)fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;
 
    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * transform = NULL;
    transform = (ts_type*) fftwf_malloc ( sizeof ( ts_type ) * ts_length);

   //fseek(ifile, 0L, SEEK_SET);
   while (ts_loaded < dataset_size) 
   {
           //ts_type * ts_orig  = (ts_type *) index->buffer_manager->current_ts_record;
           //fread(ts_orig, sizeof(ts_type), ts_length, ifile);
	   //for (int i =0; i< ts_length; ++i)
	   //  ts[i] =ts_orig[i];
           //index->buffer_manager->current_ts_record += sizeof(ts_typeà) * ts_length;

           COUNT_PARTIAL_SEQ_INPUT		  
	   COUNT_PARTIAL_INPUT_TIME_START
           fread(ts_orig, sizeof(ts_type), ts_length, ifile);
	   COUNT_PARTIAL_INPUT_TIME_END
           COUNT_TOTAL_TS(1)

           //index->buffer_manager->current_record_index++; no need because we will read from beg after
           // fftwf_execute ( plan_forward);
   	   for (unsigned int i =0; i< ts_length; ++i)
	      ts[i] =ts_orig[i];

            fft_from_ts(index, ts, ts_out, transform, plan_forward);

  	     ts_type *cur_coeff_line;

    	     index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
             for (int j = 0; j < transforms_size; ++j)	       
	     {
		 cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
		 ts_type value =(ts_type) roundf(transform[j]*100.0)/100.0;      
                 // ts_type value =transform[j];      
		 //we will use the dft memory to temporarily store
		 //the transforms, which will be discarded
		 //once the bins are set. The dft memory array will
		 //then store the transforms for the overlapping
		 //subsequences.
		 //index->buffer_manager->dft_mem_array[j-offset][i] = value;
		 cur_coeff_line[ts_loaded] = value;      
		 //fprintf(stderr,"order_line[%d][%d] = %g \n", j,i,index->order_line[j][i]);
		 index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;     
	     }
	     //reset current record to point to the start of the array
	     index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;

	   ++ts_loaded;
   }

   fftwf_destroy_plan (plan_forward);
   fftwf_free (ts);
   fftwf_free (transform);
   free (ts_orig);
   fftwf_free (ts_out);

   //reset to start from beginning of ts memory buffer
   //index->buffer_manager->current_ts_record = index->buffer_manager->ts_mem_array;
   index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
      
   // Train Quantization bins on the sample
      
   vaplus_index_bins_init(index);

   //initialize borders as with original VA-file: equi-depth
   vaplus_index_borders_init(index);

   //improve borders using k-means
   vaplus_index_borders_kmeans(index);
       
   //vaplus_index_fill_order_line(index);
  
   COUNT_PARTIAL_INPUT_TIME_START
   fclose(ifile);
   COUNT_PARTIAL_INPUT_TIME_END

   //vaplus_print_bins(index);
   //reset_dft_memory_size(index);

   //index->buffer_manager->current_ts_record = index->buffer_manager->ts_mem_array;
   index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
   index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;
   index->buffer_manager->current_record_index = 0;


   //return SUCCESS; 
}
/*
void vaplus_index_fill_order_line(struct vaplus_index *index)
{
  double mean;
  double stdev;
 unsigned int dataset_size = index->settings->dataset_size;
  struct vaplus_index_settings * settings = index->settings; 

  unsigned int ts_length = index->settings->timeseries_size;
    
  //int word_length = index->settings->word_length;
  int transforms_size = index->settings->fft_size;

  //boolean is_lower_bounding = index->settings->is_lower_bounding;
  //boolean is_norm_mean = index->settings->is_norm_mean;    

  double norm = 1;
  int offset = 0;    

  fprintf(stderr,"Dataset size %lu\n", dataset_size);     
    
  //fft_from_ts_chunk(index);

  ts_type * cur_coeff_line;

  for (int j = 0; j < transforms_size; ++j)
  {
    cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;

    qsort(cur_coeff_line,
	  dataset_size,
	  sizeof(ts_type),
	  compare_ts_type);

    index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }
  
  index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;  
 
}

*/

void vaplus_index_borders_init(struct vaplus_index *index)
{


  double mean;
  double stdev;
  //unsigned int sample_size = index->settings->sample_size;
  unsigned int dataset_size = index->settings->dataset_size;
  
  struct vaplus_index_settings * settings = index->settings; 

  unsigned int ts_length = index->settings->timeseries_size;
    
  //int word_length = index->settings->word_length;
  int transforms_size = index->settings->fft_size;

  //boolean is_lower_bounding = index->settings->is_lower_bounding;
  //boolean is_norm_mean = index->settings->is_norm_mean;    

  double norm = 1;
  int offset = 0;    

  //fprintf(stderr,"Sample size %lu\n", sample_size);     
    
  //fft_from_ts_chunk(index);

  ts_type * cur_coeff_line;

  ts_type * temp_coeff_line = calloc(1, dataset_size * sizeof(ts_type));
  ts_type depth;
  unsigned int num_cells;
  
  for (int i = 0; i < transforms_size; ++i)
  {
    cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
    memcpy(temp_coeff_line, cur_coeff_line, dataset_size * sizeof(ts_type));

    num_cells = index->cells[i];
    depth  = (ts_type) dataset_size / num_cells; 
    
    qsort(temp_coeff_line,
	  dataset_size,
	  sizeof(ts_type),
	  compare_ts_type);


    index->bins[i][0] = temp_coeff_line[0];
      
    for (int j=0; j < num_cells; ++j)
      {
	unsigned int ceiling = ceil (depth * (j+1));
	index->bins[i][j+1] = temp_coeff_line[ceiling-1];
      }
    index->bins[i][num_cells] = temp_coeff_line[dataset_size -1];
    index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }
  
  index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;  

  free(temp_coeff_line);
  
}

void vaplus_index_borders_kmeans(struct vaplus_index *index)
{
  srand(time(0));

  double mean;
  double stdev;
  //unsigned int sample_size = index->settings->sample_size;
  unsigned int dataset_size = index->settings->dataset_size;
  
  struct vaplus_index_settings * settings = index->settings; 

  unsigned int ts_length = index->settings->timeseries_size;
    
  //int word_length = index->settings->word_length;
  int transforms_size = index->settings->fft_size;

  //boolean is_lower_bounding = index->settings->is_lower_bounding;
  //boolean is_norm_mean = index->settings->is_norm_mean;    

  double norm = 1;
  int offset = 0;    

  //fprintf(stderr,"Sample size %lu\n", sample_size);     
    
  //fft_from_ts_chunk(index);

  ts_type * cur_coeff_line;

  ts_type depth;

  ts_type m1;
  ts_type m2;
  ts_type delta = FLT_MAX, delta_new= FLT_MAX;
  ts_type stop=0.001; 
  unsigned int * count_coeff_per_cell;

  ts_type ** coeff_per_cell;
  
  for (int i = 0; i < transforms_size; ++i)
  {
     
    unsigned int num_cells = index->cells[i];
    delta= FLT_MAX;

    if (num_cells > 1 )
    {
      m1 = index->bins[i][0];
      m2 = index->bins[i][num_cells];

      ts_type * centroids = calloc(1, num_cells * sizeof(ts_type));
      coeff_per_cell = calloc(num_cells, sizeof(ts_type*));
      unsigned int * cur_coeff_per_cell_idx = calloc(num_cells,sizeof(unsigned));
      cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
	
      while (true)
      {
  
          /*
	for (int j = 0; j < num_cells; ++j)
	  {
	    //The max size of each cell is dataset_size
	    //values are initialized to FLT_MAX
	    coeff_per_cell[j] = calloc(1, sizeof(ts_type)* dataset_size);

	    //should not be int, but unsigned int, check later
	    for (unsigned int k = 0; k < dataset_size; ++k)
	      coeff_per_cell[j][k] = FLT_MAX;           
	  }
         */
        /*We need to determine first the number of coefficients per cell
          Then we will allocate enough memory for each cell
         */

        count_coeff_per_cell = calloc(num_cells,sizeof(unsigned int));
          
	for (int k = 0; k < dataset_size; ++k)
	  {

	    for (int j = 0; j < num_cells;++j)
	      {
		if (cur_coeff_line[k] >= index->bins[i][j] &&
		    cur_coeff_line[k] < index->bins[i][j+1])
		  {
		    ++count_coeff_per_cell[j];
		    break;
		  }	  
	      }
	  }

	for (int j = 0; j < num_cells; ++j)
	  {
	    coeff_per_cell[j] = calloc(1, sizeof(ts_type)* count_coeff_per_cell[j]);
            cur_coeff_per_cell_idx [j]= 0;           

	    for (unsigned int k = 0; k < count_coeff_per_cell[j]; ++k)
	      coeff_per_cell[j][k] = FLT_MAX;           
	  }


	for (int k = 0; k < dataset_size; ++k)
	  {
	    for (int j = 0; j < num_cells;++j)
	      {
		if (cur_coeff_line[k] >= index->bins[i][j] &&
		    cur_coeff_line[k] < index->bins[i][j+1])
		  {
		    coeff_per_cell[j][cur_coeff_per_cell_idx[j]] = cur_coeff_line[k];
                    ++cur_coeff_per_cell_idx[j];
		    break;
		  }	  
	      }
	  }


	//By now, all coefficients are stored in the right cluster coeff_per_region
	//each cluster i contains count_coeff_per_region[i] coefficients.
      
	for (int j = 0; j < num_cells;++j)
	  {
	    unsigned int count = count_coeff_per_cell[j]; 
	    if (count > 0)
	      {
		centroids[j] = calc_mean(coeff_per_cell[j], 0, count);
	      }
	    else
	      {
		//a random value between 0 and 1
		ts_type random =   (double) rand() / (double) RAND_MAX;
		centroids[j] = random * (m2 -m1) + m1;
	       }
	  }


      //Calculate distance from each value to its centroid
	for (int j = 0; j < num_cells;++j)
	  {
	    unsigned int count = count_coeff_per_cell[j]; 	  
	    for (int k = 0; k < count; ++k)
	      {
		ts_type distance = pow((coeff_per_cell[j][k] - centroids[j]), 2);
		delta_new += distance;
	      }
	  
	  }
      
	qsort(centroids,
	      num_cells,
	      sizeof(ts_type),
	      compare_ts_type);

	//Find the best intervals

	//skip cell 0 since it already has the min value
	for (int j = 1; j < num_cells;++j)
	  {
 	    index->bins[i][j] = (centroids[j-1] + centroids[j])/2 ;	  
	  }

	if ((delta - delta_new) / delta < stop)
	{
	   break; 
	}
	else
	{
	  delta = delta_new;
	}

	for (int j = 0; j < num_cells; ++j)
	  {
	    //The max size of each cell is dataset_size
	    //values are initialized to FLT_MAX
	    free(coeff_per_cell[j]);
	  }
         free(count_coeff_per_cell);      

      }	    
      free(centroids);
      free(coeff_per_cell);      
      free(cur_coeff_per_cell_idx);
	for (int j = 0; j < num_cells; ++j)
	  {
	    //The max size of each cell is dataset_size
	    //values are initialized to FLT_MAX
	    free(coeff_per_cell[j]);
	  }
         free(count_coeff_per_cell);      

    }
    
    index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
  }  
  index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;  
}

/*

void vaplus_index_build(struct vaplus_index *index)
{


  double mean;
  double stdev;

  unsigned int dataset_size = index->settings->dataset_size;
  
  struct vaplus_index_settings * settings = index->settings; 

  unsigned int ts_length = index->settings->timeseries_size;
    
  //int word_length = index->settings->word_length;
  int transforms_size = index->settings->fft_size;

  ts_type * cur_coeff_line;

  unsigned int num_cells;

  for (int i = 0; i < transforms_size; ++i)
    {

    cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
    cur_approx_line = (unsigned char *) index->buffer_manager->current_approx_record;    

    num_cells = index->cells[i];

    for (int k = 0; k < dataset_size; ++k)
     {
       unsigned int c;
       for (c = 0; c < num_cells; c++)
       {
	if (cur_coeff_line[k] >= index->bins[i][c] &&
	    cur_coeff_line[k] < index->bins[i][c+1])
	  {
              cur_approx_line[k] = (unsigned char) (c);
              break;
	  }
       }
     }

     index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
     index->buffer_manager->current_approx_record += sizeof(unsigned char) * dataset_size;
    }
     index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
     index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   
}
*/

void vaplus_index_build(struct vaplus_index *index)
{


  double mean;
  double stdev;

  unsigned int dataset_size = index->settings->dataset_size;
  
  struct vaplus_index_settings * settings = index->settings; 

  unsigned int ts_length = index->settings->timeseries_size;
    
  //int word_length = index->settings->word_length;
  int transforms_size = index->settings->fft_size;

  ts_type * cur_coeff_line;
  unsigned int * cur_approx_line;

  unsigned int num_cells;

  for (int i = 0; i < transforms_size; ++i)
    {

    cur_coeff_line = (ts_type *) index->buffer_manager->current_dft_record;
    cur_approx_line = (unsigned int *) index->buffer_manager->current_approx_record;    

    num_cells = index->cells[i];

    for (int k = 0; k < dataset_size; ++k)
     {
        int coucou;
        unsigned int c;
        if (k == 723755)
           coucou = 0;

	if (cur_coeff_line[k] < index->bins[i][0])
	  {
              cur_approx_line[k] = 0;
	  }
	else if (cur_coeff_line[k] >= index->bins[i][num_cells])
	  {
              cur_approx_line[k] = index->bins[i][num_cells];
	  }
        else 
        { 
         for (c = 0; c < num_cells; c++)
         {
    	  if (cur_coeff_line[k] >= index->bins[i][c] &&
	    cur_coeff_line[k] < index->bins[i][c+1])
	  {
              cur_approx_line[k] = c;
              break;
	  }
         }
       }
     }

     index->buffer_manager->current_dft_record += sizeof(ts_type) * dataset_size;
     index->buffer_manager->current_approx_record += sizeof(unsigned int) * dataset_size;
    }
     index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
     index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   
}


int vaplus_index_get_best_depth(unsigned int count, unsigned int chunk_size) {
    int index_depth = (int) (roundf(logf(count / chunk_size) / logf(8)));
    fprintf(stderr,"Using index depth:\t %d (%d buckets)\n", index_depth, (int) pow(8,index_depth));
    return index_depth;
}
