//
//  vaplus_file_buffer_manager.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef vaplus_vaplus_file_buffer_manager_h
#define vaplus_vaplus_file_buffer_manager_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vaplus_index.h"
#include "vaplus_file_buffer.h"

struct vaplus_file_buffer_manager {
  struct vaplus_file_map * file_map;
  struct vaplus_file_map * file_map_tail;  
  
  unsigned long max_buffered_size;
  long current_count;
  long batch_remove_size;

  unsigned long chunk_size;
  
  int current_record_index;
  int max_record_index;

  char * ts_mem_array;
  char * current_ts_record;

  char * dft_mem_array;
  char * current_dft_record;
  
  char * approx_mem_array;
  char * current_approx_record;
  
  int file_map_size;
};


struct vaplus_file_map{
  struct vaplus_file_buffer * file_buffer;
  struct vaplus_file_map * next;
  struct vaplus_file_map * prev;  
};

enum response init_file_buffer_manager(struct vaplus_index *index);
enum response set_buffered_memory_size(struct vaplus_index * index);
enum response get_file_buffer(struct vaplus_index *index, struct vaplus_node *node);
enum response save_all_buffers_to_disk(struct vaplus_index *index);
enum response add_file_buffer_to_map(struct vaplus_index * index, struct vaplus_node *node);  

#endif
