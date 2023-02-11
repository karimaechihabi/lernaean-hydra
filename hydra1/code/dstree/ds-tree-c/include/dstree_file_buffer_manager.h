//
//  dstree_file_buffer_manager.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef dstree_dstree_file_buffer_manager_h
#define dstree_dstree_file_buffer_manager_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dstree_index.h"
#include "dstree_file_buffer.h"

struct dstree_file_buffer_manager {
  struct dstree_file_map * file_map;
  struct dstree_file_map * file_map_tail;  
  
  unsigned long max_buffered_size;
  long current_count;
  long batch_remove_size;

  char *mem_array;
  char *current_record;
  int current_record_index;
  int max_record_index;
  
  int file_map_size;
};


struct dstree_file_map{
  struct dstree_file_buffer * file_buffer;
  struct dstree_file_map * next;
  struct dstree_file_map * prev;  
};

enum response init_file_buffer_manager(struct dstree_index *index);
enum response set_buffered_memory_size(struct dstree_index * index);
enum response get_file_buffer(struct dstree_index *index, struct dstree_node *node);
enum response save_all_buffers_to_disk(struct dstree_index *index);
enum response add_file_buffer_to_map(struct dstree_index * index, struct dstree_node *node);  

#endif
