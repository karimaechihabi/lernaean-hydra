//
//  isax_query_engine.c
//  al_isax
//
//  Created by Kostas Zoumpatianos on 4/13/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

// NOTICE: Adaptive splitting only splits when there is no full data file
// To enable full data file splitting remove this check:
// && !node->has_full_data_file &&
#ifdef VALUES
#include <values.h>
#endif
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "ads/isax_query_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"


ts_type * get_ads_record(unsigned long tid, isax_index *index) {
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_SEQ_INPUT      
    COUNT_PARTIAL_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    fseek(raw_file, tid * index->settings->ts_byte_size, SEEK_SET);
    fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
    fclose(raw_file);
    COUNT_PARTIAL_INPUT_TIME_END    
    return ts_buffer;
}


query_result approximate_search (ts_type *query_ts, ts_type *query_paa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf, isax_index *index) {
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(query_paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);


    if (index->fbl->soft_buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->soft_buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree

        // Adaptive splitting
        if (node->is_leaf && !node->has_full_data_file &&
            (node->leaf_size > index->settings->min_leaf_size))
        {
            split_node(index, node);
        }

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
            if (node->is_leaf && !node->has_full_data_file &&
                (node->leaf_size > index->settings->min_leaf_size))
            {
                split_node(index, node);
            }
        }
        result.distance = calculate_node_distance(index, node, query_ts_reordered,
						  query_order,offset,bsf);

        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}

void ads_calc_tlb(ts_type *query_ts, ts_type *query_paa, isax_index *index) {
     
    unsigned long i;
    FILE *raw_file;

    ts_type curr_data_lb_dist = 0;  

    char *ts_str = NULL; //= malloc(sizeof(char) * 2000);
    size_t linecap = 0;
    ssize_t linelen;
    char delimiter = ' ';

    raw_file = fopen(index->settings->raw_filename, "rb");

    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);

    index->stats->query_sum_tlb = 0;
    index->stats->tlb_ts_count = 0;
     

    for(i=0; i<index->sax_cache_size; i++) {
      fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
      
      float curr_exact_dist = ts_euclidean_distance_non_opt(query_ts, ts_buffer, index->settings->timeseries_size);	  
      if (isnan(curr_exact_dist) || isinf(curr_exact_dist) || (curr_exact_dist == 0))
      {
	  continue;
      }
      
      sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
      curr_data_lb_dist = minidist_paa_to_isax_raw(query_paa, sax, index->settings->max_sax_cardinalities,
						    index->settings->sax_bit_cardinality,
						    index->settings->sax_alphabet_cardinality,
						    index->settings->paa_segments, MINVAL, MAXVAL,
						    index->settings->mindist_sqrt);
      ++total_ts_count;
      /*
      printf("total_ts_count: %lu exact_distance = %g  data_lb_distance = %g\n",
	     total_ts_count,
	     curr_exact_dist,
	     curr_data_lb_dist);
      */
      total_data_tlb += (ts_type)sqrt((double)curr_data_lb_dist / (double)curr_exact_dist);

      
    }
    free(ts_buffer);
    fclose(raw_file);

}


query_result exact_search_serial(ts_type *query_ts, ts_type *query_paa,  ts_type * query_ts_reordered, int * query_order, unsigned int offset, isax_index *index, float minimum_distance, int min_checked_leaves) {

        ts_type bsf = FLT_MAX;

	RESET_BYTES_ACCESSED


	query_result approximate_result = approximate_search(query_ts, query_paa, query_ts_reordered, query_order, offset, bsf, index);
        query_result bsf_result = approximate_result;
        

	int tight_bound = index->settings->tight_bound;
	int aggressive_check = index->settings->aggressive_check;

        /*
        // Early termination...
        
        if (approximate_result.distance == 0) {
           return approximate_result;
        }

        */
        if (approximate_result.distance != 0) {
        
           if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
          	   //approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
                   approximate_result = refine_answer(query_ts, query_paa, query_ts_reordered, query_order, offset, index, approximate_result, minimum_distance,min_checked_leaves);
           }
 
        } 

          COUNT_PARTIAL_TIME_END
          index->stats->query_filter_total_time  = partial_time;	
    
          index->stats->query_filter_input_time  = partial_input_time;
          index->stats->query_filter_output_time = partial_output_time;
          index->stats->query_filter_load_node_time = partial_load_node_time;
          index->stats->query_filter_cpu_time    = partial_time-partial_input_time-partial_output_time;
          index->stats->query_filter_seq_input_count   = partial_seq_input_count;
          index->stats->query_filter_seq_output_count  = partial_seq_output_count;
          index->stats->query_filter_rand_input_count  = partial_rand_input_count;
          index->stats->query_filter_rand_output_count = partial_rand_output_count;

          index->stats->query_filter_loaded_nodes_count = loaded_nodes_count;
          index->stats->query_filter_loaded_ts_count = loaded_ts_count;
          index->stats->query_filter_checked_nodes_count = checked_nodes_count;
          index->stats->query_filter_checked_ts_count = checked_ts_count;

          index->stats->queries_filter_total_time  +=  index->stats->query_filter_total_time;
    
          index->stats->queries_filter_input_time  +=  index->stats->query_filter_input_time;
          index->stats->queries_filter_output_time +=  index->stats->query_filter_output_time;
          index->stats->queries_filter_load_node_time += index->stats->query_filter_load_node_time;
          index->stats->queries_filter_cpu_time    += index->stats->query_filter_cpu_time;
 
          index->stats->queries_filter_seq_input_count   += index->stats->query_filter_seq_input_count;
          index->stats->queries_filter_seq_output_count  += index->stats->query_filter_seq_output_count;
          index->stats->queries_filter_rand_input_count  += index->stats->query_filter_rand_input_count;
          index->stats->queries_filter_rand_output_count += index->stats->query_filter_rand_output_count;
    
          if(approximate_result.node != NULL) {  
             index->stats->query_approx_distance = sqrtf(approximate_result.distance);
             index->stats->query_approx_node_filename = approximate_result.node->filename;
             index->stats->query_approx_node_size = approximate_result.node->leaf_size;;
             index->stats->query_approx_node_level = approximate_result.node->level;
          }
 
          if (approximate_result.distance == 0) {
             index->stats->query_exact_distance = sqrtf(approximate_result.distance);
             index->stats->query_exact_node_filename = approximate_result.node->filename;
             index->stats->query_exact_node_size = approximate_result.node->leaf_size;;
             index->stats->query_exact_node_level = approximate_result.node->level;
             return approximate_result;
          }

	  RESET_QUERY_COUNTERS()
          RESET_PARTIAL_COUNTERS()
          COUNT_PARTIAL_TIME_START
           
       
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/

    unsigned long i;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT      
    COUNT_PARTIAL_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);

#ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
#endif

    SET_APPROXIMATE(approximate_result.distance);
    
    //unsigned long prev_pos = 0;
    //unsigned long new_pos = 0;

    //the number of checked and loaded ts will be the same for SIMS
    unsigned long long loaded_ts = 0;
    //unsigned long long checked_ts = 0;

    for(i=0; i<index->sax_cache_size; i++) {
    	sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
    	float mindist = minidist_paa_to_isax_raw(query_paa, sax, index->settings->max_sax_cardinalities,
						 index->settings->sax_bit_cardinality,
						 index->settings->sax_alphabet_cardinality,
						 index->settings->paa_segments, MINVAL, MAXVAL,	
						 index->settings->mindist_sqrt);

    	if(mindist <= approximate_result.distance) {
    		/*bit_array_set_bit(bitarray, i);*/
    		COUNT_PARTIAL_RAND_INPUT
    		COUNT_PARTIAL_SEQ_INPUT		  
		COUNT_PARTIAL_INPUT_TIME_START

 
    		fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
                ++loaded_ts;
    		fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
    		COUNT_PARTIAL_INPUT_TIME_END
                ts_type dist = ts_euclidean_distance_reordered(query_ts_reordered,
						ts_buffer,
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						approximate_result.distance,
						query_order);     

    		//float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, approximate_result.distance);
    		if(dist < approximate_result.distance) {
     			approximate_result.distance = dist;
                        #ifdef STORE_ANSWER
                        memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
                        #endif
    		}
    	}
    }


    free(ts_buffer);
    COUNT_PARTIAL_INPUT_TIME_START    
    fclose(raw_file);
    COUNT_PARTIAL_INPUT_TIME_END    
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/

   COUNT_PARTIAL_TIME_END 
      //COUNT_TOTAL_TIME_END
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
    
    index->stats->query_exact_distance = sqrtf(approximate_result.distance);
    index->stats->query_exact_node_filename = approximate_result.node->filename;
    index->stats->query_exact_node_size = approximate_result.node->leaf_size;;
    index->stats->query_exact_node_level = approximate_result.node->level;

    //index->stats->query_refine_loaded_nodes_count = loaded_nodes_count;
    index->stats->query_refine_loaded_ts_count = loaded_ts;
    //index->stats->query_refine_checked_nodes_count = checked_nodes_count;
    index->stats->query_refine_checked_ts_count = loaded_ts;
  
    index->stats->query_total_loaded_nodes_count = loaded_nodes_count 
                                    + index->stats->query_filter_loaded_nodes_count;
    index->stats->query_total_loaded_ts_count = loaded_ts
                                    + index->stats->query_filter_loaded_ts_count;
    index->stats->query_total_checked_nodes_count = checked_nodes_count
                                    + index->stats->query_filter_checked_nodes_count;
    index->stats->query_total_checked_ts_count = loaded_ts
                                    + index->stats->query_filter_checked_ts_count;

    //index->stats->query_checked_ts_count = (unsigned long long) (BYTES_ACCESSED/index->settings->ts_byte_size);

    index->stats->queries_refine_total_time  += index->stats->query_refine_total_time;	
    
    index->stats->queries_refine_input_time  += partial_input_time;
    index->stats->queries_refine_output_time += partial_output_time;
    index->stats->queries_refine_load_node_time += partial_load_node_time;        
    index->stats->queries_refine_cpu_time    += partial_time
                                           - partial_input_time
                                           - partial_output_time;
    index->stats->queries_refine_seq_input_count   += partial_seq_input_count;
    index->stats->queries_refine_seq_output_count  += partial_seq_output_count;
    index->stats->queries_refine_rand_input_count  += partial_rand_input_count;
    index->stats->queries_refine_rand_output_count += partial_rand_output_count;


    index->stats->queries_total_input_time  = index->stats->queries_refine_input_time
                                              +index->stats->queries_filter_input_time;	
    index->stats->queries_total_output_time  = index->stats->queries_refine_output_time
                                               +index->stats->queries_filter_output_time;	
    index->stats->queries_total_load_node_time  = index->stats->queries_refine_load_node_time
                                               +index->stats->queries_filter_load_node_time;
    index->stats->queries_total_cpu_time  = index->stats->queries_refine_cpu_time
                                               +index->stats->queries_filter_cpu_time;
    
    index->stats->queries_total_time  = index->stats->queries_refine_total_time
                                         +index->stats->queries_filter_total_time;
    
    index->stats->queries_total_seq_input_count   = index->stats->queries_filter_seq_input_count
                                                    + index->stats->queries_refine_seq_input_count;
    index->stats->queries_total_seq_output_count  = index->stats->queries_filter_seq_output_count
                                                    + index->stats->queries_refine_seq_output_count;
    index->stats->queries_total_rand_input_count  = index->stats->queries_filter_rand_input_count
                                                    + index->stats->queries_refine_rand_input_count;
    index->stats->queries_total_rand_output_count = index->stats->queries_filter_rand_output_count
                                                    + index->stats->queries_refine_rand_output_count;

    
    
    //keep a running sum then divide by the total number of queries
    index->stats->queries_avg_checked_nodes_count += index->stats->query_total_checked_nodes_count;
    index->stats->queries_avg_checked_ts_count += index->stats->query_total_checked_ts_count;
    index->stats->queries_avg_loaded_nodes_count += index->stats->query_total_loaded_nodes_count;
    index->stats->queries_avg_loaded_ts_count += index->stats->query_total_loaded_ts_count; 

    return approximate_result;
}

//query_result refine_answer (ts_type *ts, ts_type *paa, isax_index *index,
//							query_result approximate_bsf_result,
//                           float minimum_distance, int limit) {
query_result refine_answer (ts_type *query_ts, ts_type *query_paa, 
                            ts_type *query_ts_reordered, int * query_order,
                            int offset_q, isax_index *index,
			    query_result approximate_bsf_result,
                           float minimum_distance, int limit) {
	query_result bsf_result = approximate_bsf_result;

 	int tight_bound = index->settings->tight_bound;
	int aggressive_check = index->settings->aggressive_check;

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);

    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
		query_result * mindist_result = malloc(sizeof(query_result));
		mindist_result->distance =  minidist_paa_to_isax(query_paa, current_root_node->isax_values,
                                              current_root_node->isax_cardinalities,
                                              index->settings->sax_bit_cardinality,
                                              index->settings->sax_alphabet_cardinality,
                                              index->settings->paa_segments,
                                              MINVAL, MAXVAL,
                                              index->settings->mindist_sqrt);
		mindist_result->node = current_root_node;
        pqueue_insert(pq, mindist_result);
        current_root_node = current_root_node->next;
    }
    query_result * n;
    int checks = 0;
    while ((n = pqueue_pop(pq)))
    {
		// The best node has a worse mindist, so search is finished!
        if (n->distance >= bsf_result.distance || n->distance > minimum_distance) {
            pqueue_insert(pq, n);
            break;
        }
        else {
	        // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {
				// *** ADAPTIVE SPLITTING ***
		        if (!n->node->has_full_data_file &&
					(n->node->leaf_size > index->settings->min_leaf_size))
		        {
					// Split and push again in the queue
		            split_node(index, n->node);
					pqueue_insert(pq, n);
		            continue;
		        }
				// *** EXTRA BOUNDING ***
				if(tight_bound) {
					float mindistance = calculate_minimum_distance(index, n->node, query_ts, query_paa);
					if(mindistance >= bsf_result.distance)
					{
						free(n);
						continue;
					}
				}
				// *** REAL DISTANCE ***
				checks++;
				float distance = calculate_node_distance(index, n->node, query_ts_reordered, query_order, offset_q,bsf_result.distance);
				if (distance < bsf_result.distance)
				{
					bsf_result.distance = distance;
					bsf_result.node = n->node;
				}
				if(checks > limit) {
					pqueue_insert(pq, n);
					break;
				}
			}
            else {
	            // If it is an intermediate node calculate mindist for children
	            // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
					if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
    				                float distance = calculate_node_distance(index, n->node->left_child, query_ts_reordered, query_order, offset_q,bsf_result.distance);
						if (distance < bsf_result.distance)
						{
							bsf_result.distance = distance;
							bsf_result.node = n->node->left_child;
						}
					}
					else {
                    query_result * mindist_result = malloc(sizeof(query_result));
                    mindist_result->distance =  minidist_paa_to_isax(query_paa, n->node->left_child->isax_values,
                                                                     n->node->left_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
					mindist_result->node = n->node->left_child;
                    pqueue_insert(pq, mindist_result);
					}
                }
                if (n->node->right_child->isax_cardinalities != NULL) {
					if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
    				                float distance = calculate_node_distance(index, n->node->right_child, query_ts_reordered, query_order, offset_q,bsf_result.distance);
						if (distance < bsf_result.distance)
						{
							bsf_result.distance = distance;
							bsf_result.node = n->node->right_child;
						}
					}
					else {
                    query_result * mindist_result = malloc(sizeof(query_result));
					mindist_result->distance =  minidist_paa_to_isax(query_paa, n->node->right_child->isax_values,
                                                                     n->node->right_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
                    mindist_result->node = n->node->right_child;
                    pqueue_insert(pq, mindist_result);
					}
                }
            }

            // Free the node currently popped.
           free(n);
        }
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        free(n);
    }
    // Free the priority queue.
    pqueue_free(pq);

   return bsf_result;
}

