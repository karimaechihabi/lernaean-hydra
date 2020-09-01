//
//  isax_node.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/10/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "globals.h"
#include "isax_node.h"

/**
 This function initializes an isax root node.
 */
isax_node * isax_root_node_init(root_mask_type mask, int sax_size, int ts_size) 
{
    isax_node *node = isax_leaf_node_init(sax_size, ts_size);
    node->mask = mask;
    return node;
}

/**
 This function initalizes an isax leaf node.
 */
isax_node * isax_leaf_node_init(int sax_size, int ts_size) 
{
    COUNT_NEW_NODE
    isax_node *node = malloc(sizeof(isax_node));
    if(node == NULL) {
        fprintf(stderr,"error: could not allocate memory for new .\n");
        return NULL;
    }
    node->has_full_data_file = 0;
    
    node->isax_values = NULL;
    node->isax_cardinalities = NULL;
    
    node->right_child = NULL;
    node->left_child = NULL;
    node->parent = NULL;
    node->next = NULL;
    node->leaf_size = 0;
    node->filename = NULL;
    node->previous = NULL;
    node->split_data = NULL;
    
    node->ts_buffer_size = 0;
    node->sax_buffer_size = 0;
    node->prev_sax_buffer_size = 0;
    
    node->max_sax_buffer_size = 0;
    node->max_ts_buffer_size = 0;
    node->max_prev_sax_buffer_size = 0;

    node->level = 0;
    
    return node;
}
