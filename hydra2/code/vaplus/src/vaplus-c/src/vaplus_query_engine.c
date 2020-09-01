//
//  vaplus_query_engine.c
//  ds-tree C version
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
#include "../include/stats.h"

extern struct stats_info stats;

#include "../include/pqueue.h"
#ifdef VALUES
#include <values.h>
#endif




struct query_result exact_search (ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx, ts_type * query_ts_reordered, int * query_order, unsigned int offset, struct vaplus_index *index,ts_type minimum_distance, float epsilon, float delta)
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

    	if(mindist <= bsf_result.ed_distance/(1+epsilon)) {
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
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
     //  printf("Pruning_ratio = : %2.2lf\n", 1-((double)loaded_ts/(double)index->settings->dataset_size) );
  
    COUNT_PARTIAL_TIME_END 
    COUNT_LOADED_TS(loaded_ts)
    stats_update_query_exact_answer(bsf_result.ed_distance);        

    return bsf_result;
}

struct query_result exact_ng_knn_search (ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx,
					 ts_type * query_ts_reordered, int * query_order, unsigned int offset,
					 struct vaplus_index *index,ts_type minimum_distance,
					 unsigned int q_id, char * qfilename,
					 unsigned int k, unsigned int nprobes)
{

  unsigned int curr_size = 0;
  unsigned long i;

  ts_type bsf = FLT_MAX;
  ts_type kth_bsf = FLT_MAX;
  ts_type temp_bsf = FLT_MAX;
  struct query_result bsf_result;
  struct query_result temp;    
  //the number of found NN
  unsigned int found_knn = 0;

  //queue containing kNN results
  struct query_result * knn_results = calloc(k,sizeof(struct query_result));
  for (unsigned int idx = 0; idx < k; ++idx)
  {
      knn_results[idx].file_position = -1;
      knn_results[idx].ed_distance = FLT_MAX;      
  }  
  COUNT_PARTIAL_RAND_INPUT
  COUNT_PARTIAL_RAND_INPUT      
  COUNT_PARTIAL_INPUT_TIME_START
  FILE *raw_file = fopen(index->settings->raw_filename, "rb");
  fseek(raw_file, 0, SEEK_SET);
  COUNT_PARTIAL_INPUT_TIME_END

  ts_type *ts_buffer = (ts_type*) malloc(index->settings->ts_byte_size);
  unsigned int transforms_size = index->settings->fft_size;
           
  unsigned int loaded_ts = 0;

  bsf_result.ed_distance = FLT_MAX;
  bsf_result.file_position = -1;

  index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   

  for(i=0; (i<index->settings->dataset_size) && (loaded_ts < nprobes); i++) {

    unsigned int * vaplus_approx = (unsigned int *) index->buffer_manager->current_approx_record;    

    ts_type mindist = mindist_fft_to_approx(index, vaplus_approx, query_fft, query_approx);

    //get the k-th bsf
    temp = knn_results[k-1];
    kth_bsf =  temp.ed_distance;	
    
    if(mindist < kth_bsf) {
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
							   kth_bsf,
							   query_order);     

      if (distance < kth_bsf)
	{
	  struct query_result object_result;
	  object_result.ed_distance =  distance;		
	  object_result.file_position = i;
	  queue_bounded_sorted_insert(knn_results, object_result, &curr_size,k);
	}      


    }

    index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;

  }

  index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   
  COUNT_LOADED_TS(loaded_ts)

  for (unsigned int pos = found_knn; pos < k; ++pos)
  {
      bsf_result = knn_results[pos];
      found_knn = pos+1;
      COUNT_PARTIAL_TIME_END
      stats_update_knn_query_exact_answer(q_id, found_knn, bsf_result.ed_distance);
      print_knn_query_stats(q_id, found_knn,qfilename);	
      //report all results for found_knn - last_found_knn or print their results
      RESET_QUERY_COUNTERS()
      RESET_PARTIAL_COUNTERS()
      COUNT_PARTIAL_TIME_START	
   }
  
  free(ts_buffer);

  fclose(raw_file);

    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    //  printf("Pruning_ratio = : %2.2lf\n", 1-((double)loaded_ts/(double)index->settings->dataset_size) );
  
  COUNT_PARTIAL_TIME_END 


  return bsf_result;
}

struct query_result exact_de_knn_search (ts_type *query_ts, ts_type *query_fft, unsigned int * query_approx,
					 ts_type * query_ts_reordered, int * query_order, unsigned int offset,
					 struct vaplus_index *index,ts_type minimum_distance,
					 unsigned int q_id, char * qfilename,
					 unsigned int k, float epsilon, float r_delta)
{

  unsigned int curr_size = 0;
  unsigned long i;

  ts_type bsf = FLT_MAX;
  ts_type kth_bsf = FLT_MAX;
  ts_type temp_bsf = FLT_MAX;
  struct query_result bsf_result;
  struct query_result temp;
  
  //the number of found NN
  unsigned int found_knn = 0;

  //queue containing kNN results
  struct query_result * knn_results = calloc(k,sizeof(struct query_result));
  for (unsigned int idx = 0; idx < k; ++idx)
  {
      knn_results[idx].file_position = -1;
      knn_results[idx].ed_distance = FLT_MAX;      
  }  
  COUNT_PARTIAL_RAND_INPUT
  COUNT_PARTIAL_RAND_INPUT      
  COUNT_PARTIAL_INPUT_TIME_START
  FILE *raw_file = fopen(index->settings->raw_filename, "rb");
  fseek(raw_file, 0, SEEK_SET);
  COUNT_PARTIAL_INPUT_TIME_END

  ts_type *ts_buffer = (ts_type*) malloc(index->settings->ts_byte_size);
  unsigned int transforms_size = index->settings->fft_size;
           
  unsigned int loaded_ts = 0;

  bsf_result.ed_distance = FLT_MAX;
  bsf_result.file_position = -1;

  index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   

  for(i=0; i<index->settings->dataset_size; i++) {

    unsigned int * vaplus_approx = (unsigned int *) index->buffer_manager->current_approx_record;    

    ts_type mindist = mindist_fft_to_approx(index, vaplus_approx, query_fft, query_approx);

    //get the k-th bsf
    temp = knn_results[k-1];
    kth_bsf =  temp.ed_distance;	
    if (r_delta != FLT_MAX && (kth_bsf  <= r_delta * (1 + epsilon)))
      break;
    
    if(mindist < kth_bsf/(1+epsilon)) {
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
							   kth_bsf,
							   query_order);     

      if (distance < kth_bsf)
	{
	  struct query_result object_result;
	  object_result.ed_distance =  distance;		
	  object_result.file_position = i;
	  queue_bounded_sorted_insert(knn_results, object_result, &curr_size,k);
	}      


    }

    index->buffer_manager->current_approx_record += sizeof(unsigned int) * transforms_size;

  }

  index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;   
  COUNT_LOADED_TS(loaded_ts)

  for (unsigned int pos = found_knn; pos < k; ++pos)
  {
      bsf_result = knn_results[pos];
      found_knn = pos+1;
      COUNT_PARTIAL_TIME_END
      stats_update_knn_query_exact_answer(q_id, found_knn, bsf_result.ed_distance);
      print_knn_query_stats(q_id, found_knn,qfilename);	
      //report all results for found_knn - last_found_knn or print their results
      RESET_QUERY_COUNTERS()
      RESET_PARTIAL_COUNTERS()
      COUNT_PARTIAL_TIME_START	
   }
  
  free(ts_buffer);

  fclose(raw_file);

    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    //  printf("Pruning_ratio = : %2.2lf\n", 1-((double)loaded_ts/(double)index->settings->dataset_size) );
  
  COUNT_PARTIAL_TIME_END 


  return bsf_result;
}


int queue_bounded_sorted_insert(struct  query_result *q, struct query_result d, unsigned int *cur_size, unsigned int k)
{
    struct query_result  temp;
    size_t i;
    size_t newsize;

    bool is_duplicate = false;
    for (unsigned int itr = 0 ; itr < *cur_size ; ++itr)
      {
	if (q[itr].ed_distance == d.ed_distance)
	  is_duplicate = true;
      }
   
    if (!is_duplicate)
      {
    
	/* the queue is full, overwrite last element*/
	if (*cur_size == k) {      
	  q[k-1].ed_distance = d.ed_distance;
	  q[k-1].file_position = d.file_position;
	}
	else
	  {
	    q[*cur_size].ed_distance = d.ed_distance;
	    q[*cur_size].file_position = d.file_position;      
	    ++(*cur_size);
	  }

	unsigned int idx,j;

	idx = 1;
    
	while (idx < *cur_size)
	  {
	    j = idx;
	    while ( j > 0 &&  ( (q[j-1]).ed_distance > q[j].ed_distance)) 
	      {
		temp = q[j];
		q[j].ed_distance = q[j-1].ed_distance;
		q[j].file_position = q[j-1].file_position;	
		q[j-1].ed_distance = temp.ed_distance;
		q[j-1].file_position = temp.file_position;		
		--j;
	      }
	    ++idx;
	  }
      }
   return 0;
}

