//
//  isax_node_record.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/11/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>

#include "ads/isax_node_record.h"

isax_node_record * isax_node_record_init(int sax_size, int ts_size, 
                                         enum insertion_mode mode)
{
    isax_node_record *record = malloc(sizeof(isax_node_record));
    if(record == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record.\n");
        return NULL;
    }
    record->sax = malloc(sizeof(sax_type) * sax_size);
    if(record->sax == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record's sax representation.\n");
        return NULL;
    }
    
    record->ts = malloc(sizeof(ts_type) * ts_size);
    if(record->ts == NULL) {
        fprintf(stderr,"error: could not allocate memory for new record's raw time series.\n");
        return NULL;
    }
    
    record->insertion_mode = mode;
    
    return record;
}

void isax_node_record_destroy(isax_node_record *node)
{
    free(node->sax);
    free(node->ts);
    free(node);
}
