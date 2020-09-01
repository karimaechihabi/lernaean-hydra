//
//  first_buffer_layer.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/20/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
//
//  Modified by Karima Echihabi on 09/08/17 to improve memory management.
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  

#include "config.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "sax/sax.h"
#include "isax_first_buffer_layer.h"



struct first_buffer_layer * initialize_fbl(int initial_buffer_size, int number_of_buffers , 
				           unsigned long long max_total_size, isax_index *index) 
{
    struct first_buffer_layer *fbl = malloc(sizeof(first_buffer_layer));


    fbl->max_total_size = max_total_size; //+
    
    fbl->initial_buffer_size = initial_buffer_size;
    fbl->number_of_buffers = number_of_buffers;

    unsigned long long hard_buffer_size = (unsigned long long)(index->settings->sax_byte_size + index->settings->position_byte_size + index->settings->ts_byte_size) * max_total_size;
    fbl->hard_buffer = malloc(hard_buffer_size);
 
    if(fbl->hard_buffer == NULL) {
	fprintf(stderr, "Could not initialize hard buffer of size: %llu\n", hard_buffer_size);
	exit(-1);	
    } 
    
    fbl->buffers = malloc(sizeof(first_buffer_layer_node) * number_of_buffers);
    if(fbl->buffers == NULL) {
	fprintf(stderr, "Could not initialize buffers%llu\n");
	exit(-1);	
    } 

    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    
    int i;
    for (i=0; i<number_of_buffers; i++) {
        fbl->buffers[i].initialized = 0;
    }


    
    return fbl;
}


isax_node * insert_to_fbl(first_buffer_layer *fbl, isax_node_record *record,
                          root_mask_type mask, isax_index *index) {
    
    first_buffer_layer_node *current_fbl_node = &fbl->buffers[(int) mask];
    
    
    // ############ CREATE NEW FBL ###########
    if (!current_fbl_node->initialized) {
#ifdef DEBUG
        printf("*** Creating new FBL node. ***\n\n");
#endif
        current_fbl_node->initialized = 1;
        current_fbl_node->max_buffer_size = 0;
        current_fbl_node->buffer_size = 0;
        
        current_fbl_node->node = isax_root_node_init(mask, index->settings->paa_segments,
                                                     index->settings->timeseries_size);
        index->root_nodes++;
        current_fbl_node->node->is_leaf = 1;
        
        if(index->first_node == NULL)
        {
            index->first_node = current_fbl_node->node;
            current_fbl_node->node->next = NULL;
            current_fbl_node->node->previous = NULL;
        }
        else
        {
            isax_node * prev_first = index->first_node;
            index->first_node = current_fbl_node->node;
            index->first_node->next = prev_first;
            prev_first->previous = current_fbl_node->node;
        }
        
        
    }  

    if (current_fbl_node->buffer_size >= current_fbl_node->max_buffer_size) {
        
        if(current_fbl_node->max_buffer_size == 0) {
            current_fbl_node->max_buffer_size = fbl->initial_buffer_size;

            current_fbl_node->ts_buffer = malloc(sizeof(ts_type *) * 
                                                 current_fbl_node->max_buffer_size);
            current_fbl_node->sax_buffer = malloc(sizeof(sax_type *) * 
                                                  current_fbl_node->max_buffer_size);
            current_fbl_node->position_buffer = malloc(sizeof(file_position_type *) * current_fbl_node->max_buffer_size);   
        }
        else {
            current_fbl_node->max_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            current_fbl_node->ts_buffer = realloc(current_fbl_node->ts_buffer, 
                                                  sizeof(ts_type *) *  
                                                  current_fbl_node->max_buffer_size);
            current_fbl_node->sax_buffer = realloc(current_fbl_node->sax_buffer, 
                                                   sizeof(sax_type *) * 
                                                   current_fbl_node->max_buffer_size);  
            current_fbl_node->position_buffer = realloc(current_fbl_node->position_buffer, sizeof(file_position_type *) * current_fbl_node->max_buffer_size);
        }
    }
    
    if (current_fbl_node->sax_buffer == NULL) {
        fprintf(stderr, "error: Could not allocate memory for sax representations in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }
    if (current_fbl_node->ts_buffer == NULL) {
        fprintf(stderr, "error: Could not allocate memory for ts representations in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }
    /* START+*/
    if (current_fbl_node->position_buffer == NULL) {
        fprintf(stderr, "error: Could not allocate memory for pos representations in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }    
    /* END+*/


     /* START+ */
    current_fbl_node->sax_buffer[current_fbl_node->buffer_size] = (sax_type *) fbl->current_record;
    memcpy((void *) (fbl->current_record), (void *) (record->sax), index->settings->sax_byte_size);
    fbl->current_record += index->settings->sax_byte_size;

    current_fbl_node->ts_buffer[current_fbl_node->buffer_size] = (ts_type *) fbl->current_record;
    memcpy((void *) (fbl->current_record), (void *) (record->ts), index->settings->ts_byte_size);
    fbl->current_record += index->settings->ts_byte_size;

    current_fbl_node->position_buffer[current_fbl_node->buffer_size] = (file_position_type *) fbl->current_record;
    //*(fbl->current_record) = (file_position_type) record->position;
    memcpy(fbl->current_record, &(record->position), index->settings->position_byte_size);
    fbl->current_record += index->settings->position_byte_size;    

    
     /* END+ */

	
#ifdef DEBUG
    printf("*** Added to node ***\n\n");
#endif
    current_fbl_node->buffer_size++;
    /* START+ */
    fbl->current_record_index++;
    /* END+ */
    
    return current_fbl_node->node;
}


enum response flush_fbl(first_buffer_layer *fbl, isax_index *index) {
    
    
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
    printf("\n");
    fflush(stdout);
    unsigned long long i=1;
    fprintf(stdout, "\x1b[31mFlushing: \x1b[36m00.00%%\x1b[0m");
#endif
#if VERBOSE_LEVEL == 1
    printf("Flushing...\n");
#endif
#else
    printf("*** FLUSHING ***\n\n");
#endif
    unsigned long long c = 1;
    unsigned long long j;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    for (j=0; j<fbl->number_of_buffers; j++) {
        
        first_buffer_layer_node *current_fbl_node = &index->fbl->buffers[j];
        
        if (!current_fbl_node->initialized) {
            continue;
        }
        
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        fprintf(stdout,"\r\x1b[31mFlushing: \x1b[36m%2.2lf%%\x1b[0m", ((float)c/(float)index->root_nodes)*100);
        c++;
        fflush(stdout);
#endif
#endif
        unsigned long long i;
        if (current_fbl_node->buffer_size > 0) {
            // For all records in this buffer 
            for (i=0; i<current_fbl_node->buffer_size; i++) {

	        /*  START+ */ 
  	        r->sax = (sax_type *) current_fbl_node->sax_buffer[i];
                r->ts = (ts_type *) current_fbl_node->ts_buffer[i];
                r->position = (file_position_type) current_fbl_node->position_buffer[i];
	        /*  END+ */
		
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            
            // flush index node
            flush_subtree_leaf_buffers(index, current_fbl_node->node);
            
            // clear FBL records moved in LBL buffers
            free(current_fbl_node->ts_buffer);
            free(current_fbl_node->sax_buffer);
            free(current_fbl_node->position_buffer);
            // clear records read from files (free only prev sax buffers)
            isax_index_clear_node_buffers(index, current_fbl_node->node, 1, 0);

	    
            // Set to 0 in order to re-allocate original space for buffers.
            current_fbl_node->buffer_size = 0;
            current_fbl_node->max_buffer_size = 0;
        }
        
    }
    free(r);
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
    printf("\n");
#endif
#endif

    return SUCCESS;
}

void destroy_fbl(first_buffer_layer *fbl, int use_index)
{
    if (use_index)
      free(fbl->hard_buffer);
    
    free(fbl->buffers);
    free(fbl);
}

