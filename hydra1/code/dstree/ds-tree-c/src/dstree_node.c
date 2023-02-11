//
//  dstree_node.c
//  ds-tree C version
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
#include "../include/dstree_node.h"
#include "../include/dstree_index.h"
#include <math.h>
#include <pqueue.h>
#include <float.h>
#include <limits.h>
#include "../include/pqueue.h"
#include "../include/dstree_query_engine.h"

/**
 This function initializes a dstree root node.
 */

struct dstree_node * dstree_root_node_init(struct dstree_index_settings * settings) 
{

    struct dstree_node * node = dstree_leaf_node_init();
    int ts_length = settings->timeseries_size;
    int segment_size = settings->init_segments;
    
    node->node_segment_split_policies = malloc (sizeof (struct node_segment_split_policy) * 2 );

    if(node->node_segment_split_policies == NULL) {
     fprintf(stderr,"Error in dstree_node.c: Could not allocate memory for root node segment \
                     split policies.\n");
     return FAILURE;	
    }
    
    node->node_segment_split_policies[0].indicator_split_idx = 0;  //Mean  
    node->node_segment_split_policies[1].indicator_split_idx = 1;  //Stdev
    node->num_node_segment_split_policies = 2;  //Stdev and Mean
          
    //calc the split points by segmentSize
    short * split_points = NULL;
    split_points = malloc (sizeof(short) * segment_size);

    if(split_points == NULL) {
      fprintf(stderr,"Error in dstree_node.c: Could not \
                     allocate memory for root split points.\n");
      return FAILURE;	
    }

    if(!calc_split_points(split_points, ts_length, segment_size)) 
    {
      fprintf(stderr,"Error in dstree_node.c: Could not \
                     calculate the split points for the root.\n");
      return FAILURE;	
    }
    
    if(!node_init_segments(node, split_points, segment_size))
    {
      fprintf(stderr,"Error in dstree_node.c: Could not \
                     initialize the segments for the root.\n");
      return FAILURE;	
    }
    
    if(!create_dstree_node_filename(settings, node,NULL))
    {
      fprintf(stderr,"Error in dstree_node.c: Could not \
                     create a filename for the root node.\n");
      return FAILURE;	
    }

    free(split_points);
    return node;
}

/**
 This function initalizes a dstree leaf node.
 */

struct dstree_node * dstree_leaf_node_init(void) 
{
    COUNT_NEW_NODE
 
    struct dstree_node *node = malloc(sizeof(struct dstree_node));
    if(node == NULL) {
        fprintf(stderr,"error: could not allocate memory for new node.\n");
        return NULL;
    }
    node->right_child = NULL;
    node->left_child = NULL;
    node->parent = NULL;

    node->filename = NULL;
    
    node->node_segment_split_policies=NULL;
    node->num_node_segment_split_policies=2;
    
    node->range=0;  

    node->level = 0;   
    node->is_left = false;
    node->is_leaf = true;
    
    node->split_policy=NULL; 
    node->node_points=NULL;   
    node->hs_node_points=NULL;
    node->num_node_points=0;   
    node->num_hs_node_points=0;
    
    node->node_segment_sketches=NULL;
    node->hs_node_segment_sketches=NULL;

    node->max_segment_length = 2; 
    node->max_value_length = 10; 

    node->file_buffer = NULL;
      
    node->node_size = 0;    
    
    return node;
}



enum response node_init_segments(struct dstree_node * node, short * split_points, int segment_size)
{     

  node->num_node_points = segment_size;

  node->node_points = NULL;
  node->node_points = malloc (sizeof(short) * segment_size);
  if(node->node_points == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for node points.\n");
     return FAILURE;	
  }
    
  for (int i = 0; i < segment_size; ++i)
  {
    node->node_points[i] = split_points[i];
  }
  
  node->hs_node_points = NULL;
  node->hs_node_points = malloc (sizeof(short) * segment_size * 2);
  if(node->hs_node_points == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for hs node points.\n");
     return FAILURE;	
  }
  
  int min_length = 1; //mininum length of new segment = 1
  int num_hs_split_points = 0;
    
  calc_hs_split_points(node->hs_node_points, &node->num_hs_node_points,node->node_points, segment_size, min_length);

  //allocate mem for array of sketches


  node->node_segment_sketches = NULL;
  node->node_segment_sketches = malloc (sizeof(struct segment_sketch) * segment_size);
  if(node->node_segment_sketches == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for node segment sketches.\n");
     return FAILURE;	
  }
  
  node->hs_node_segment_sketches = NULL;  
  node->hs_node_segment_sketches = malloc (sizeof(struct segment_sketch) * node->num_hs_node_points);
  if(node->hs_node_segment_sketches == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for hs node segment sketches.\n");
     return FAILURE;	
  }
  
  //allocate memory for vertical indicators
  for (int i = 0; i < segment_size; ++i)
  {
    node->node_segment_sketches[i].indicators = NULL;    
    node->node_segment_sketches[i].indicators = malloc (sizeof(ts_type) * 4); //node segment has 4 indicators

    if(node->node_segment_sketches[i].indicators == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for node segment \
                     sketch indicator.\n");
     return FAILURE;	
    }
    
    node->node_segment_sketches[i].indicators[0]= -FLT_MAX;
    node->node_segment_sketches[i].indicators[1]= FLT_MAX;
    node->node_segment_sketches[i].indicators[2]= -FLT_MAX;
    node->node_segment_sketches[i].indicators[3]= FLT_MAX;
    node->node_segment_sketches[i].num_indicators= 4;
  }

  //allocate memory for horizontal indicators
  for (int i = 0; i < node->num_hs_node_points; ++i)
  {
    node->hs_node_segment_sketches[i].indicators = NULL;
    node->hs_node_segment_sketches[i].indicators = malloc (sizeof(ts_type) * 4); //node segment has 4 indicators
    if(node->hs_node_segment_sketches[i].indicators == NULL) {
     fprintf(stderr,"Error in node_init_segments(): Could not allocate memory for hs node segment \
                     sketch indicator.\n");
     return FAILURE;	
    }
    node->hs_node_segment_sketches[i].indicators[0]= -FLT_MAX;
    node->hs_node_segment_sketches[i].indicators[1]= FLT_MAX;
    node->hs_node_segment_sketches[i].indicators[2]= -FLT_MAX;
    node->hs_node_segment_sketches[i].indicators[3]= FLT_MAX;    
    node->hs_node_segment_sketches[i].num_indicators= 4;
  }  

  return SUCCESS;

}

enum response append_ts_to_node(struct dstree_index * index,
				struct dstree_node * node,
				ts_type * timeseries)
{

  if (!get_file_buffer(index, node))
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

  int ts_length = index->settings->timeseries_size;
  int max_leaf_size = index->settings->max_leaf_size;

  if (idx == 0)
  {
      node->file_buffer->buffered_list = NULL;
      node->file_buffer->buffered_list = malloc(sizeof(struct ts_type *) * max_leaf_size);
      
      if (node->file_buffer->buffered_list == NULL)
      {
        fprintf(stderr, "Error in dstree_index.c:  Could not \
                         allocate memory for the buffered list. \n");
        return FAILURE;                  
      }
  }
  /*
  node->file_buffer->buffered_list[idx] = NULL;
  node->file_buffer->buffered_list[idx] = malloc(sizeof(ts_type) * ts_length);
  */

  node->file_buffer->buffered_list[idx] = (ts_type *) index->buffer_manager->current_record;
  index->buffer_manager->current_record += sizeof(ts_type) * ts_length;
  index->buffer_manager->current_record_index++;
    
  if (node->file_buffer->buffered_list[idx] == NULL)
  {
        fprintf(stderr, "Error in dstree_index.c:  Could not \
                         allocate memory for the time series in\
                         the buffer.\n");
        return FAILURE;                  
  }
  
  for(int i=0; i<ts_length; ++i)
  {
        node->file_buffer->buffered_list[idx][i] = timeseries[i];
  }
    
  ++node->file_buffer->buffered_list_size;
  index->buffer_manager->current_count += ts_length; 
    
  return SUCCESS;
}

enum response append_ts_to_child_node(struct dstree_index * index,
				struct dstree_node * node,
				ts_type * timeseries)
{

  // fprintf(stderr, "IN APPEND TS TO CHILD NODE.\n");
  if (!get_file_buffer(index, node))
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

  int ts_length = index->settings->timeseries_size;
  int max_leaf_size = index->settings->max_leaf_size;

  if (idx == 0)
  {
      node->file_buffer->buffered_list = NULL;
      node->file_buffer->buffered_list = malloc(sizeof(struct ts_type *) * max_leaf_size);
      
      if (node->file_buffer->buffered_list == NULL)
      {
        fprintf(stderr, "Error in dstree_index.c:  Could not \
                         allocate memory for the buffered list. \n");
        return FAILURE;                  
      }
  }


  node->file_buffer->buffered_list[idx] = (ts_type *) index->buffer_manager->current_record;
  index->buffer_manager->current_record += sizeof(ts_type) * ts_length;
  index->buffer_manager->current_record_index++;
    
  if (node->file_buffer->buffered_list[idx] == NULL)
  {
        fprintf(stderr, "Error in dstree_index.c:  Could not \
                         allocate memory for the time series in\
                         the buffer.\n");
        return FAILURE;                  
  }
  
  for(int i=0; i<ts_length; ++i)
  {
        node->file_buffer->buffered_list[idx][i] = timeseries[i];
  }
    
  ++node->file_buffer->buffered_list_size;

 
    
  return SUCCESS;
}


enum response create_dstree_node_filename(struct dstree_index_settings *settings,
                                          struct dstree_node * node,
                                          struct dstree_node * parent_node)
{
    int i;
    
    node->filename = malloc(sizeof(char) * (settings->max_filename_size));

    int l = 0;
    l += sprintf(node->filename+l ,"%02d", node->num_node_points);
    
    // If this has level other than 0 then it is not a root node and as such it does have some 
    // split data on its parent.

    if (node->level) {
      if(node->is_left)
      {
  	 l += sprintf(node->filename+l ,"%s", "_L");
      }
      else
      {
  	 l += sprintf(node->filename+l ,"%s", "_R");	
      }

      //Add parent split policy, just the code 0 for mean and 1 for stdev
      struct node_split_policy *policy;
      policy = parent_node->split_policy;
      
      if(policy->indicator_split_idx)
      {
  	 l += sprintf(node->filename+l ,"%s", "_1");
      }
      else
      {
  	 l += sprintf(node->filename+l ,"%s", "_0");	
      }
      
      l += sprintf(node->filename+l ,"_(%d,%d,%g)",policy->split_from, policy->split_to,
		   policy->indicator_split_value);
      

    }
    
    // If its level is 0 then it is a root
    l += sprintf(node->filename+l ,"_%d", node->level);	      

    return SUCCESS;
}


enum response update_node_statistics(struct dstree_node * node, ts_type * timeseries)
{

  //update vertical node_segment_sketch
  for (int i = 0; i < node->num_node_points; i++) {
    int from = 0;
    int to = 0;

    from = get_segment_start(node->node_points, i);
    to = get_segment_end(node->node_points, i);

    if (!node_segment_sketch_update_sketch(&node->node_segment_sketches[i], timeseries, from,to))
    {
        fprintf(stderr, "Error in dstree_index.c:  Could not update vertical sketch for node segment.\n");
        return FAILURE;                  
    }
          
  }
  
  //update horizontal node_segment_sketch
  for (int i = 0; i < node->num_hs_node_points; i++) {
    if (!node_segment_sketch_update_sketch(&node->hs_node_segment_sketches[i], timeseries, get_segment_start(node->hs_node_points, i), get_segment_end(node->hs_node_points, i)))
    {
        fprintf(stderr, "Error in dstree_index.c:  Could not update horizontal sketch for node segment.\n");
        return FAILURE;                  
    }
    
  }

    ++node->node_size;

    return SUCCESS;
}


/*

  This function calculates the euclidean distance of query to a 
  given node.
  
  The dstree and isax both load all the time series in the node 
  and compare each to the query. 

  The function returns the smallest distance between the query
  and the time series in the node.

 */

void calculate_node_knn_distance (struct dstree_index *index, struct dstree_node *node,
				  ts_type *query_ts_reordered, int *query_order,
				  unsigned int offset, ts_type bsf,
				  unsigned int k,
				  struct query_result  *knn_results,
				  struct bsf_snapshot ** bsf_snapshots,
				  unsigned int *cur_bsf_snapshot,
				  unsigned int * cur_size)
{
     ts_type distance = FLT_MAX;

     //get the k-th distance from the results queue
     ts_type kth_bsf = FLT_MAX;

     
    //count the number of leaves and the number of time series that were checked
    //this is different from the count_loaded_node and counted_loaded_ts
    //which count the number of leaves and time series that were not found in
    //memory and had to be retrieved from disk
    //checked_nodes = loaded_nodes + nodes_in_memory    
    COUNT_CHECKED_NODE
    COUNT_CHECKED_TS(node->node_size)		  

    
    //TEST THAT DATA IS FULLY IN MEM
    //If the leaf's data is in disk, load it
    if (node->file_buffer->buffered_list_size == 0) 
    {
      COUNT_LOADED_NODE      
      COUNT_LOADED_TS(node->node_size)
      COUNT_PARTIAL_LOAD_NODE_TIME_START
 
      node->file_buffer->buffered_list = get_all_time_series_in_node(index, node);
      node->file_buffer->buffered_list_size = node->file_buffer->disk_count;


      if (node->file_buffer->buffered_list == NULL)
       {
            fprintf(stderr, "Error in dstree_index.c:  Could not retrieve all time series for node %s.\n", node->filename);
       }
       COUNT_PARTIAL_LOAD_NODE_TIME_END      
    }
    //If the leaf's data is in memory, proceed. A leaf's data is either fully in disk or in memory 
    double tS_bsf;
    double tE_bsf;
    struct timeval current_time_bsf;
	
    for (int idx = 0; idx < node->file_buffer->buffered_list_size; ++idx)
    {  

      struct query_result result;
      result =  knn_results[k-1];
      
      kth_bsf = result.distance;

      distance = ts_euclidean_distance_reordered(query_ts_reordered,
						 node->file_buffer->buffered_list[idx],
						 offset,  //offset is 0 for whole matching
						 index->settings->timeseries_size,
						 kth_bsf,
						 query_order);


      if (distance < kth_bsf)
      {
	struct query_result object_result;// =  malloc(sizeof(struct query_result));
	object_result.node = node;
	object_result.distance =  distance;
	

	queue_bounded_sorted_insert(knn_results, object_result, cur_size, k);
	if (cur_bsf_snapshot != NULL)
	{
	  gettimeofday(&current_time_bsf, NULL);
	  tS_bsf = partial_time_start.tv_sec*1000000 + (partial_time_start.tv_usec); 
	  tE_bsf = current_time_bsf.tv_sec*1000000  + (current_time_bsf.tv_usec);	
	  for (int j = 0; j < k; ++j)
	    {
	      bsf_snapshots[j][*cur_bsf_snapshot].distance = knn_results[j].distance;
	      bsf_snapshots[j][*cur_bsf_snapshot].time =tE_bsf - tS_bsf;
	    }
	  ++(*cur_bsf_snapshot);
	}
	
      }

    }

    //clearing the data for this node
     for (int i = 0 ; i < index->settings->max_leaf_size; ++i)
     {
       free(node->file_buffer->buffered_list[i]);
     }

     free(node->file_buffer->buffered_list);

     node->file_buffer->buffered_list = NULL;
     node->file_buffer->buffered_list_size = 0;
    
}


ts_type calculate_node_distance (struct dstree_index *index, struct dstree_node *node, ts_type *query_ts_reordered, int *query_order, unsigned int offset, ts_type bsf)
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


    //TEST THAT DATA IS FULLY IN MEM
    //If the leaf's data is in disk, load it
    if (node->file_buffer->buffered_list_size == 0) 
    {
      COUNT_LOADED_NODE      
      COUNT_LOADED_TS(node->node_size)
      COUNT_PARTIAL_LOAD_NODE_TIME_START
 
      node->file_buffer->buffered_list = get_all_time_series_in_node(index, node);
      node->file_buffer->buffered_list_size = node->file_buffer->disk_count;


      if (node->file_buffer->buffered_list == NULL)
       {
            fprintf(stderr, "Error in dstree_index.c:  Could not retrieve all time series for node %s.\n", node->filename);
       }
       COUNT_PARTIAL_LOAD_NODE_TIME_END      
    }
    //If the leaf's data is in memory, proceed. A leaf's data is either fully in disk or in memory 

    dist = calculate_ts_in_node_distance(index, node, query_ts_reordered, query_order, offset,bsf); 

    //clearing the data for this node
     for (int i = 0 ; i < index->settings->max_leaf_size; ++i)
     {
       free(node->file_buffer->buffered_list[i]);
     }

     free(node->file_buffer->buffered_list);

    node->file_buffer->buffered_list = NULL;
    node->file_buffer->buffered_list_size = 0;
    

    return dist;
}

ts_type calculate_ts_in_node_distance (struct dstree_index *index,
				       struct dstree_node *node,
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
						node->file_buffer->buffered_list[idx],
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf,
						query_order);     
      if (temp_dist < bsf)
      {
        bsf = temp_dist;	
      }
    }

    return bsf;
}



ts_type calculate_node_min_distance (struct dstree_index *index, struct dstree_node *node, ts_type *query)
{
    ts_type sum = 0;
    short *points = node->node_points;
    int num_points = (int) node->num_node_points;

    
    ts_type * mean_per_segment = malloc(sizeof(ts_type) * num_points);
    ts_type * stdev_per_segment= malloc(sizeof(ts_type) * num_points);

    calc_mean_per_segment(query, points,mean_per_segment, num_points);
    calc_stdev_per_segment(query, points,stdev_per_segment, num_points);


    for (int i=0; i < num_points; ++i)
    {
      //use mean and standard deviation to estimate the distance
       ts_type temp_dist = 0;

        if ((stdev_per_segment[i] - node->node_segment_sketches[i].indicators[2]) *
	    (stdev_per_segment[i] - node->node_segment_sketches[i].indicators[3]) > 0)
	{
          temp_dist += pow(fmin(fabs(stdev_per_segment[i] - node->node_segment_sketches[i].indicators[2]),
				fabs(stdev_per_segment[i] - node->node_segment_sketches[i].indicators[3])), 2);
        }

        if ((mean_per_segment[i] - node->node_segment_sketches[i].indicators[0]) *
	    (mean_per_segment[i] - node->node_segment_sketches[i].indicators[1]) > 0)
	{
          temp_dist += pow(fmin(fabs(mean_per_segment[i] - node->node_segment_sketches[i].indicators[0]),
				fabs(mean_per_segment[i] - node->node_segment_sketches[i].indicators[1])), 2);
        }
        sum += temp_dist * get_segment_length(points, i);
    }


    //sum = sqrt(sum);
    
    free(mean_per_segment);
    free(stdev_per_segment);
    return sum;
}

ts_type calculate_node_max_distance (struct dstree_index *index, struct dstree_node *node, ts_type *query)
{
    ts_type sum = 0;
    short *points = node->node_points;
    int num_points = (int) node->num_node_points;

    
    ts_type * mean_per_segment = malloc(sizeof(ts_type) * num_points);
    ts_type * stdev_per_segment= malloc(sizeof(ts_type) * num_points);

    calc_mean_per_segment(query, points,mean_per_segment, num_points);
    calc_stdev_per_segment(query, points,stdev_per_segment, num_points);


    for (int i=0; i < num_points; ++i)
      {
	//use mean and standard deviation to estimate the distance
	ts_type temp_dist = 0;
	
	temp_dist += pow(stdev_per_segment[i] + node->node_segment_sketches[i].indicators[2], 2);
	    
	ts_type ub_threshold = (node->node_segment_sketches[i].indicators[0] + node->node_segment_sketches[i].indicators[1]) / 2.0;
	
        if (mean_per_segment[i] <= ub_threshold)
	  {
	    temp_dist += pow(fabs(mean_per_segment[i] - node->node_segment_sketches[i].indicators[0]), 2);
	  }
	  else
	    {
	      temp_dist += pow(fabs(mean_per_segment[i] - node->node_segment_sketches[i].indicators[1]), 2);	  
	    }
	  sum += temp_dist * get_segment_length(points, i);
      }
    //sum = sqrt(sum);
    
    free(mean_per_segment);
    free(stdev_per_segment);
    
    if (sum == 0)
      return FLT_MAX;
    else
      return sum;
}

int queue_bounded_sorted_insert(struct  query_result *q, struct query_result d, unsigned int *cur_size, unsigned int k)
{
    struct query_result  temp;
    size_t i;
    size_t newsize;

    bool is_duplicate = false;
    for (unsigned int itr = 0 ; itr < *cur_size ; ++itr)
      {
	if (q[itr].distance == d.distance)
	  is_duplicate = true;
      }
   
    if (!is_duplicate)
      {
    
	/* the queue is full, overwrite last element*/
	if (*cur_size == k) {      
	  q[k-1].distance = d.distance;
	  q[k-1].node = d.node;
	}
	else
	  {
	    q[*cur_size].distance = d.distance;
	    q[*cur_size].node = d.node;      
	    ++(*cur_size);
	  }

	unsigned int idx,j;

	idx = 1;
    
	while (idx < *cur_size)
	  {
	    j = idx;
	    while ( j > 0 &&  ( (q[j-1]).distance > q[j].distance)) 
	      {
		temp = q[j];
		q[j].distance = q[j-1].distance;
		q[j].node = q[j-1].node;	
		q[j-1].distance = temp.distance;
		q[j-1].node = temp.node;		
		--j;
	      }
	    ++idx;
	  }
      }
   return 0;
}


