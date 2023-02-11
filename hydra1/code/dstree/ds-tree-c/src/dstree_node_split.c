//
//  dstree_node_split.c
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
#include "../include/dstree_node.h"
#include "../include/dstree_index.h"
#include "../include/dstree_node_split.h"
#include "../include/calc_utils.h"
#include "float.h"


enum response mean_node_segment_split_policy_split(struct node_segment_split_policy * policy, struct segment_sketch sketch, struct segment_sketch * ret)
{

  const int num_splits=2; //default split into 2 node
  
  ts_type max_mean = sketch.indicators[0]; 
  ts_type min_mean = sketch.indicators[1];
  
  policy->indicator_split_idx = 0 ;  //stdev based split
  policy->indicator_split_value = (max_mean + min_mean) / 2;  //the mean value is split value

  ret[0].num_indicators = sketch.num_indicators;
  ret[1].num_indicators = sketch.num_indicators;
    
  for (int i = 0; i < num_splits; i++) {
    for(int j= 0; j<ret[i].num_indicators; ++j){
	ret[i].indicators[j] = sketch.indicators[j];
    }
  }

  /* make sure indicator split idx is 0*/
  ret[0].indicators[1] = policy->indicator_split_value;
  ret[1].indicators[0] = policy->indicator_split_value;

  return SUCCESS;
}

enum response stdev_node_segment_split_policy_split(struct node_segment_split_policy * policy, struct segment_sketch sketch, struct segment_sketch *ret){

  const int num_splits=2; //default split into 2 node
  
  ts_type max_stdev = sketch.indicators[2]; 
  ts_type min_stdev = sketch.indicators[3];

  policy->indicator_split_idx = 1 ;  //stdev based split
  policy->indicator_split_value = (ts_type)(max_stdev + min_stdev) / 2;  //the mean of stdev value is split value

  ret[0].num_indicators = sketch.num_indicators;
  ret[1].num_indicators = sketch.num_indicators;
    
  for (int i = 0; i < num_splits; i++) {
    for(int j= 0; j<ret[i].num_indicators; ++j){
	ret[i].indicators[j] = sketch.indicators[j];
    }
  }

    /* make sure indicator split idx is 1*/
  ret[0].indicators[2] = policy->indicator_split_value;
  ret[1].indicators[3] = policy->indicator_split_value;

  return SUCCESS;
}

enum response series_segment_sketch_do_sketch(struct segment_sketch * series_segment_sketch, ts_type * series, int fromIdx, int toIdx)
{

   series_segment_sketch->indicators[0] = calc_mean (series, fromIdx, toIdx);
   series_segment_sketch->indicators[1] = calc_stdev (series, fromIdx, toIdx);

   return SUCCESS;
}

enum response node_segment_sketch_update_sketch(struct segment_sketch * node_segment_sketch, ts_type * series, int fromIdx, int toIdx)
{

   struct segment_sketch series_segment_sketch;

   series_segment_sketch.indicators = NULL;
   series_segment_sketch.indicators = malloc (sizeof(ts_type)*2);
   if(series_segment_sketch.indicators == NULL) {
        fprintf(stderr,"Error in dstree_node_split.c: Could not allocate memory for series \
                        segment sketch indicators.\n");
        return FAILURE;
   }
   series_segment_sketch.num_indicators = 2;  
   
   if(!series_segment_sketch_do_sketch (&series_segment_sketch, series, fromIdx, toIdx))
   {
    fprintf(stderr,"Error in node_init_segments(): Could not calculate the series segment  \
                    sketch.\n");
     return FAILURE;	
   }
    

   if (node_segment_sketch->indicators == NULL)
   {
     node_segment_sketch->indicators =  malloc(sizeof(ts_type) * 4);
     if(node_segment_sketch->indicators == NULL) {
        fprintf(stderr,"Error in dstree_node_split.c: Could not allocate memory for node \
                        segment sketch indicators.\n");
        return FAILURE;
     }     
     node_segment_sketch->indicators[0] = -FLT_MAX ; //for max mean
     node_segment_sketch->indicators[1] = FLT_MAX; //for min mean
     node_segment_sketch->indicators[2] = -FLT_MAX; //for max stdev
     node_segment_sketch->indicators[3] = FLT_MAX; //for min stdev
     node_segment_sketch->num_indicators = 4;                     
   }
   
   
   node_segment_sketch->indicators[0] = fmaxf(node_segment_sketch->indicators[0], series_segment_sketch.indicators[0]);
   node_segment_sketch->indicators[1] = fminf(node_segment_sketch->indicators[1], series_segment_sketch.indicators[0]);
   node_segment_sketch->indicators[2] = fmaxf(node_segment_sketch->indicators[2], series_segment_sketch.indicators[1]);
   node_segment_sketch->indicators[3] = fminf(node_segment_sketch->indicators[3], series_segment_sketch.indicators[1]);
   node_segment_sketch->num_indicators = 4;                     

   free(series_segment_sketch.indicators);
   series_segment_sketch.indicators = NULL;

   return SUCCESS;
}

boolean node_split_policy_route_to_left (struct dstree_node * node, ts_type * series)
{


  struct segment_sketch series_segment_sketch;
  series_segment_sketch.indicators = NULL;
  series_segment_sketch.indicators = malloc (sizeof(ts_type)*2);
  if(series_segment_sketch.indicators == NULL) {
        fprintf(stderr,"Error in dstree_node_split.c: Could not allocate memory for series \
                        segment sketch indicators.\n");
  }

  series_segment_sketch.num_indicators = 2;

  struct node_split_policy policy = *node->split_policy;
   boolean route_to_left = false;
  
   if(!series_segment_sketch_do_sketch (&series_segment_sketch, series, policy.split_from, policy.split_to))
     {
        fprintf(stderr,"Error in dstree_node_split.c: Could not calculate the series  \
                        segment sketch .\n");
     }

     
  
  if (series_segment_sketch.indicators[policy.indicator_split_idx] < policy.indicator_split_value)
  {
    route_to_left = true;
  }
  
    free(series_segment_sketch.indicators);
    series_segment_sketch.indicators = NULL;

  return route_to_left;

}



enum response split_node (struct dstree_index * index, struct dstree_node * node, short * child_node_points, int num_child_node_points)
{
  if (!node->is_leaf)
  {
	
        fprintf(stderr,"Error in dstree_node_split.c: Trying to split a node  \
                        that is not a leaf %s.\n", node->filename);
	return FAILURE;
  }
  else
  {
     //init children nodes
    //COUNT_LEAF_NODE //only add one leaf since parent was already counted
      
    node->left_child = create_child_node(node);
    if (node->left_child == NULL)
    {
        fprintf(stderr,"Error in dstree_node_split.c: Left child not \
                        initialized properly.\n");
	return FAILURE;
    }
    
    if (!node_init_segments(node->left_child,child_node_points,num_child_node_points))
    {
        fprintf(stderr,"Error in dstree_node_split.c: Could not initialize       \
                        segments for left child of node %s.\n", node->filename);
	return FAILURE;
    }
    
    node->left_child->is_leaf = 1;
    node->left_child->is_left = true;
    //the parent node is passed since it has the policy and segments info

    if (!create_dstree_node_filename(index->settings, node->left_child, node))
    {
        fprintf(stderr,"Error in dstree_node_split.c: Could not create        \
                        filename for left child of node %s.\n", node->filename);
	return FAILURE;
    }
  
    node->right_child = create_child_node(node);
    if (node->right_child == NULL)
    {
        fprintf(stderr,"Error in dstree_node_split.c: Right child not \
                        initialized properly.\n");
	return FAILURE;
    }
    
	
    if (!node_init_segments(node->right_child,child_node_points,num_child_node_points))
    {
        fprintf(stderr,"Error in dstree_node_split.c: Could not initialize       \
                        segments for right child of node %s.\n", node->filename);
	return FAILURE;
    }
    
    node->right_child->is_leaf = 1;

    if (!create_dstree_node_filename(index->settings, node->right_child, node))
    {
        fprintf(stderr,"Error in dstree_node_split.c: Could not create        \
                        filename for right child of node %s.\n", node->filename);
	return FAILURE;
    }

    node->is_leaf = 0; //after splitting, node is no longer a leaf
  }

  return SUCCESS;
}
	
	
struct dstree_node *  create_child_node(struct dstree_node * parent) 
{


    
  struct dstree_node * child_node = dstree_leaf_node_init();

  if (child_node == NULL)
  {
        fprintf(stderr,"Error in dstree_node_split.c: Could not initialize        \
                        child node of parent %s.\n", parent->filename);
	return NULL;
  }    


  child_node->node_segment_split_policies = parent->node_segment_split_policies;
  child_node->num_node_segment_split_policies = parent->num_node_segment_split_policies;
  
  child_node->range = parent->range;

  child_node->parent = parent;

  child_node->level = parent->level + 1;

  child_node->is_leaf = 1;

  return child_node;
}


short get_hs_split_point(short * points, short from, short to, int size_points) 
{

  short *res;

  res= (short *) bsearch(&to, points, size_points,
                                       sizeof(short), compare_short);

  
  if(res != NULL) //key found,
  {
      return from;
  }
  else
  {
    return to;
  }
}



boolean is_split_policy_mean(struct node_segment_split_policy policy)
{
  
  if (policy.indicator_split_idx == 0)
    return true;
  else
    return false;
}

boolean is_split_policy_stdev(struct node_segment_split_policy policy)
{
  
  if (policy.indicator_split_idx == 1)
    return true;
  else
    return false;
  

}

ts_type range_calc(struct segment_sketch sketch, int len)
{
  ts_type mean_width = sketch.indicators[0] -sketch.indicators[1];
  ts_type stdev_upper = sketch.indicators[2];
  ts_type stdev_lower = sketch.indicators[3];

  return len * (mean_width * mean_width + stdev_upper * stdev_upper);
  
}


enum response calc_split_points (short * points, int ts_length, int segment_size )
{

  int avg_length = ts_length / segment_size;

  for (int i = 0; i < segment_size; ++i)
  {
    points[i] = (short) ((i+1) * avg_length);    
  }

  //set the last one
  points [segment_size-1] = (short) ts_length;

  return SUCCESS;
}

/*
  This function uses the vertical split points to
  initialize the the horizontal split points in hs_split_points
  The number of hs_split_points is double that of the vertical ones
*/


enum response calc_hs_split_points(short * hs_split_points,int * num_hs_split_points, short * split_points, int segment_size, int min_length)
{
  int c = 0;

  for (int i = 0; i < segment_size; i++) {
      short length = split_points[i]; //i==0
      if (i > 0)
      {
          length = (short) (split_points[i] - split_points[i - 1]);
      }
      if (length >= min_length * 2)
      {
          int start = 0;
          if (i > 0)
	  {
             start = split_points[i - 1];
	  }
          hs_split_points[c++] = (short) (start + (length / 2));
      }
      hs_split_points[c++] = split_points[i];
  }

  *num_hs_split_points = c;

  return SUCCESS;
}



