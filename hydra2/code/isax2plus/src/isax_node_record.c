//
//  isax_node_record.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/11/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "isax_node_record.h"

isax_node_record * isax_node_record_init(int sax_size, int ts_size)
{
    isax_node_record *node = malloc(sizeof(isax_node_record));
    if(node == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record.\n");
        return NULL;
    }
    node->sax = malloc(sizeof(sax_type) * sax_size);
    if(node->sax == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record's sax representation.\n");
        return NULL;
    }
    
    node->ts = malloc(sizeof(ts_type) * ts_size);
    if(node->ts == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record's raw time series.\n");
        return NULL;
    }
    
    /*node->position = malloc(sizeof(file_position_type));
    if(node->position == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record's file position.\n");
        return NULL;
    }*/
    node->position = -1;
    
    return node;
}

void isax_node_record_destroy(isax_node_record *node)
{
    free(node->sax);
    free(node->ts);
    //free(node->position);
    free(node);
}
