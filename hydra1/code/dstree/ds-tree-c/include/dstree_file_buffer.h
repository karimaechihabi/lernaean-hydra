//
//  dstree_file_buffer.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef dstree_dstree_file_buffer_h
#define dstree_dstree_file_buffer_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dstree_node.h"
#include "dstree_index.h"


struct dstree_file_buffer {

  struct dstree_node * node; //the buffer points back to its node
  struct dstree_file_map * position_in_map; //the buffer points back to its position in file map

  ts_type ** buffered_list;
  unsigned int disk_count; //  by default
  int buffered_list_size;   //number of series currently stored in this buffer

  boolean in_disk; //false by default
  boolean do_not_flush;
};

enum response dstree_file_buffer_init(struct dstree_node *node);
enum response flush_buffer_to_disk(struct dstree_index *index, struct dstree_node *node);
enum response clear_file_buffer(struct dstree_index *index, struct dstree_node * node);
enum response delete_file_buffer(struct dstree_index * index,struct dstree_node * node);
//enum response get_all_time_series_in_node(struct dstree_index * index, struct dstree_node * node);
ts_type ** get_all_time_series_in_node(struct dstree_index * index, struct dstree_node * node);



#endif				   
