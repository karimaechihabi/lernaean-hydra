//
//  sfa_query_engine.c
//  sfa C version
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

#include "../include/sfa_query_engine.h"
#include "../include/sfa_file_buffer.h"
#include "../include/sfa_file_buffer_manager.h"
#include "../include/sfa_trie.h"
#include "../include/sfa_node.h"

#include "../include/pqueue.h"
#ifdef VALUES
#include <values.h>
#endif

//returns child in index position
//if empty, returns any non empty sibling

struct sfa_node * get_non_empty_child(struct sfa_trie * trie, struct sfa_node * parent, unsigned int index)
{
  int i=0, j=0;
  unsigned int num_siblings = 0;
  struct sfa_node **siblings;
  unsigned int num_children = trie->settings->num_symbols;
  
  siblings  = calloc(num_children, sizeof(struct sfa_node*));
  
  struct sfa_node * child_node;
  
  child_node = parent->children;
  
  while (child_node != NULL)
    {
      if (child_node->node_size != 0)
	{
	  siblings[num_siblings] = child_node;
	}
      else
	{
	  siblings[num_siblings] = NULL;
	}
      num_siblings++;
      child_node = child_node->next;	     	     
    }
  
  child_node = siblings[index];
  
  //The node with the corresponding key is empty
  //so choose any other node
  if (child_node == NULL)
    {
      //choose arbitrary node
      for (j = 0; j< num_siblings; ++j )
	{
	  if (siblings[j] != NULL)
	    {
	      child_node = siblings[j];
	      break;
	    }
	}
    }

  free(siblings);

  return child_node;
  
}
  
struct query_result approximate_search (ts_type *query_ts, unsigned char * query_sfa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, ts_type bsf,struct sfa_trie *trie)
{

    struct query_result result;

    struct sfa_node * parent = trie->first_node;
    struct sfa_node * child_node = trie->first_node;
   
     //get the leaf node
    unsigned char key;
    unsigned int index = 0;
    int i=0;
    int j=0;
     
    if (parent != NULL)
    {
       for (i = 0; i < trie->settings->fft_size; ++i)
	{	   
	   key = query_sfa[i];
	   //index = key - 'A';
	   index = (unsigned int )key;
	   
	   parent = child_node;
	   child_node = get_non_empty_child(trie,parent,index);
	              	   
	   if(child_node->is_leaf)	     
	   {
	     break;
	   }
	}
        if(child_node->is_leaf)
	  {
	    result.distance = calculate_node_distance(trie, child_node, query_ts_reordered, query_order,offset,bsf);
	    result.node = child_node;
	  }
	else
	  {
	    printf("Error: Could not set approximate distance \n");        
	    result.node = NULL;
	    result.distance = MAXFLOAT;
	  }
    }
    else
    {
      printf("Error: trie is empty \n");        
      result.node = NULL;
      result.distance = MAXFLOAT;
    }

    return result;
}

struct query_result exact_search (ts_type *query_ts, ts_type *query_fft, unsigned char * query_sfa, ts_type * query_ts_reordered, int * query_order, unsigned int offset, struct sfa_trie *trie,ts_type minimum_distance)
{
      
    ts_type bsf = FLT_MAX;
      
    struct query_result approximate_result = approximate_search(query_ts, query_sfa,query_ts_reordered, query_order, offset, bsf, trie);


    COUNT_PARTIAL_TIME_END                 
    trie->stats->query_filter_total_time  = partial_time;	
    
    trie->stats->query_filter_input_time  = partial_input_time;
    trie->stats->query_filter_output_time = partial_output_time;
    trie->stats->query_filter_load_node_time = partial_load_node_time;
    trie->stats->query_filter_cpu_time    = partial_time-partial_input_time-partial_output_time;
    trie->stats->query_filter_seq_input_count   = partial_seq_input_count;
    trie->stats->query_filter_seq_output_count  = partial_seq_output_count;
    trie->stats->query_filter_rand_input_count  = partial_rand_input_count;
    trie->stats->query_filter_rand_output_count = partial_rand_output_count;
    
    trie->stats->query_filter_loaded_nodes_count = loaded_nodes_count;
    trie->stats->query_filter_loaded_ts_count = loaded_ts_count;
    trie->stats->query_filter_checked_nodes_count = checked_nodes_count;
    trie->stats->query_filter_checked_ts_count = checked_ts_count;

    
    trie->stats->queries_filter_total_time  +=  trie->stats->query_filter_total_time;
    
    trie->stats->queries_filter_input_time  +=  trie->stats->query_filter_input_time;
    trie->stats->queries_filter_output_time +=  trie->stats->query_filter_output_time;
    trie->stats->queries_filter_load_node_time += trie->stats->query_filter_load_node_time;
    trie->stats->queries_filter_cpu_time    += trie->stats->query_filter_cpu_time;

    trie->stats->queries_filter_seq_input_count   += trie->stats->query_filter_seq_input_count;
    trie->stats->queries_filter_seq_output_count  += trie->stats->query_filter_seq_output_count;
    trie->stats->queries_filter_rand_input_count  += trie->stats->query_filter_rand_input_count;
    trie->stats->queries_filter_rand_output_count += trie->stats->query_filter_rand_output_count;
    
    
    if(approximate_result.node != NULL) {  
             trie->stats->query_approx_distance = sqrtf(approximate_result.distance);
             trie->stats->query_approx_node_filename = approximate_result.node->filename;
             trie->stats->query_approx_node_size = approximate_result.node->node_size;;
             trie->stats->query_approx_node_level = approximate_result.node->level;
    }
 
    // Early termination...
    if (approximate_result.distance == 0) {
             trie->stats->query_exact_distance = sqrtf(approximate_result.distance);
             trie->stats->query_exact_node_filename = approximate_result.node->filename;
             trie->stats->query_exact_node_size = approximate_result.node->node_size;;
             trie->stats->query_exact_node_level = approximate_result.node->level; 
             return approximate_result;
    }

    RESET_QUERY_COUNTERS()
    RESET_PARTIAL_COUNTERS()
    COUNT_PARTIAL_TIME_START
 
    struct query_result bsf_result = approximate_result;
    
   
    pqueue_t *pq = pqueue_init(trie->first_node->node_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
	
    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
		    pqueue_insert(pq, &approximate_result);
    }
    
    struct query_result *do_not_remove = &approximate_result;

    
    //Add the root to the priority queue

    struct query_result * root_pq_item = malloc(sizeof(struct query_result));
    root_pq_item->node = trie->first_node;
    
    //Using SFA Lower Bounding
    if (trie->settings->lb_dist == 0)
    {
      root_pq_item->distance = calculate_sfa_node_min_distance (trie,
								trie->first_node,
								query_ts,
								query_fft,
								query_sfa,
								bsf_result.distance,
								true);
    }
    //Using DFT Lower Bounding
    else if (trie->settings->lb_dist == 1)
    {
      root_pq_item->distance = calculate_node_min_distance (trie, trie->first_node, query_ts, query_fft);
    }
    else
    {
      fprintf(stderr, "Wrong lower bounding distance \n");
      exit(-1);      
    }  
    		

    //ts_type dist_test = FLT_MAX;
    
    //initialize the lb distance to be the distance of the query to the root.

    pqueue_insert(pq, root_pq_item);    

    struct query_result * n;
    
    struct sfa_node * child_node;
    ts_type child_distance;
    ts_type distance;
    
    while ((n = pqueue_pop(pq)))
    {
            
        if (n->distance > bsf_result.distance)
	{
          break;
        }
      
            // If it is a leaf check its real distance.
        if (n->node->is_leaf)
        {
	  if (n->node->node_size != 0)
	    {
	      distance = calculate_node_distance(trie, n->node, query_ts_reordered,
						 query_order, offset, bsf_result.distance);
                
                if (distance < bsf_result.distance)
                {
                    bsf_result.distance = distance;
                    bsf_result.node = n->node;
                }
		
	    }
        }
            // If it is an intermediate node calculate mindist for children
            // and push them in the queue
        else 
        {
          child_node = n->node->children;
	  printf("node_size %lu \n", n->node->node_size);
	  while(child_node != NULL)
	  {
	    if (child_node->node_size != 0)
	    {
	      if (child_node->is_leaf)
	      {
		distance = calculate_node_distance(trie, child_node,query_ts_reordered,
						   query_order, offset, bsf_result.distance);		  
		  if (distance < bsf_result.distance)
		    {
		      bsf_result.distance = distance;
		      bsf_result.node = child_node;
		    }
	      }
	      else
	      {
		//Using SFA Lower Bounding
		if (trie->settings->lb_dist == 0)
		  {
		    child_distance = calculate_sfa_node_min_distance (trie,
								  child_node,
								  query_ts,
								  query_fft,
								  query_sfa,
								  bsf_result.distance,
								  true);
		  }
		//Using DFT Lower Bounding
		else if (trie->settings->lb_dist == 1)
		  {
		    child_distance = calculate_node_min_distance (trie, child_node, query_ts, query_fft);
		  }
		else
		  {
		    fprintf(stderr, "Wrong lower bounding distance \n");
		    exit(-1);      
		  }  
		
			
		if (child_distance < bsf_result.distance )
		{
		    struct query_result * mindist_result = malloc(sizeof(struct query_result));
		    mindist_result->node = child_node;
		    mindist_result->distance =  child_distance;
		    pqueue_insert(pq, mindist_result);
		}			
	      }
	    }
	    child_node = child_node->next;	      	      	    
	  }
        }
            // Free the node currently popped.
	 if(n != do_not_remove)
            	free(n);
    }
    
    // Free the nodes that were not popped.
    while ((n = pqueue_pop(pq)))
    {
      if(n != do_not_remove)
        	free(n);
    }
    // Free the priority queue.
    
    pqueue_free(pq);


    COUNT_PARTIAL_TIME_END 


    trie->stats->query_refine_total_time  = partial_time;	
        
    trie->stats->query_refine_input_time  = partial_input_time;
    trie->stats->query_refine_output_time = partial_output_time;
    trie->stats->query_refine_load_node_time = partial_load_node_time;    
    trie->stats->query_refine_cpu_time    = partial_time
                                           - partial_input_time
                                           - partial_output_time;
    trie->stats->query_refine_seq_input_count   = partial_seq_input_count;
    trie->stats->query_refine_seq_output_count  = partial_seq_output_count;
    trie->stats->query_refine_rand_input_count  = partial_rand_input_count;
    trie->stats->query_refine_rand_output_count = partial_rand_output_count;

    trie->stats->query_total_time  = partial_time
                                    +  trie->stats->query_filter_total_time;
    trie->stats->query_total_input_time  = partial_input_time
                                    +  trie->stats->query_filter_input_time;
    trie->stats->query_total_output_time  = partial_output_time
                                    +  trie->stats->query_filter_output_time;
    trie->stats->query_total_load_node_time  = partial_load_node_time
                                    +  trie->stats->query_filter_load_node_time;
    trie->stats->query_total_cpu_time  =  trie->stats->query_total_time
                                        -  trie->stats->query_total_input_time
                                        -  trie->stats->query_total_output_time;
    
    trie->stats->query_total_seq_input_count   = partial_seq_input_count
                                   + trie->stats->query_filter_seq_input_count;
    trie->stats->query_total_seq_output_count   = partial_seq_output_count
                                   + trie->stats->query_filter_seq_output_count;
    trie->stats->query_total_rand_input_count   = partial_rand_input_count
                                   + trie->stats->query_filter_rand_input_count;
    trie->stats->query_total_rand_output_count   = partial_rand_output_count
                                   + trie->stats->query_filter_rand_output_count;
    
    trie->stats->query_exact_distance = sqrtf(bsf_result.distance);
    trie->stats->query_exact_node_filename = bsf_result.node->filename;
    trie->stats->query_exact_node_size = bsf_result.node->node_size;;
    trie->stats->query_exact_node_level = bsf_result.node->level;

    trie->stats->query_refine_loaded_nodes_count = loaded_nodes_count;
    trie->stats->query_refine_loaded_ts_count = loaded_ts_count;
    trie->stats->query_refine_checked_nodes_count = checked_nodes_count;
    trie->stats->query_refine_checked_ts_count = checked_ts_count;
  
    trie->stats->query_total_loaded_nodes_count = loaded_nodes_count 
                                    + trie->stats->query_filter_loaded_nodes_count;
    trie->stats->query_total_loaded_ts_count = loaded_ts_count
                                    + trie->stats->query_filter_loaded_ts_count;
    trie->stats->query_total_checked_nodes_count = checked_nodes_count
                                    + trie->stats->query_filter_checked_nodes_count;
    trie->stats->query_total_checked_ts_count = checked_ts_count
                                    + trie->stats->query_filter_checked_ts_count;


    trie->stats->queries_refine_total_time  += trie->stats->query_refine_total_time;	
    
    trie->stats->queries_refine_input_time  += partial_input_time;
    trie->stats->queries_refine_output_time += partial_output_time;
    trie->stats->queries_refine_load_node_time += partial_load_node_time;        
    trie->stats->queries_refine_cpu_time    += partial_time
                                           - partial_input_time
                                           - partial_output_time;
    trie->stats->queries_refine_seq_input_count   += partial_seq_input_count;
    trie->stats->queries_refine_seq_output_count  += partial_seq_output_count;
    trie->stats->queries_refine_rand_input_count  += partial_rand_input_count;
    trie->stats->queries_refine_rand_output_count += partial_rand_output_count;


    trie->stats->queries_total_input_time  = trie->stats->queries_refine_input_time
                                              +trie->stats->queries_filter_input_time;	
    trie->stats->queries_total_output_time  = trie->stats->queries_refine_output_time
                                               +trie->stats->queries_filter_output_time;	
    trie->stats->queries_total_load_node_time  = trie->stats->queries_refine_load_node_time
                                               +trie->stats->queries_filter_load_node_time;
    trie->stats->queries_total_cpu_time  = trie->stats->queries_refine_cpu_time
                                               +trie->stats->queries_filter_cpu_time;
    
    trie->stats->queries_total_time  = trie->stats->queries_refine_total_time
                                         +trie->stats->queries_filter_total_time;
    
    trie->stats->queries_total_seq_input_count   = trie->stats->queries_filter_seq_input_count
                                                    + trie->stats->queries_refine_seq_input_count;
    trie->stats->queries_total_seq_output_count  = trie->stats->queries_filter_seq_output_count
                                                    + trie->stats->queries_refine_seq_output_count;
    trie->stats->queries_total_rand_input_count  = trie->stats->queries_filter_rand_input_count
                                                    + trie->stats->queries_refine_rand_input_count;
    trie->stats->queries_total_rand_output_count = trie->stats->queries_filter_rand_output_count
                                                    + trie->stats->queries_refine_rand_output_count;

       
    //keep a running sum then divide by the total number of queries
    trie->stats->queries_avg_checked_nodes_count += trie->stats->query_total_checked_nodes_count;
    trie->stats->queries_avg_checked_ts_count += trie->stats->query_total_checked_ts_count;
    trie->stats->queries_avg_loaded_nodes_count += trie->stats->query_total_loaded_nodes_count;
    trie->stats->queries_avg_loaded_ts_count += trie->stats->query_total_loaded_ts_count; 

    
    //COUNT_TOTAL_TIME_START
    return bsf_result;
}

void sfa_calc_tlb_leaf (ts_type *query_ts, ts_type *query_fft,
		   unsigned char * query_sfa, struct sfa_trie *trie, struct sfa_node * curr_node)
{

  ts_type curr_node_sfa_lb_dist = 0;
  ts_type curr_node_dft_lb_dist = 0;  
  ts_type curr_exact_dist = 0;

  ts_type bsf = FLT_MAX;  
  //Using SFA Lower Bounding
  curr_node_sfa_lb_dist = calculate_sfa_node_min_distance (trie,
							   curr_node,
							   query_ts,
							   query_fft,
							   query_sfa,
							   bsf,
							   true);
		
  //Using DFT Lower Bounding
  curr_node_dft_lb_dist = calculate_node_min_distance (trie, curr_node, query_ts, query_fft);
  
  if (curr_node->file_buffer->buffered_list_size == 0) 
    {	
      curr_node->file_buffer->buffered_list = get_all_time_series_in_node(trie, curr_node);
      curr_node->file_buffer->buffered_list_size = curr_node->file_buffer->disk_count;

      if (curr_node->file_buffer->buffered_list == NULL)
	{
	  fprintf(stderr,
		  "Error in sfa_query_engine.c:  Could not retrieve all time series for node %s.\n",
		  curr_node->filename);
	}	
    }
  ++leaf_nodes_count;
    
  for (int idx = 0; idx < curr_node->file_buffer->buffered_list_size; ++idx)
    {  
      curr_exact_dist = ts_euclidean_distance(query_ts,
					      curr_node->file_buffer->buffered_list[idx].timeseries,
					      trie->settings->timeseries_size);
      //printf("Leaf node: %lu exact_distance = %g dft_lb_distance = %g sfa_lb_distance = %g\n",
      //	  leaf_nodes_count, curr_exact_dist, curr_node_dft_lb_dist, curr_node_sfa_lb_dist);
      if (isnan(curr_exact_dist) || isinf(curr_exact_dist) || (curr_exact_dist == 0))
      {
	  continue;
      }
      ++total_ts_count;

      total_sfa_tlb += (ts_type)sqrt((double)curr_node_sfa_lb_dist / (double)curr_exact_dist);
      total_dft_tlb += (ts_type)sqrt((double)curr_node_dft_lb_dist / (double)curr_exact_dist);      
      
    }	   

}

  void sfa_calc_tlb (ts_type *query_ts, ts_type *query_fft,
		   unsigned char * query_sfa, struct sfa_trie *sfa, struct sfa_node * curr_node)
{
    
  //This is an internal node, traverse left and right children
  if (curr_node == NULL)
  {
    return;
  }

  if (!curr_node->is_leaf)
  {
    struct sfa_node * child_node;
    child_node = curr_node->children;
	  
    while(child_node != NULL)
    {
	if (child_node->node_size != 0)
	{
	    if (child_node->is_leaf)
	    {
 	       sfa_calc_tlb_leaf (query_ts, query_fft, query_sfa,sfa,child_node);
	    }
	    else
	    {
	        sfa_calc_tlb(query_ts, query_fft, query_sfa, sfa, child_node);	
	   }
	}
	child_node = child_node->next;	      	      	    
    }
  }
  else {
    sfa_calc_tlb_leaf (query_ts, query_fft, query_sfa,sfa,curr_node);
  }
  

}

