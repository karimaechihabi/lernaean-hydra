//
//  vaplus_query_engine.c
//  vaplus C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/vaplus_query_engine.h"
#include "../include/vaplus_file_buffer.h"
#include "../include/vaplus_file_buffer_manager.h"
#include "../include/vaplus_index.h"
#include "../include/vaplus_node.h"

#include "../include/pqueue.h"
#ifdef VALUES
#include <values.h>
#endif


struct query_result exact_search (ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx, ts_type * query_ts_reordered, int * query_order, unsigned int offset, struct vaplus_index *index,ts_type minimum_distance)
{


    unsigned long i;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT      
    COUNT_PARTIAL_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END

    ts_type *ts_buffer = (ts_type*) malloc(index->settings->ts_byte_size);
    unsigned int transforms_size = index->settings->fft_size;
    
       
    ts_type bsf = FLT_MAX;
    unsigned int loaded_ts = 0;

     struct query_result bsf_result;
     bsf_result.ed_distance = FLT_MAX;
     bsf_result.file_position = -1;



    index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   

    for(i=0; i<index->settings->dataset_size; i++) {

        unsigned int * vaplus_approx = (unsigned int *) index->buffer_manager->current_approx_record;    

    	ts_type mindist = mindist_fft_to_approx(index, vaplus_approx, query_fft, query_approx);
         //ts_type mindist = 0;
    	if(mindist <= bsf_result.ed_distance) {
		COUNT_PARTIAL_RAND_INPUT
    		COUNT_PARTIAL_SEQ_INPUT		  
		COUNT_PARTIAL_INPUT_TIME_START
                fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
                ++loaded_ts;
    		fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
    		COUNT_PARTIAL_INPUT_TIME_END
                
                 ts_type distance = ts_euclidean_distance_reordered(query_ts_reordered,
						ts_buffer,
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf_result.ed_distance,
						query_order);     

   	        if (distance <= bsf_result.ed_distance)
	        {
	           bsf_result.ed_distance = distance;	           
	           bsf_result.file_position = i;
	        }      



    	}

        index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;

    }

     index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   

      free(ts_buffer);

    COUNT_PARTIAL_INPUT_TIME_START    
    fclose(raw_file);
    COUNT_PARTIAL_INPUT_TIME_END    
  
   COUNT_PARTIAL_TIME_END 
    index->stats->query_refine_total_time  = partial_time;	
        
    index->stats->query_refine_input_time  = partial_input_time;
    index->stats->query_refine_output_time = partial_output_time;
    index->stats->query_refine_load_node_time = partial_load_node_time;    
    index->stats->query_refine_cpu_time    = partial_time
                                           - partial_input_time
                                           - partial_output_time;
    index->stats->query_refine_seq_input_count   = partial_seq_input_count;
    index->stats->query_refine_seq_output_count  = partial_seq_output_count;
    index->stats->query_refine_rand_input_count  = partial_rand_input_count;
    index->stats->query_refine_rand_output_count = partial_rand_output_count;

    index->stats->query_total_time  = partial_time
                                    +  index->stats->query_filter_total_time;
    index->stats->query_total_input_time  = partial_input_time
                                    +  index->stats->query_filter_input_time;
    index->stats->query_total_output_time  = partial_output_time
                                    +  index->stats->query_filter_output_time;
    index->stats->query_total_load_node_time  = partial_load_node_time
                                    +  index->stats->query_filter_load_node_time;
    index->stats->query_total_cpu_time  =  index->stats->query_total_time
                                        -  index->stats->query_total_input_time
                                        -  index->stats->query_total_output_time;
    
    index->stats->query_total_seq_input_count   = partial_seq_input_count
                                   + index->stats->query_filter_seq_input_count;
    index->stats->query_total_seq_output_count   = partial_seq_output_count
                                   + index->stats->query_filter_seq_output_count;
    index->stats->query_total_rand_input_count   = partial_rand_input_count
                                   + index->stats->query_filter_rand_input_count;
    index->stats->query_total_rand_output_count   = partial_rand_output_count
                                   + index->stats->query_filter_rand_output_count;
    
    index->stats->query_exact_distance = sqrtf(bsf_result.ed_distance);
    index->stats->query_refine_loaded_ts_count = loaded_ts;
    index->stats->query_refine_checked_ts_count = loaded_ts;
  
    index->stats->query_total_loaded_nodes_count = loaded_nodes_count 
                                    + index->stats->query_filter_loaded_nodes_count;
    index->stats->query_total_loaded_ts_count = loaded_ts
                                    + index->stats->query_filter_loaded_ts_count;
    index->stats->query_total_checked_nodes_count = checked_nodes_count
                                    + index->stats->query_filter_checked_nodes_count;
    index->stats->query_total_checked_ts_count = loaded_ts
                                    + index->stats->query_filter_checked_ts_count;


    return bsf_result;
}


void vaplus_calc_tlb(ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx, struct vaplus_index *index) {
     
    unsigned long i;
    FILE *raw_file;

    ts_type curr_data_lb_dist = 0;  


    char *ts_str = NULL; //= malloc(sizeof(char) * 2000);
    size_t linecap = 0;
    ssize_t linelen;
    char delimiter = ' ';

    raw_file = fopen(index->settings->raw_filename, "rb");

    ts_type *ts_buffer = (ts_type*) malloc(index->settings->ts_byte_size);
    unsigned int transforms_size = index->settings->fft_size;

    fseek(raw_file, 0, SEEK_SET);
     
    index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   

    for(i=0; i<index->settings->dataset_size; i++) {

        fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
        ts_type curr_exact_dist = ts_euclidean_distance_non_opt(query_ts, ts_buffer, index->settings->timeseries_size);	  

        if (isnan(curr_exact_dist) || isinf(curr_exact_dist) || (curr_exact_dist == 0))
        {
           index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;
	   continue;
        }

        unsigned int * vaplus_approx = (unsigned int *) index->buffer_manager->current_approx_record;    

        curr_data_lb_dist = mindist_fft_to_approx(index, vaplus_approx, query_fft, query_approx);

        ++total_ts_count;

        total_data_tlb += (ts_type)sqrt((double)curr_data_lb_dist / (double)curr_exact_dist);
        //printf("Series # %lu, Total_data_tlb: %g\n", i, total_data_tlb);

        index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;
    }

     index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   


    free(ts_buffer);
    fclose(raw_file);

}

