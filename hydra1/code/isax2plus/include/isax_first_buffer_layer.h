//
//  first_buffer_layer.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/20/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
//
//  Modified by Karima Echihabi on 09/08/17 to improve memory management.
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  

#ifndef isaxlib_first_buffer_layer_h
#define isaxlib_first_buffer_layer_h
#include "config.h"
#include "globals.h"
#include "isax_node.h"
#include "isax_index.h"
#include "isax_node_record.h"


typedef struct first_buffer_layer_node {
    isax_node *node;
    
    /*
    sax_type *sax_buffer;
    ts_type *ts_buffer;
    file_position_type *position_buffer;
    */
    /* START+*/
    sax_type ** sax_buffer;
    ts_type ** ts_buffer;
    file_position_type ** position_buffer;
    /* END+*/
  
    unsigned long long max_buffer_size;
    unsigned long long buffer_size;
    
    int initialized;
    
} first_buffer_layer_node;

typedef struct first_buffer_layer {
    unsigned long long number_of_buffers;
    unsigned long long initial_buffer_size;

    /* START+*/
    int max_total_size;
    int current_record_index;
    int max_record_index;
    char *current_record;
    char *hard_buffer;
    /* END+*/
  
    first_buffer_layer_node *buffers;
 
    //this buffer is used to store internal node at the finalization
    //step as allocating memory for each internal node and releasing it
    //is not efficient.

    //isax_node_record *internal_node_buffer;
    //int internal_node_buffer_size;

} first_buffer_layer;

first_buffer_layer * initialize_fbl(int initial_buffer_size, int max_size,
				    unsigned long long max_total_size, isax_index *index);
void destroy_fbl(first_buffer_layer *fbl, int use_index);
isax_node * insert_to_fbl(first_buffer_layer *fbl, isax_node_record *record, 
                            root_mask_type mask, isax_index *index);
enum response flush_fbl(first_buffer_layer *fbl, isax_index *index);
#endif
