//
//  isax_query_engine.c
//  al_isax
//
//  Created by Kostas Zoumpatianos on 4/13/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "isax_query_engine.h"
#include "isax_first_buffer_layer.h"
#include "sax/sax.h"
#include "pqueue.h"
#ifdef VALUES
#include <values.h>
#endif

  //query_result approximate_search (ts_type *ts, ts_type *paa, isax_index *index) {
void calculate_node_knn_distance (isax_index *index, isax_node *node,
				  ts_type *query_ts_reordered, int *query_order,
				  unsigned int offset, ts_type bsf,
				  unsigned int k,
				  struct query_result  *knn_results,
				  unsigned int *cur_size);

query_result approximate_search (ts_type *query_ts, ts_type *query_paa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf,isax_index *index) {

    query_result result;
    
    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(query_paa, sax, index->settings->paa_segments, 
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);
    
    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);
    
    if (index->fbl->buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree
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
        }
        result.distance = calculate_node_distance(index, node, query_ts_reordered,
						  query_order,offset,bsf,&result.raw_file_position);
        
	// result.distance = calculate_node_distance(index, node, ts, 
        //                                          &result.raw_file_position);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = MAXFLOAT;
    }
    
    free(sax);
    
    return result;
}

void approximate_knn_search (ts_type *query_ts, ts_type *query_paa,
			     ts_type * query_ts_reordered, int * query_order,
			     unsigned int offset, ts_type bsf,isax_index *index,
			     struct query_result *knn_results, unsigned int k,
			     unsigned int * curr_size
			     )
{

    query_result result;
    
    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(query_paa, sax, index->settings->paa_segments, 
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);
    
    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);
    
    if (index->fbl->buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree
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
        }
        //result.distance = calculate_node_distance(index, node, query_ts_reordered,
	//				  query_order,offset,bsf,&result.raw_file_position);
	calculate_node_knn_distance(index, node, query_ts_reordered,
				    query_order,offset,bsf,
				    k,knn_results,curr_size);    
        
	// result.distance = calculate_node_distance(index, node, ts, 
        //                                          &result.raw_file_position);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = MAXFLOAT;
    }
    
    free(sax);
    
    return result;
}


query_result exact_search (ts_type *query_ts, ts_type *query_paa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, isax_index *index,ts_type minimum_distance, ts_type epsilon, ts_type delta)

{

    ts_type bsf = FLT_MAX;
    //file_position_type *bsf_position;
    //*bsf_position = -1;
    query_result approximate_result = approximate_search(query_ts, query_paa, query_ts_reordered, query_order, offset, bsf, index);

     //query_result approximate_result = approximate_search(ts, paa, index);

    //query_result bsf_result = approximate_result;    

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
    
	    //bsf_result = approximate_result;

	  pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
	
    
    //    printf("%s @ %lf\n", approximate_result.node->filename, approximate_result.distance);
    
    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        pqueue_insert(pq, &approximate_result);
    }
    
	query_result *do_not_remove = &approximate_result;
    
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
    query_result bsf_result =  approximate_result;
    
    while ((n = pqueue_pop(pq)))
    {
		//printf("pop: [%lf]\n", n->distance);
      
        if (n->distance > bsf_result.distance/(1 + epsilon)) {
	  //pqueue_insert(pq, n);
	  break;
        }
           // If it is a leaf check its real distance.
	if (n->node->is_leaf) {
	      
	  file_position_type raw_file_position = -1;
	  //float distance = calculate_node_distance(index, n->node, ts, 
	  //                                         &raw_file_position);
	  ts_type distance = calculate_node_distance(index, n->node, query_ts_reordered,
						     query_order,offset,
						     bsf_result.distance,
						     &raw_file_position);

	  
	  if (distance < bsf_result.distance)
	    {
	      bsf_result.distance = distance;
	      bsf_result.node = n->node;
	      bsf_result.raw_file_position = raw_file_position;
	    }
	}
	// If it is an intermediate node calculate mindist for children
	// and push them in the queue
	else {
	  ts_type child_distance;
	  
	  if (n->node->left_child->isax_cardinalities != NULL) {
	    child_distance =  minidist_paa_to_isax(query_paa, n->node->left_child->isax_values, 
						   n->node->left_child->isax_cardinalities,
						   index->settings->sax_bit_cardinality,  
						   index->settings->sax_alphabet_cardinality, 
						   index->settings->paa_segments, 
						   MINVAL, MAXVAL,
						   index->settings->mindist_sqrt);	    

	    if ( child_distance <= bsf_result.distance/(1 + epsilon) )
	      {

		query_result * mindist_result = malloc(sizeof(query_result));
		mindist_result->distance =  child_distance;
		mindist_result->node = n->node->left_child;
		pqueue_insert(pq, mindist_result);
	      }
	  }
	  if (n->node->right_child->isax_cardinalities != NULL) {
	    child_distance =  minidist_paa_to_isax(query_paa, n->node->right_child->isax_values, 
							     n->node->right_child->isax_cardinalities,
							     index->settings->sax_bit_cardinality,  
							     index->settings->sax_alphabet_cardinality, 
							     index->settings->paa_segments, 
							     MINVAL, MAXVAL,
							     index->settings->mindist_sqrt);

	    if ( child_distance <= bsf_result.distance/(1 + epsilon) )
	      {
		query_result * mindist_result = malloc(sizeof(query_result));
		mindist_result->distance =  child_distance;
		mindist_result->node = n->node->right_child;
		pqueue_insert(pq, mindist_result);
	      }
	  }

	}
            
	// Free the node currently popped.
	if(n != do_not_remove)
	  free(n);
    }
    
    
    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
        	free(n);
	}
    // Free the priority queue.
    
    pqueue_free(pq);
    
    //printf("BEST: %lf\n", bsf_result.distance);


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
    
    index->stats->query_exact_distance = sqrtf(bsf_result.distance);
    index->stats->query_exact_node_filename = bsf_result.node->filename;
    index->stats->query_exact_node_size = bsf_result.node->leaf_size;;
    index->stats->query_exact_node_level = bsf_result.node->level;

    index->stats->query_refine_loaded_nodes_count = loaded_nodes_count;
    index->stats->query_refine_loaded_ts_count = loaded_ts_count;
    index->stats->query_refine_checked_nodes_count = checked_nodes_count;
    index->stats->query_refine_checked_ts_count = checked_ts_count;
  
    index->stats->query_total_loaded_nodes_count = loaded_nodes_count 
                                    + index->stats->query_filter_loaded_nodes_count;
    index->stats->query_total_loaded_ts_count = loaded_ts_count
                                    + index->stats->query_filter_loaded_ts_count;
    index->stats->query_total_checked_nodes_count = checked_nodes_count
                                    + index->stats->query_filter_checked_nodes_count;
    index->stats->query_total_checked_ts_count = checked_ts_count
                                    + index->stats->query_filter_checked_ts_count;

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

    
    return bsf_result;
}


void exact_de_knn_search (ts_type *query_ts, ts_type *query_paa,
		       ts_type * query_ts_reordered,
		       int * query_order, unsigned int offset,
		       isax_index *index,ts_type minimum_distance,
		       ts_type epsilon, ts_type r_delta,
		       unsigned int k, unsigned int q_id, char * qfilename) 
{

  unsigned int curr_size = 0;

  ts_type bsf = FLT_MAX;
  
  ts_type kth_bsf = FLT_MAX;
  struct query_result temp;

    //the next NN found by incremental search
    unsigned int found_knn = 0;


    //queue containing kNN results
    struct query_result * knn_results = calloc(k,sizeof(struct query_result));
    for (int idx = 0; idx < k; ++idx)
    {
      knn_results[idx].node = NULL;
      knn_results[idx].distance = FLT_MAX;      
    }
    
    //file_position_type *bsf_position;
    //*bsf_position = -1;
    approximate_knn_search(query_ts, query_paa, query_ts_reordered,
			   query_order, offset, bsf, index,
			   knn_results,k, &curr_size);

    struct query_result approximate_result = knn_results[0];

    //query_result approximate_result = approximate_search(ts, paa, index);

    //query_result bsf_result = approximate_result;    

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
             //return approximate_result;
	     update_query_stats(index,q_id, found_knn, approximate_result);
	     get_query_stats(index, found_knn);
	     print_query_stats(index, q_id, found_knn,qfilename);	     	     
          }

          RESET_QUERY_COUNTERS()
          RESET_PARTIAL_COUNTERS()
          COUNT_PARTIAL_TIME_START
    
	  struct query_result  bsf_result = approximate_result;

	  pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
	
    
    //    printf("%s @ %lf\n", approximate_result.node->filename, approximate_result.distance);
    
    
	  //query_result *do_not_remove = &approximate_result;
    
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
        if(current_root_node != approximate_result.node)
	{
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
	}
        current_root_node = current_root_node->next;
    }
    
    query_result * n;
    //bsf_result =  approximate_result;
    
    while ((n = pqueue_pop(pq)))
    {
      // If it is a leaf check its real distance.
      temp = knn_results[k-1];
      kth_bsf =  temp.distance;
      if (n->distance > kth_bsf/(1 + epsilon)) {
	  //pqueue_insert(pq, n);
	break;
      }
      
      if (n->node->is_leaf)
       {
	      
	file_position_type raw_file_position = -1;
	//float distance = calculate_node_distance(index, n->node, ts, 
	//                                         &raw_file_position);
	 calculate_node_knn_distance(index, n->node, query_ts_reordered,
				     query_order,offset,
				     kth_bsf,
				     k,knn_results,&curr_size);
	 
	 //if (r_delta != FLT_MAX && knn_results[k-1].distance  <= r_delta * (1 + epsilon))
	 // break;
	 
	 
      }
      // If it is an intermediate node calculate mindist for children
      // and push them in the queue
      else {

	temp = knn_results[k-1];
	kth_bsf =  temp.distance;
	
	ts_type child_distance;
	  
	if (n->node->left_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->left_child->isax_values, 
						 n->node->left_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);	    
       
	  if ( child_distance < kth_bsf/(1 + epsilon)
	    &&
	      (n->node->left_child != approximate_result.node)) //add epsilon)
	    {

	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->left_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}
	if (n->node->right_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->right_child->isax_values, 
						 n->node->right_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);

	  if ( child_distance < kth_bsf/(1 + epsilon)
	    &&
	      (n->node->right_child != approximate_result.node)) //add epsilon)
	    {
	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->right_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}

      }
            
      // Free the node currently popped.
      //if(n != do_not_remove)
	free(n);
    }
    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
      {
        //if(n != do_not_remove)
	  free(n);
      }
    // Free the priority queue.
    
    pqueue_free(pq);

    //report the elements that were not reported already
    for (unsigned int pos = found_knn; pos < k; ++pos)
      {
	bsf_result = knn_results[pos];
	found_knn = pos+1;
        COUNT_PARTIAL_TIME_END
        update_query_stats(index,q_id, found_knn, bsf_result);
	get_query_stats(index, found_knn);
	print_query_stats(index, q_id, found_knn,qfilename);	
	//report all results for found_knn - last_found_knn or print their results
	RESET_QUERY_COUNTERS()
        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START		
      }
    
    free(knn_results);	
}

void exact_ng_knn_search (ts_type *query_ts, ts_type *query_paa,
		       ts_type * query_ts_reordered,
		       int * query_order, unsigned int offset,
		       isax_index *index,ts_type minimum_distance,
		       unsigned int k, unsigned int q_id, char * qfilename, unsigned int nprobes) 
{

  unsigned int curr_size = 0;
  unsigned int cur_probes = 0;  

  ts_type bsf = FLT_MAX;
  
  ts_type kth_bsf = FLT_MAX;
  struct query_result temp;

    //the next NN found by incremental search
    unsigned int found_knn = 0;


    //queue containing kNN results
    struct query_result * knn_results = calloc(k,sizeof(struct query_result));
    for (int idx = 0; idx < k; ++idx)
    {
      knn_results[idx].node = NULL;
      knn_results[idx].distance = FLT_MAX;      
    }
    
    //file_position_type *bsf_position;
    //*bsf_position = -1;
    approximate_knn_search(query_ts, query_paa, query_ts_reordered,
			   query_order, offset, bsf, index,
			   knn_results,k, &curr_size);

    ++cur_probes;    
    struct query_result approximate_result = knn_results[0];

    //query_result approximate_result = approximate_search(ts, paa, index);

    //query_result bsf_result = approximate_result;    

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
             //return approximate_result;
	     update_query_stats(index,q_id, found_knn, approximate_result);
	     get_query_stats(index, found_knn);
	     print_query_stats(index, q_id, found_knn,qfilename);	     	     
          }

          RESET_QUERY_COUNTERS()
          RESET_PARTIAL_COUNTERS()
          COUNT_PARTIAL_TIME_START
    
	  struct query_result  bsf_result = approximate_result;

	  pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
	
    
    //    printf("%s @ %lf\n", approximate_result.node->filename, approximate_result.distance);
    
    
	  //query_result *do_not_remove = &approximate_result;
    
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
        if(current_root_node != approximate_result.node)
	{
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
	}
        current_root_node = current_root_node->next;
    }
    
    query_result * n;
    //bsf_result =  approximate_result;
    
    while ((n = pqueue_pop(pq)) && cur_probes < nprobes)
    {
      // If it is a leaf check its real distance.
      if (n->distance > bsf_result.distance) {
	  //pqueue_insert(pq, n);
	break;
      }
      
      if (n->node->is_leaf)
       {
	      
	file_position_type raw_file_position = -1;
	//float distance = calculate_node_distance(index, n->node, ts, 
	//                                         &raw_file_position);
	 calculate_node_knn_distance(index, n->node, query_ts_reordered,
				     query_order,offset,
				     bsf_result.distance,
				     k,knn_results,&curr_size);
	 
	 
	  ++cur_probes;	 
	 
      }
      // If it is an intermediate node calculate mindist for children
      // and push them in the queue
      else {

	temp = knn_results[k-1];
	kth_bsf =  temp.distance;
	
	ts_type child_distance;
	  
	if (n->node->left_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->left_child->isax_values, 
						 n->node->left_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);	    
       
	  if ( child_distance <= kth_bsf
	    &&
	      (n->node->left_child != approximate_result.node)) //add epsilon)
	    {

	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->left_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}
	if (n->node->right_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->right_child->isax_values, 
						 n->node->right_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);

	  if ( child_distance <= kth_bsf
	    &&
	      (n->node->right_child != approximate_result.node)) //add epsilon)
	    {
	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->right_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}

      }
            
      // Free the node currently popped.
      //if(n != do_not_remove)
	free(n);
    }
    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
      {
        //if(n != do_not_remove)
	  free(n);
      }
    // Free the priority queue.
    
    pqueue_free(pq);

    //report the elements that were not reported already
    for (unsigned int pos = found_knn; pos < k; ++pos)
      {
	bsf_result = knn_results[pos];
	found_knn = pos+1;
        COUNT_PARTIAL_TIME_END
        update_query_stats(index,q_id, found_knn, bsf_result);
	get_query_stats(index, found_knn);
	print_query_stats(index, q_id, found_knn,qfilename);	
	//report all results for found_knn - last_found_knn or print their results
	RESET_QUERY_COUNTERS()
        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START		
      }
    
    free(knn_results);	
}

void exact_incr_knn_search (ts_type *query_ts, ts_type *query_paa,
			       ts_type * query_ts_reordered,
			       int * query_order, unsigned int offset,
			       isax_index *index,ts_type minimum_distance,
			       ts_type epsilon, ts_type r_delta,
			    unsigned int k, unsigned int q_id, char * qfilename, unsigned int nprobes) 
{

  unsigned int curr_size = 0;
  unsigned int cur_probes = 0;
  
  ts_type bsf = FLT_MAX;
  
  ts_type kth_bsf = FLT_MAX;
  struct query_result temp;

    //the next NN found by incremental search
    unsigned int found_knn = 0;


    //queue containing kNN results
    struct query_result * knn_results = calloc(k,sizeof(struct query_result));
    for (int idx = 0; idx < k; ++idx)
    {
      knn_results[idx].node = NULL;
      knn_results[idx].distance = FLT_MAX;      
    }
    
    //file_position_type *bsf_position;
    //*bsf_position = -1;
    approximate_knn_search(query_ts, query_paa, query_ts_reordered,
			   query_order, offset, bsf, index,
			   knn_results,k, &curr_size);

    ++cur_probes;
    
    struct query_result approximate_result = knn_results[0];

    //query_result approximate_result = approximate_search(ts, paa, index);

    //query_result bsf_result = approximate_result;    

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
             //return approximate_result;
	     update_query_stats(index,q_id, found_knn, approximate_result);
	     get_query_stats(index, found_knn);
	     print_query_stats(index, q_id, found_knn,qfilename);	     	     
          }

          RESET_QUERY_COUNTERS()
          RESET_PARTIAL_COUNTERS()
          COUNT_PARTIAL_TIME_START
    
	  struct query_result  bsf_result = approximate_result;

	  pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
	
    
    //    printf("%s @ %lf\n", approximate_result.node->filename, approximate_result.distance);
    
    
	  //query_result *do_not_remove = &approximate_result;
    
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
        if(current_root_node != approximate_result.node)
	{
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
	}
        current_root_node = current_root_node->next;
    }
    
    query_result * n;
    //bsf_result =  approximate_result;
    while ((n = pqueue_pop(pq)) && cur_probes < nprobes)    
    {

      for (unsigned int pos = found_knn; pos < k; ++pos)
	{
	  bsf_result = knn_results[pos];
	
	  if (n->distance > bsf_result.distance/(1+epsilon)) //add epsilon+1
	    {
	      found_knn = pos+1;
	      COUNT_PARTIAL_TIME_END

	      update_query_stats(index,q_id, found_knn, bsf_result);
	      get_query_stats(index, found_knn);
	      print_query_stats(index, q_id, found_knn,qfilename);

	      //reset the bsf for the next NN
	      if (found_knn < k){	    
		bsf_result = knn_results[found_knn];
	      }
	  
	      RESET_QUERY_COUNTERS()
	      RESET_PARTIAL_COUNTERS()
	      COUNT_PARTIAL_TIME_START
	    }
	}

      if (found_knn == k)
	{
	  //printf("found all kNN\n");
	  break;
	}
      
      //if (n->distance > bsf_result.distance/(1 + epsilon)) {
	//pqueue_insert(pq, n);
	//break;
      //}
      
      // If it is a leaf check its real distance.
      if (n->node->is_leaf)
       {
	      
	file_position_type raw_file_position = -1;
	//float distance = calculate_node_distance(index, n->node, ts, 
	//                                         &raw_file_position);
	 calculate_node_knn_distance(index, n->node, query_ts_reordered,
				     query_order,offset,
				     bsf_result.distance,
				     k,knn_results,&curr_size);
	  if (r_delta != FLT_MAX && knn_results[k-1].distance  <= r_delta * (1 + epsilon))
	    break;
	  ++cur_probes;	 
      }
      // If it is an intermediate node calculate mindist for children
      // and push them in the queue
      else {

	temp = knn_results[k-1];
	kth_bsf =  temp.distance;
	
	ts_type child_distance;
	  
	if (n->node->left_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->left_child->isax_values, 
						 n->node->left_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);	    
       
	  if ( child_distance <= kth_bsf/(1 + epsilon)
	    &&
	      (n->node->left_child != approximate_result.node)) //add epsilon)
	    {

	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->left_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}
	if (n->node->right_child->isax_cardinalities != NULL) {
	  child_distance =  minidist_paa_to_isax(query_paa, n->node->right_child->isax_values, 
						 n->node->right_child->isax_cardinalities,
						 index->settings->sax_bit_cardinality,  
						 index->settings->sax_alphabet_cardinality, 
						 index->settings->paa_segments, 
						 MINVAL, MAXVAL,
						 index->settings->mindist_sqrt);

	  if ( child_distance <= kth_bsf/(1 + epsilon)
	    &&
	      (n->node->right_child != approximate_result.node)) //add epsilon)
	    {
	      query_result * mindist_result = malloc(sizeof(query_result));
	      mindist_result->distance =  child_distance;
	      mindist_result->node = n->node->right_child;
	      pqueue_insert(pq, mindist_result);
	    }
	}

      }
            
      // Free the node currently popped.
      //if(n != do_not_remove)
	free(n);
    }

    //report the elements that were not reported already
    for (unsigned int pos = found_knn; pos < k; ++pos)
      {
	bsf_result = knn_results[pos];
	found_knn = pos+1;
        COUNT_PARTIAL_TIME_END
        update_query_stats(index,q_id, found_knn, bsf_result);
	get_query_stats(index, found_knn);
	print_query_stats(index, q_id, found_knn,qfilename);	
	//report all results for found_knn - last_found_knn or print their results
	RESET_QUERY_COUNTERS()
        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START	
      }
    
    
    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
      {
        //if(n != do_not_remove)
	  free(n);
      }
    // Free the priority queue.
    
    pqueue_free(pq);
    free(knn_results);	
}


void isax2plus_calc_tlb (ts_type *query_ts, ts_type * query_paa, isax_index *index, isax_node * curr_node)
{
  ts_type curr_node_lb_dist = 0;
  ts_type curr_data_lb_dist = 0;  
  //ts_type curr_exact_dist = 0;


  // void isax2plus_calc_tlb(isax_index *index, isax_node *node, int *already_finalized) 
    if (curr_node == NULL)
    {
        isax_node *curr_node = index->first_node;
        while (curr_node != NULL) {
	  isax2plus_calc_tlb(query_ts, query_paa, index, curr_node);
          curr_node = curr_node->next;
        }
    }
    else
    {
        if (!curr_node->is_leaf)
	{
	  isax2plus_calc_tlb (query_ts, query_paa, index, curr_node->left_child);
	  isax2plus_calc_tlb (query_ts, query_paa, index, curr_node->right_child);	  
        }
	//This is a leaf, calculate the tlb = lb_distance/exact_distance
	else
	{
          if (curr_node->leaf_size != 0 )
	    {
	  curr_node_lb_dist =  minidist_paa_to_isax(query_paa, curr_node->isax_values, 
					       curr_node->isax_cardinalities,
					       index->settings->sax_bit_cardinality,  
					       index->settings->sax_alphabet_cardinality, 
					       index->settings->paa_segments, 
					       MINVAL, MAXVAL,
					       index->settings->mindist_sqrt);


	    
	  if (curr_node->has_full_data_file)
	  {
	    char * full_fname = malloc(sizeof(char) * (strlen(curr_node->filename) + 4));
	    strcpy(full_fname, curr_node->filename);
	    strcat(full_fname, ".ts");

	    FILE * full_file = fopen(full_fname, "r");

	    if(full_file != NULL) {
	      fseek(full_file, 0L, SEEK_END);
	      size_t file_size = ftell(full_file);
	      fseek(full_file, 0L, SEEK_SET);
	      int file_records = (int) file_size / (int)(index->settings->ts_byte_size + sizeof(file_position_type));
	      //ts_type *ts = malloc(index->settings->ts_byte_size);
	      //file_position_type *position =  malloc(sizeof(file_position_type));


	      
	      sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
	      ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
      	      ++leaf_nodes_count;
	      while (file_records > 0)
	      {
		  file_records--;
		  fread(ts, sizeof(ts_type), index->settings->timeseries_size, full_file);
		  //fread(position, sizeof(file_position_type), 1, full_file);
		  //get the sax representation with the full alphabet size
		  float curr_exact_dist = ts_euclidean_distance(query_ts, ts, index->settings->timeseries_size);
		  if (isnan(curr_exact_dist) || isinf(curr_exact_dist) || (curr_exact_dist == 0))
		  {
		    continue;
		  }
        	  ++total_ts_count;
		  
		  sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment, 
			      index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
			      index->settings->sax_bit_cardinality);
		  
		  curr_data_lb_dist =  minidist_paa_to_isax(query_paa, sax, 
							index->settings->max_sax_cardinalities,
							index->settings->sax_bit_cardinality,  
							index->settings->sax_alphabet_cardinality, 
							index->settings->paa_segments, 
							MINVAL, MAXVAL,
							index->settings->mindist_sqrt);
		  
                  /*
	          printf("Leaf node: %lu exact_distance = %g node_lb_distance = %g, data_lb_distance = %g\n",
			 leaf_nodes_count,
			 curr_exact_dist,
			 curr_node_lb_dist,
			 curr_data_lb_dist);
		  */

		  total_node_tlb += (ts_type)sqrt((double)curr_node_lb_dist / (double)curr_exact_dist);
		  total_data_tlb += (ts_type)sqrt((double)curr_data_lb_dist / (double)curr_exact_dist);

		  /*
	          printf("Leaf node: %lu exact_distance = %g node_lb_distance = %g\n",
			 leaf_nodes_count,
			 curr_exact_dist,
			 curr_node_lb_dist);
		  */

	      }
	      
	      free(sax);
	      free(ts);
	      //free(position);
	    }
	    fclose(full_file);
	    free(full_fname);
	    }
	  }
	  else
	  {
	    //printf("**** NO FULL DATA FILE...\n");
	    //exit(FAILURE);
	  }
        }
    }
}


void update_query_stats(isax_index * index,
			unsigned int query_id,
			unsigned int found_knn,
			struct query_result bsf_result)
{
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

  index->stats->query_refine_loaded_nodes_count = loaded_nodes_count;
  index->stats->query_refine_loaded_ts_count = loaded_ts_count;
  index->stats->query_refine_checked_nodes_count = checked_nodes_count;
  index->stats->query_refine_checked_ts_count = checked_ts_count;
  
  if (found_knn == 1)
    {
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

      index->stats->query_total_loaded_nodes_count = loaded_nodes_count 
	+ index->stats->query_filter_loaded_nodes_count;
      index->stats->query_total_loaded_ts_count = loaded_ts_count
	+ index->stats->query_filter_loaded_ts_count;
      index->stats->query_total_checked_nodes_count = checked_nodes_count
	+ index->stats->query_filter_checked_nodes_count;
      index->stats->query_total_checked_ts_count = checked_ts_count
	+ index->stats->query_filter_checked_ts_count;      
    }
  else
    {
      index->stats->query_total_time  = partial_time;
      index->stats->query_total_input_time  = partial_input_time;
      index->stats->query_total_output_time  = partial_output_time;
      index->stats->query_total_load_node_time  = partial_load_node_time;

      index->stats->query_total_cpu_time  =  index->stats->query_total_time
	-  index->stats->query_total_input_time
	-  index->stats->query_total_output_time;
    
      index->stats->query_total_seq_input_count   = partial_seq_input_count;
      index->stats->query_total_seq_output_count   = partial_seq_output_count;
      index->stats->query_total_rand_input_count   = partial_rand_input_count;
      index->stats->query_total_rand_output_count   = partial_rand_output_count;

      index->stats->query_total_loaded_nodes_count = loaded_nodes_count; 
      index->stats->query_total_loaded_ts_count = loaded_ts_count;
      index->stats->query_total_checked_nodes_count = checked_nodes_count;
      index->stats->query_total_checked_ts_count = checked_ts_count;
      
    }
  
  index->stats->query_exact_distance = sqrtf(bsf_result.distance);
  index->stats->query_exact_node_filename = bsf_result.node->filename;
  index->stats->query_exact_node_size = bsf_result.node->leaf_size;;
  index->stats->query_exact_node_level = bsf_result.node->level;


}


void get_query_stats(isax_index * index, unsigned int found_knn)
{

  if (total_ts_count != 0)
    {
        index->stats->query_pruning_ratio = 1.0 - ((double)index->stats->query_total_checked_ts_count/
						 index->stats->total_ts_count);
    }
    
  if (found_knn == 1)
    {
      if (index->stats->query_exact_distance != 0)
	{
	  index->stats->query_eff_epsilon =   (index->stats->query_approx_distance
					       -index->stats->query_exact_distance
					       )/ index->stats->query_exact_distance;	  
	}

    }

}


void print_query_stats(isax_index * index, unsigned int query_num, unsigned int found_knn, char * queries)
{

  if (found_knn == 1)
    {
        printf("Query_filter_input_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_filter_input_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_filter_output_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_filter_output_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_filter_load_node_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_filter_cpu_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );
	
        printf("Query_filter_total_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_filter_total_time/1000000,	       
	       queries,	       
	       query_num,
	       found_knn
	     );

        printf("Query_filter_seq_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_seq_input_count,
	       queries,
	       query_num,
	       found_knn
	       );
	
        printf("Query_filter_seq_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_seq_output_count,
	       queries,
	       query_num,
	       found_knn
	       );
	
        printf("Query_filter_rand_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_rand_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_filter_rand_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_rand_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
        printf("Query_filter_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_filter_checked_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_filter_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_checked_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_filter_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_filter_loaded_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_filter_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_filter_loaded_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
        printf("Query_approx_distance\t%f\t%s\t%u\t%u\n",
	       index->stats->query_approx_distance,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_approx_node_filename\t%s\t%s\t%u\t%u\n",
	       index->stats->query_approx_node_filename,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_approx_node_size\t%u\t%s\t%u\t%u\n",
	       index->stats->query_approx_node_size,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_approx_node_level\t%u\t%s\t%u\t%u\n",
	       index->stats->query_approx_node_level,
	       queries,
	       query_num,
	       found_knn
	       );

        printf("Query_eff_epsilon\t%f\t%s\t%u\t%u\n",
	       index->stats->query_eff_epsilon,	       
	       queries,
	       query_num,
	       found_knn	       
	       );		

	
    }

        printf("Query_refine_input_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_refine_input_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_refine_output_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_refine_output_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_refine_load_node_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_refine_cpu_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );
	
        printf("Query_refine_total_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_refine_total_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_refine_seq_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_seq_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_refine_seq_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_seq_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_refine_rand_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_rand_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_refine_rand_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_rand_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );

	
        printf("Query_refine_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_refine_checked_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_refine_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_checked_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_refine_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_refine_loaded_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_refine_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_refine_loaded_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );

        printf("Query_total_input_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_total_input_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_total_output_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_total_load_node_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\t%u\n",
	       index->stats->query_total_time/1000000,	       
	       queries,
	       query_num,
	       found_knn	       
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_seq_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_seq_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_rand_input_count,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_rand_output_count,
	       queries,
	       query_num,
	       found_knn	       
	       );


        printf("Query_total_checked_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_total_checked_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_total_checked_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_checked_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_total_loaded_nodes_count\t%u\t%s\t%u\t%u\n",
	       index->stats->query_total_loaded_nodes_count,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_total_loaded_ts_count\t%llu\t%s\t%u\t%u\n",
	       index->stats->query_total_loaded_ts_count,
	       queries,
	       query_num,
	       found_knn	       
	       );

        printf("Query_exact_distance\t%f\t%s\t%u\t%u\n",
	       index->stats->query_exact_distance,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_exact_node_filename\t%s\t%s\t%u\t%u\n",
	       index->stats->query_exact_node_filename,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
        printf("Query_exact_node_size\t%u\t%s\t%u\t%u\n",
	       index->stats->query_exact_node_size,
	       queries,
	       query_num,
	       found_knn	       
	       );	

        printf("Query_exact_node_level\t%u\t%s\t%u\t%u\n",
	       index->stats->query_exact_node_level,
	       queries,
	       query_num,
	       found_knn	       
	       );
	
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%u\t%u\n",
	       index->stats->query_pruning_ratio,	       
	       queries,
	       query_num,
	       found_knn	       
	       );

}
