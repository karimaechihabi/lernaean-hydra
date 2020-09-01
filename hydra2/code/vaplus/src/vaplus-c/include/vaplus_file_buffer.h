//
//  vaplus_file_buffer.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef vaplus_vaplus_file_buffer_h
#define vaplus_vaplus_file_buffer_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vaplus_node.h"
#include "vaplus_index.h"


struct vaplus_file_buffer {

  struct vaplus_node * node; //the buffer points back to its node
  struct vaplus_file_map * position_in_map; //the buffer points back to its position in file map

  //ts_type ** buffered_list;
  struct vaplus_record * buffered_list;
  
  unsigned int disk_count; //  by default
  int buffered_list_size;   //number of series currently stored in this buffer

  boolean in_disk; //false by default
  boolean do_not_flush;
};

enum response vaplus_file_buffer_init(struct vaplus_node *node);
enum response flush_buffer_to_disk(struct vaplus_index *index, struct vaplus_node *node, boolean full_flush);
enum response clear_file_buffer(struct vaplus_index *index, struct vaplus_node * node);
enum response delete_file_buffer(struct vaplus_index * index,struct vaplus_node * node);
//enum response get_all_time_series_in_node(struct vaplus_index * index, struct vaplus_node * node);
struct vaplus_record * get_all_time_series_in_node(struct vaplus_index * index, struct vaplus_node * node);
struct vaplus_record * get_all_records_in_node(struct vaplus_index * index, struct vaplus_node * node);



#endif				   
