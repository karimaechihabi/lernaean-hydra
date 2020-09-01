//
//  sfa_node.c
//  SFA C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//



#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../globals.h"
#include "../include/sfa_node.h"
#include "../include/sfa_trie.h"
#include <math.h>
#include <float.h>
#include <limits.h>

/**
 This function initializes a sfa root node.
 */


struct sfa_node * sfa_root_node_init(struct sfa_trie_settings * settings) 
{

    struct sfa_node * node = sfa_leaf_node_init(settings);

    return node;
}

/*
 This function initalizes a sfa leaf node.
 */


struct sfa_node * sfa_leaf_node_init(struct sfa_trie_settings * settings) 
{
    COUNT_NEW_NODE
 
    struct sfa_node *node = malloc(sizeof(struct sfa_node));
    if(node == NULL) {
        fprintf(stderr,"error: could not allocate memory for new node.\n");
        exit; //return NULL;
    }
    node->parent = NULL;
    //each node has at most num_symbols children
    node->children = NULL;
    node->next = NULL;
    
    node->filename = NULL;

    node->sfa_word = NULL;
    node->sfa_word = malloc(settings->fft_size*sizeof(unsigned char));

    node->min_values = NULL; //size of both should be equal to prefix length
    node->min_values = calloc(settings->fft_size,sizeof(ts_type));
    
    node->max_values = NULL;
    node->max_values = calloc(settings->fft_size,sizeof(ts_type));
    
    for (int i=0; i < settings->fft_size; ++i)
    {
      node->min_values[i] = FLT_MAX;
      node->max_values[i] = -FLT_MAX;
      node->sfa_word[i] = '\0';
    }
 

    node->prefix_length = 0;
        
    node->is_leaf = true;
    
    node->file_buffer = NULL;
      
    node->node_size = 0;    
    node->level = 0;    
    
    return node;
}


enum response sfa_node_create_children(struct sfa_trie * trie, struct sfa_node * parent)
{

  //create all children at once, node has at most num_symbols
  //children

  unsigned int num_symbols = trie->settings->num_symbols;
  
  unsigned int i = 0;
  unsigned int j = 0;

  struct sfa_node * cur = NULL;
  
  while (i < num_symbols)
  {
     if (i == 0)
     {
        parent->children = sfa_leaf_node_init(trie->settings);
        cur = parent->children;
     }
     else
     {
        cur->next = sfa_leaf_node_init(trie->settings);
	cur = cur->next;
     }
     cur->parent = parent;
     cur->level = parent->level + 1;
     cur->prefix_length = parent->prefix_length + 1;
     
     for (j = 0 ; j < cur->prefix_length - 1 ; ++j)
     {
	 cur->sfa_word[j] = parent->sfa_word[j];	 	   
     }          
     cur->sfa_word[j] = (unsigned char) i;
    
     sfa_node_create_filename(trie->settings,cur, i);     
     ++i;
  }

  cur->next = NULL;
  parent->is_leaf = false;
      
  return SUCCESS;
}


enum response sfa_node_update_statistics(struct sfa_trie * trie, struct sfa_node * node, ts_type * dft_transform)
{

  unsigned int len = trie->settings->fft_size;
  
  for (int i = 0; i < len; i++) {
    node->min_values[i] = fminf(node->min_values[i], dft_transform[i]);
    node->max_values[i] = fmaxf(node->max_values[i], dft_transform[i]);
  }
  
  ++node->node_size;

   return SUCCESS;
}


enum response sfa_node_append_record(struct sfa_trie * trie, struct sfa_node * node, struct sfa_record* record)
{
  if (!get_file_buffer(trie, node))
  {
    fprintf(stderr, "Error in dstree_index.c:  Could not get the \
                     file buffer for this node.\n");
    return FAILURE;              
  }

  if (node->file_buffer == NULL)
  {
    fprintf(stderr, "Error in dstree_index.c:  Null file buffer for \
                     this node after creating it.\n");
    return FAILURE;              
  }

  int idx = node->file_buffer->buffered_list_size;  

  unsigned int ts_length = trie->settings->timeseries_size;
  int max_leaf_size = trie->settings->max_leaf_size;
  unsigned int transforms_size = trie->settings->fft_size; 

    
  if (idx == 0)
  {
      node->file_buffer->buffered_list = NULL;
      //node->file_buffer->buffered_list = malloc(sizeof(struct ts_type *) * max_leaf_size);
      node->file_buffer->buffered_list = malloc(sizeof(struct sfa_record) * max_leaf_size);
 
      if (node->file_buffer->buffered_list == NULL)
      {
        fprintf(stderr, "Error in dstree_index.c:  Could not \
                         allocate memory for the buffered list. \n");
        return FAILURE;                  
      }
  }

  node->file_buffer->buffered_list[idx].timeseries = (ts_type *) trie->buffer_manager->current_ts_record;
  node->file_buffer->buffered_list[idx].dft_transform = (ts_type *) trie->buffer_manager->current_dft_record;
  node->file_buffer->buffered_list[idx].sfa_word = (unsigned char *) trie->buffer_manager->current_sfa_record;

  for (int i = 0; i < ts_length;++i)
  {
    node->file_buffer->buffered_list[idx].timeseries[i] = record->timeseries[i];
  }
  for (int i = 0; i < transforms_size;++i)
  {
    node->file_buffer->buffered_list[idx].dft_transform[i] = record->dft_transform[i];
    node->file_buffer->buffered_list[idx].sfa_word[i] = record->sfa_word[i];    
  }  
  
  trie->buffer_manager->current_ts_record += sizeof(ts_type) * ts_length;
  trie->buffer_manager->current_dft_record += sizeof(ts_type) * transforms_size;
  trie->buffer_manager->current_sfa_record += sizeof(unsigned char) * transforms_size;

  trie->buffer_manager->current_record_index++;

  ++node->file_buffer->buffered_list_size;

  sfa_node_update_statistics(trie,node,record->dft_transform);     	   
  return SUCCESS;
}



enum response sfa_node_create_filename(struct sfa_trie_settings *settings,
				       struct sfa_node * node,
				       unsigned int index)
{

    
    node->filename = malloc(sizeof(char) * (settings->max_filename_size));
    
    int l = 0;
    int n = 0;
    // If this has level other than 0 then it is not a root node and as such it does have some 
    // split data on its parent.

    char * parent_name = node->parent->filename;
    
    if (node->level) //the root does not have a filename
    {
         if (parent_name != NULL)
	 {
	   l += sprintf(node->filename,"%s", parent_name);
	 }
         l += sprintf(node->filename+l,"_%u", index);
    }
    
    return SUCCESS;
}


ts_type calculate_node_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query_ts_reordered, int *query_order, unsigned int offset, ts_type bsf)
{
//    ts_type bsf = FLT_MAX;
     ts_type dist = FLT_MAX;

    //count the number of leaves and the number of time series that were checked
    //this is different from the count_loaded_node and counted_loaded_ts
    //which count the number of leaves and time series that were not found in
    //memory and had to be retrieved from disk
    //checked_nodes = loaded_nodes + nodes_in_memory    
    COUNT_CHECKED_NODE
    COUNT_CHECKED_TS(node->node_size)	  
    boolean list_was_empty = false;

    //TEST THAT DATA IS FULLY IN MEM
    //If the leaf's data is in disk, load it
    if (node->file_buffer->buffered_list_size == 0) 
    {
      COUNT_LOADED_NODE      
      COUNT_LOADED_TS(node->node_size)
      COUNT_PARTIAL_LOAD_NODE_TIME_START
	list_was_empty = true;
      node->file_buffer->buffered_list = get_all_time_series_in_node(trie, node);
      node->file_buffer->buffered_list_size = node->file_buffer->disk_count;

      if (node->file_buffer->buffered_list == NULL)
       {
            fprintf(stderr, "Error in sfa_trie.c:  Could not retrieve all time series for node %s.\n", node->filename);
       }
       COUNT_PARTIAL_LOAD_NODE_TIME_END
    }
    
    //If the leaf's data is in memory, proceed. A leaf's data is either fully in disk or in memory 

    dist = calculate_ts_in_node_distance(trie, node, query_ts_reordered, query_order, offset,bsf);


    //clearing the data for this node
    if (list_was_empty)
      {
    for (int i = 0 ; i < node->file_buffer->buffered_list_size; ++i)
      {
	free(node->file_buffer->buffered_list[i].timeseries);
      }	 
     free(node->file_buffer->buffered_list);

     node->file_buffer->buffered_list = NULL;
     node->file_buffer->buffered_list_size = 0;
      }

     return dist;
}

ts_type calculate_ts_in_node_distance (struct sfa_trie *trie,
				       struct sfa_node *node,
				       ts_type *query_ts_reordered,
				       int * query_order,
				       unsigned int offset,
				       ts_type bound)
{
    ts_type bsf = bound;
    ts_type temp_dist;
    
    for (int idx = 0; idx < node->file_buffer->buffered_list_size; ++idx)
    {  
      
      temp_dist = ts_euclidean_distance_reordered(query_ts_reordered,
						node->file_buffer->buffered_list[idx].timeseries,
						offset,  //offset is 0 for whole matching
						trie->settings->timeseries_size,
						bsf,
						query_order);     
      
      
      if (temp_dist < bsf)
      {
        bsf = temp_dist;	
      }
    }

    return bsf;
}

ts_type calculate_sfa_node_min_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query, ts_type *query_fft,
					 unsigned char * query_sfa, ts_type bsf, boolean normed)
{
    ts_type distance = 0;

    unsigned int i = 0;
    
    // mean (DC) value    
    if (!normed)
    {
      distance = sfa_fft_min_dist(trie, node->sfa_word[0],query_sfa[0],query_fft[0],0);
      distance *= distance;
      i += 2;
    }
        
    //printf(" prefix_length = %u ", node->prefix_length);
    
    for (; i < node->prefix_length; i++) {

      ts_type value = sfa_fft_min_dist(trie,node->sfa_word[i],query_sfa[i],query_fft[i],i);
      
      distance += 2*value*value;
      //distance += value*value;
      
      // pruning if distance threshold is exceeded
             
      if (distance > bsf) {
        return distance;
      }
            
    }
   
    return distance;
}

ts_type sfa_fft_min_dist (struct sfa_trie * trie, unsigned char c1_value, unsigned char c2_value, ts_type real_c2, unsigned int dim) {
  if (c1_value == c2_value) {
    return 0;
  }

  if (c1_value > c2_value)
  {
    return  (trie->bins[dim][((int)c1_value)-1] - real_c2);
  }
  
  if (c1_value < c2_value)
  {
    return  (real_c2 - trie->bins[dim][(int)c1_value]);
  }    

}


ts_type calculate_node_min_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query, ts_type *query_fft)
{
    ts_type distance = 0;
    ts_type value = 0;
    

    for (int i = 0; i < trie->settings->fft_size; i++) {
      // below
      if (query_fft[i] < node->min_values[i])
      {
	value = node->min_values[i] - query_fft[i];
        distance = distance + (2 * value * value);
      }
      // above
      else if (query_fft[i] > node->max_values[i])
      {
	value = node->max_values[i] - query_fft[i];
        distance = distance + (2 * value * value);
      }
    }    

    return distance;
}

