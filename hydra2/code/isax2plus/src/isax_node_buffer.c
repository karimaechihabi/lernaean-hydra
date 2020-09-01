//
//  isax_node_buffer.c
//  aisax
//
//  Created by Kostas Zoumpatianos on 4/6/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "globals.h"
#include "isax_node_buffer.h"
#include "isax_node.h"
#include "isax_node_record.h"

enum response add_to_node_buffer(isax_node *node, isax_node_record *record, 
                                 int sax_size, int ts_size, int initial_buffer_size)
{
    
    if (!node->is_leaf) {
        fprintf(stderr,"sanity error: Adding to non-leaf node!\n");
    }
    
    // COPY DATA TO BUFFER
    
    char full_add;
    
    // Add both sax representation and time series in buffers
    if(record->sax != NULL && record->ts != NULL)
    {
        full_add = 1;
        //fprintf(stderr,"BUF\t%lld\n", record->position);    
    }
    // Only add sax representation which comes from a parent
    else if(record->sax != NULL && record->ts == NULL)
    {
        full_add = 0;
    }
    else
    {
        fprintf(stderr, "error: no SAX representation received as input!");
        exit(-1);
    }
    
    if(full_add) {
        // Sax Buffer
        if (node->max_sax_buffer_size == 0)
        {
            // ALLOCATE MEMORY.
            node->max_sax_buffer_size = initial_buffer_size;
            node->sax_buffer = (sax_type**) malloc(sizeof(sax_type*) * node->max_sax_buffer_size);
            
            if( node->sax_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node sax buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }
            // END MEMORY ALLOCATION
        }
        else if(node->max_sax_buffer_size <= node->sax_buffer_size)
        {
            // REALLOCATE MEMORY.
            node->max_sax_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            node->sax_buffer = (sax_type**) realloc(node->sax_buffer, 
                                                    sizeof(sax_type*) * node->max_sax_buffer_size);
            
            if( node->sax_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node sax buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }
            // END MEMORY REALLOCATION
        }
        
        // Time series Buffer
        if (node->max_ts_buffer_size == 0)
        {
            // ALLOCATE MEMORY.
            node->max_ts_buffer_size = initial_buffer_size;
            node->ts_buffer = (ts_type**) malloc(sizeof(ts_type*) * node->max_ts_buffer_size);
            node->pos_buffer = (file_position_type*) malloc(sizeof(file_position_type) * node->max_ts_buffer_size);
            
            
            if( node->ts_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node ts buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            } 
            if( node->pos_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node position buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }
        }
        else if(node->max_ts_buffer_size <= node->ts_buffer_size)
        {
            // REALLOCATE MEMORY.
#ifdef HARDLBL
            int prev_ts_buffer_size = node->max_ts_buffer_size;
#endif
            node->max_ts_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            node->ts_buffer = (ts_type**) realloc(node->ts_buffer, 
                                                  sizeof(ts_type*) * node->max_ts_buffer_size);
            node->pos_buffer = (file_position_type*) realloc(node->pos_buffer,
                                                             sizeof(file_position_type) * node->max_ts_buffer_size);
            if( node->ts_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node ts buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            } 
            if( node->pos_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node position buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }
            // END MEMORY REALLOCATION
        }
    }
    else {
        if (node->max_prev_sax_buffer_size == 0)
        {
            // ALLOCATE MEMORY.
            node->max_prev_sax_buffer_size = initial_buffer_size;
            node->prev_sax_buffer = (sax_type**) malloc(sizeof(sax_type*) * node->max_prev_sax_buffer_size);
            if( node->prev_sax_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node sax buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }                       
            // END MEMORY ALLOCATION
        }
        else if(node->max_prev_sax_buffer_size <= node->prev_sax_buffer_size)
        {
            // REALLOCATE MEMORY.
            node->max_prev_sax_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            node->prev_sax_buffer = (sax_type**) realloc(node->prev_sax_buffer, 
                                                         sizeof(sax_type*) * node->max_prev_sax_buffer_size);
            if( node->prev_sax_buffer  == NULL) {
                fprintf(stderr,"error: could not allocate memory for node sax buffer.\n");
                return OUT_OF_MEMORY_FAILURE;
            }            
        }
    }
    
    
    // Add data in buffers
    if(full_add)
    {
        node->sax_buffer[node->sax_buffer_size] = record->sax;
        node->sax_buffer_size++;
        node->ts_buffer[node->ts_buffer_size] = record->ts;
        node->pos_buffer[node->ts_buffer_size] = &(record->position);
        node->ts_buffer_size++;
    }
    else 
    {
        node->prev_sax_buffer[node->prev_sax_buffer_size] = record->sax;
        node->prev_sax_buffer_size++;
    }
        
    node->leaf_size++;
    
    return SUCCESS;
}



enum response flush_node_buffer(isax_node *node, isax_index_settings *settings) {
    if(node->sax_buffer_size > 0 || node->ts_buffer_size > 0 || node->prev_sax_buffer_size > 0)
    {
#ifdef DEBUG
        printf("*** Flushing: node with %d leaf records in %s. *** \n", node->leaf_size, node->filename);
        printf("\tBuffers breakdown:\n\t------------------\n\
               \tSAX:\t\t%d\n\tTS:\t\t%d\n\tPREV:\t\t%d\n\n", node->sax_buffer_size, 
               node->ts_buffer_size, node->prev_sax_buffer_size);
#endif
        int i;
        
        if(node->filename == NULL)
        {   
            fprintf(stderr, "I do not have a node filename");
            exit(-1);
            return FAILURE;
        }
        
        if(node->ts_buffer_size > 0)
        {
            char * fname = malloc(sizeof(char) * (strlen(node->filename) + 4));
            strcpy(fname, node->filename);
            strcat(fname, ".ts");

            COUNT_PARTIAL_RAND_OUTPUT
   	    COUNT_PARTIAL_OUTPUT_TIME_START	  
            FILE *ts_file = fopen(fname, "a+");
            
            for (i=0; i<node->ts_buffer_size; i++) {
                COUNT_PARTIAL_SEQ_OUTPUT
                COUNT_PARTIAL_SEQ_OUTPUT
                fwrite(node->ts_buffer[i], sizeof(ts_type), settings->timeseries_size, ts_file);
                fwrite(&node->pos_buffer[i], sizeof(file_position_type), 1, ts_file);
                //file_position_type a = -3;
                //fwrite(&a, sizeof(file_position_type), 1, ts_file);
            }
            
            free(fname);
            fclose(ts_file);
            COUNT_PARTIAL_OUTPUT_TIME_END

        }
        
        if(node->sax_buffer_size > 0)
        {
            char * fname = malloc(sizeof(char) * (strlen(node->filename) + 5));
            strcpy(fname, node->filename);
            strcat(fname, ".sax");
            COUNT_PARTIAL_RAND_OUTPUT
   	    COUNT_PARTIAL_OUTPUT_TIME_START	  
            FILE *sax_file = fopen(fname, "a+");
            
            for (i=0; i<node->sax_buffer_size; i++) {
                fwrite(node->sax_buffer[i], sizeof(sax_type), settings->paa_segments, sax_file);
                //sax_print(node->sax_buffer[i], index->settings->paa_segments, index->settings->sax_bit_cardinality);
            }
            
            free(fname);
            fclose(sax_file);
            COUNT_PARTIAL_OUTPUT_TIME_END
        }
        
        if(node->prev_sax_buffer_size > 0)
        {
            char * fname = malloc(sizeof(char) * (strlen(node->filename) + 5));
            strcpy(fname, node->filename);
            strcat(fname, ".pre");
            COUNT_PARTIAL_RAND_OUTPUT
	    COUNT_OUTPUT_TIME_START
            FILE *psax_file = fopen(fname, "a+");
            
            for (i=0; i<node->prev_sax_buffer_size; i++) {
                COUNT_PARTIAL_SEQ_OUTPUT
		fwrite(node->prev_sax_buffer[i], sizeof(sax_type), settings->paa_segments, psax_file);
            }
            
            free(fname);
            fclose(psax_file);
            COUNT_OUTPUT_TIME_END
        }
    }
    return SUCCESS;
}

enum response clear_node_buffer(isax_node *node, const char full_clean) {
#ifdef DEBUG
    fprintf(stdout, "*** Cleaning (%d): %s\n\tLeaf size: %d\n\tBuffer sizes:\n\t\tSAX: %d\n\t\tPREV: %d\n\t\tTS: %d\n", 
            node->is_leaf,
            node->filename, node->leaf_size, 
            node->sax_buffer_size, node->prev_sax_buffer_size,
            node->ts_buffer_size);
#endif
    
    int i;
    
    for (i=0; i<node->prev_sax_buffer_size; i++) {
        free(node->prev_sax_buffer[i]);
    }
    
    if (full_clean) {
        for (i=0; i<node->sax_buffer_size; i++) {
            free(node->sax_buffer[i]);
        }
        for (i=0; i<node->ts_buffer_size; i++) {
            free(node->ts_buffer[i]);
        }
    }
    
    node->sax_buffer_size = 0;
    node->ts_buffer_size = 0;
    node->prev_sax_buffer_size = 0;
    
    if (node->max_sax_buffer_size > 0)
    {
        free(node->sax_buffer);
        node->max_sax_buffer_size = 0;
    }
    if (node->max_ts_buffer_size > 0)
    {
        free(node->ts_buffer);
        free(node->pos_buffer);
        node->max_ts_buffer_size = 0;
    }
    if (node->max_prev_sax_buffer_size > 0)
    {
        free(node->prev_sax_buffer);
        node->max_prev_sax_buffer_size = 0;
    }
    
    return SUCCESS;
}
