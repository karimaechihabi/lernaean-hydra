//
//  dstree_node_split.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef dstreelib_dstree_node_split_h
#define dstreelib_dstree_node_split_h
#include "../config.h"
#include "../globals.h"
#include "dstree_index.h"

struct segment_sketch{
  ts_type * indicators;
  int num_indicators;
}; 

struct node_segment_split_policy {
  ts_type indicator_split_value;
  int indicator_split_idx; //Set Idx = 0 for mean_based split and Idx = 1 for stdev_based split 
};

struct  node_split_policy {
  short split_from;
  short split_to;
  struct node_segment_split_policy curr_node_segment_split_policy;

  int indicator_split_idx;
  ts_type indicator_split_value;

  struct segment_sketch series_segment_sketch;
   
};

enum response split_node (struct dstree_index * index, struct dstree_node * node, short * child_node_points, int num_child_node_points);
enum response node_segment_sketch_update_sketch(struct segment_sketch * node_sketch, ts_type * series, int fromIdx, int toIdx);

enum response series_segment_sketch_do_sketch(struct segment_sketch * series_segment_sketch, ts_type * series, int fromIdx, int toIdx);

enum response mean_node_segment_split_policy_split(struct node_segment_split_policy * policy, struct segment_sketch sketch, struct segment_sketch * ret);
enum response stdev_node_segment_split_policy_split(struct node_segment_split_policy * policy, struct segment_sketch sketch, struct segment_sketch * ret);

boolean is_split_policy_mean(struct node_segment_split_policy policy);
boolean is_split_policy_stdev(struct node_segment_split_policy policy);
struct dstree_node *  create_child_node(struct dstree_node * parent);
ts_type range_calc(struct segment_sketch sketch, int len);
boolean node_split_policy_route_to_left (struct dstree_node * node, ts_type * series);
short get_hs_split_point(short * points, short from, short to, int size_points);


#endif
