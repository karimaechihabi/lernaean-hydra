//
//  isax_node.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/10/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isaxlib_isax_node_h
#define isaxlib_isax_node_h

struct isax_node;
#include "config.h"
#include "globals.h"    
#include "isax_node_record.h"

typedef struct isax_node_split_data {
    int splitpoint;
    sax_type * split_mask;
    int depth;
} isax_node_split_data;

typedef struct isax_node {
    // General
    int leaf_size;
    
    struct isax_node *next;
    struct isax_node *previous;
    root_mask_type mask;
    struct isax_node *parent;
    
    char has_full_data_file;
    
    sax_type * isax_values;
    sax_type * isax_cardinalities;
    
    
    // If is leaf
    unsigned char is_leaf; 
    char * filename;
    
    
    // Buffer stuff
    sax_type **prev_sax_buffer;
    sax_type **sax_buffer;
    ts_type **ts_buffer;
    file_position_type *pos_buffer;
    
    int prev_sax_buffer_size;
    int sax_buffer_size;
    int ts_buffer_size;
    
    int max_sax_buffer_size;
    int max_ts_buffer_size;
    int max_prev_sax_buffer_size;
    
    // If is intermediate
    struct isax_node_split_data *split_data;
    struct isax_node *left_child;
    struct isax_node *right_child;

    int level;
    
} isax_node;

isax_node * isax_root_node_init(root_mask_type mask, int sax_size, int ts_size);
isax_node * isax_leaf_node_init(int sax_size, int ts_size);
#endif
