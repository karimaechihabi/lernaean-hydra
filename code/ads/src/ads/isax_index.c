//
//  isax_index.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/10/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


/* 
 ============= NOTES: =============
 Building a mask for the following sax word:
 SAX:
 00
 00
 01
 00
 11
 01
 10
 11
 
 How to build a mask for the FIRST bit of this word (root),
 I use do:
 R = 00000000
 IF(00 AND 10) R = R OR 10000000
 IF(00 AND 10) R = R OR 01000000
 IF(01 AND 10) R = R OR 00100000
 IF(00 AND 10) R = R OR 00010000
 IF(11 AND 10) R = R OR 00001000
 IF(01 AND 10) R = R OR 00000100
 IF(10 AND 10) R = R OR 00000010
 IF(11 AND 10) R = R OR 00000001
 result: R = 00001011
 

 *** IN ORDER TO CALCULATE LOCATION BITMAP MASKS ***: 
 
 m = 2^NUMBER_OF_MASKS     (e.g for 2^3=8 = 100)
 m>> for the second mask   (e.g.            010)
 m>>>> for the third ...   (e.g.            001)
*/
#ifdef VALUES
#include <values.h>
#endif

#include "../../config.h"
#include "../../globals.h"
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ads/sax/sax.h"
#include "ads/sax/ts.h"
#include "ads/isax_index.h"
#include "ads/isax_node.h"
#include "ads/isax_node_buffer.h"
#include "ads/isax_node_split.h"
#include "ads/isax_first_buffer_layer.h"

 
/**
 This function initializes the settings of an isax index
 */
isax_index_settings * isax_index_settings_init(const char * root_directory, int timeseries_size, 
                                               int paa_segments, int sax_bit_cardinality, 
                                               int max_leaf_size, int min_leaf_size,
                                               int initial_leaf_buffer_size,
                                               unsigned long long max_total_buffer_size, int initial_fbl_buffer_size,
                                               int total_loaded_leaves, int tight_bound, int aggressive_check, int new_index)
{
    int i;
    isax_index_settings *settings = malloc(sizeof(isax_index_settings));
    if(settings == NULL) {
        fprintf(stderr,"error: could not allocate memory for index settings.\n");
        return NULL;
    }
    
    if(new_index) {
		if(chdir(root_directory) == 0)
		{
			fprintf(stderr, "WARNING! Target index directory already exists. Please delete or choose a new one.\n");
		}
		mkdir(root_directory, 0777);
        settings->max_total_full_buffer_size = max_total_buffer_size;
        settings->initial_fbl_buffer_size = initial_fbl_buffer_size;
    }
    else {
    	if(chdir(root_directory) != 0)
		{
			fprintf(stderr, "WARNING! Target index directory does not exist!\n");
		}
    	else {
    		chdir("../");
    	}
        settings->max_total_full_buffer_size = max_total_buffer_size;
        settings->initial_fbl_buffer_size = initial_fbl_buffer_size;
        //settings->max_total_full_buffer_size = 0;
        //settings->initial_fbl_buffer_size = 0;
    }

    if(paa_segments > (int)(8 * (int)sizeof(root_mask_type))){
        fprintf(stderr,"error: Too many paa segments. The maximum value is %zu.\n", 
                8 * sizeof(root_mask_type));
        return NULL;
    }
        
    if(initial_leaf_buffer_size < max_leaf_size)
    {
        fprintf(stderr,"error: Leaf buffers should be at least as big as leafs.\n");
        return NULL;
    }
    settings->total_loaded_leaves = total_loaded_leaves;
    settings->root_directory = root_directory;
    settings->raw_filename = NULL;

    settings->timeseries_size = timeseries_size;
    settings->paa_segments = paa_segments;
    settings->ts_values_per_paa_segment = timeseries_size/paa_segments;
    settings->max_leaf_size = max_leaf_size;
    settings->min_leaf_size = min_leaf_size;
    settings->initial_leaf_buffer_size = initial_leaf_buffer_size;

	
	settings->tight_bound = tight_bound;
    settings->aggressive_check = aggressive_check;
	
    settings->sax_byte_size = (sizeof(sax_type) * paa_segments);
    settings->ts_byte_size = (sizeof(ts_type) * timeseries_size);
    settings->position_byte_size = sizeof(file_position_type);
    
    settings->full_record_size = settings->sax_byte_size 
                                 + settings->position_byte_size 
                                 + settings->ts_byte_size;
    settings->partial_record_size = settings->sax_byte_size 
                                    + settings->position_byte_size;
    
    settings->sax_bit_cardinality = sax_bit_cardinality;
    settings->sax_alphabet_cardinality = pow(2, sax_bit_cardinality);
    
   settings->max_sax_cardinalities = malloc(sizeof(sax_type) * settings->paa_segments);
   
   for(i=0; i<settings->paa_segments;i++)
     settings->max_sax_cardinalities[i] = settings->sax_bit_cardinality;
	
    //settings->mindist_sqrt = sqrtf((float) settings->timeseries_size /
    //                               (float) settings->paa_segments);
    settings->mindist_sqrt = ((float) settings->timeseries_size /
                                   (float) settings->paa_segments);
    settings->root_nodes_size = pow(2, settings->paa_segments);
    
    // SEGMENTS * (CARDINALITY)
    float c_size = ceil(log10(settings->sax_alphabet_cardinality + 1));
    settings->max_filename_size = settings->paa_segments * 
                                  ((c_size * 2) + 2)
                                  + 5 + strlen(root_directory);
    
    
    if(paa_segments > sax_bit_cardinality)
    {
        settings->bit_masks = malloc(sizeof(root_mask_type) * (paa_segments+1));
        if(settings->bit_masks == NULL) {
            fprintf(stderr,"error: could not allocate memory for bit masks.\n");
            return NULL;
        }
        
        for (; paa_segments>=0; paa_segments--)
        {
            settings->bit_masks[paa_segments] = pow(2, paa_segments);
        }
    }
    else
    {
        settings->bit_masks = malloc(sizeof(root_mask_type) * (sax_bit_cardinality+1));
        if(settings->bit_masks == NULL) {
            fprintf(stderr,"error: could not allocate memory for bit masks.\n");
            return NULL;
        }
        
        for (; sax_bit_cardinality>=0; sax_bit_cardinality--)
        {
            settings->bit_masks[sax_bit_cardinality] = pow(2, sax_bit_cardinality);
        }
    }
    
    if(new_index) {
    settings->max_total_buffer_size = (unsigned long long) ((float)(settings->full_record_size/
                                       (float)settings->partial_record_size) * settings->max_total_full_buffer_size);
    }
    else {
    settings->max_total_buffer_size = settings->max_total_full_buffer_size;
    }
    
    
    return settings;
}


/**
 This function initializes an isax index
 @param isax_index_settings *settings
 @return isax_index
 */
isax_index * isax_index_init(isax_index_settings *settings)
{
    isax_index *index = malloc(sizeof(isax_index));
    if(index == NULL) {
        fprintf(stderr,"error: could not allocate memory for index structure.\n");
        return NULL;
    }
    index->memory_info.mem_tree_structure = 0;
    index->memory_info.mem_data = 0;
    index->memory_info.mem_summaries = 0;
    index->memory_info.disk_data_full = 0;
    index->memory_info.disk_data_partial = 0;
    
    index->settings = settings;
    index->first_node = NULL;
    index->fbl = initialize_fbl(settings->initial_fbl_buffer_size,
                                pow(2, settings->paa_segments), 
                                settings->max_total_buffer_size, index);
    char *sax_filename = malloc((strlen(settings->root_directory) + 15) * sizeof(char));
    sax_filename = strcpy(sax_filename, settings->root_directory);
    sax_filename = strcat(sax_filename, "isax_file.sax");


    if( access( sax_filename, F_OK ) != -1 ) {
        COUNT_PARTIAL_RAND_INPUT
        COUNT_PARTIAL_INPUT_TIME_START
    	index->sax_file = fopen(sax_filename, "rb");
        COUNT_PARTIAL_INPUT_TIME_END	
    } else {
        COUNT_PARTIAL_RAND_OUTPUT
        COUNT_PARTIAL_OUTPUT_TIME_START	  
    	index->sax_file = fopen(sax_filename, "w+b");
        COUNT_PARTIAL_OUTPUT_TIME_END	  	
    }

    free(sax_filename);
    index->sax_cache = NULL;
    
    index->total_records = 0;
    index->loaded_records = 0;

    index->root_nodes = 0;
    index->allocated_memory = 0;
    index->has_wedges = 0;
    //index->locations = malloc(sizeof(int) * settings->timeseries_size);

    index->answer = malloc(sizeof(ts_type) * settings->timeseries_size);

    init_stats(index);
    
    return index;
}


/** 
 This function destroys an index.
 @param isax_index *index
 @param isax_ndoe *node
 */
void isax_index_destroy(isax_index *index, isax_node *node)
{
    if (node == NULL) {
        free(index->answer);
    	free(index->settings->bit_masks);
    	free(index->settings->raw_filename);
    	free(index->settings->max_sax_cardinalities);
    	free(index->settings);

		// TODO: OPTIMIZE TO FLUSH WITHOUT TRAVERSAL!
        isax_node *subtree_root = index->first_node;
        
        while (subtree_root != NULL) 
        {
            isax_node *next =  subtree_root->next;
            isax_index_destroy(index, subtree_root);
            subtree_root = next;
        }
        destroy_fbl(index->fbl);
		#ifdef CLUSTERED
			free(index->locations);
		#endif
        fclose(index->sax_file);
        if(index->sax_cache != NULL)
            free(index->sax_cache);
        free(index);
    }
    else {
        // Traverse tree
        if(!node->is_leaf) {
            isax_index_destroy(index, node->right_child);
            isax_index_destroy(index, node->left_child);
        }
        
        if(node->split_data != NULL)
        {
            free(node->split_data->split_mask);
            free(node->split_data);
        }
        if(node->filename != NULL)
        {
            free(node->filename);
        }
        if(node->isax_cardinalities != NULL)
        {
            free(node->isax_cardinalities);
        }
        if(node->isax_values != NULL)
        {
            free(node->isax_values);
        }
        if(node->buffer != NULL)
        {
            destroy_node_buffer(node->buffer);
        }
    
        free(node);
    }
}



/**
 This function inserts an isax_node_record in an isax index with an fbl
 @param isax_index *index
 @param isax_node_record *record
 */
root_mask_type isax_fbl_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos)
{
	COUNT_PARTIAL_SEQ_OUTPUT
	COUNT_PARTIAL_OUTPUT_TIME_START
	fwrite(sax, index->settings->sax_byte_size, 1, index->sax_file);
	COUNT_PARTIAL_OUTPUT_TIME_END
	  
    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    
    // TODO: Create INSERTION SHORT AND BINARY SEARCH METHODS.
    root_mask_type first_bit_mask = 0x00;
    CREATE_MASK(first_bit_mask, index, sax);
    
    insert_to_fbl(index->fbl, sax, pos,
                  first_bit_mask, index);
    index->total_records++;
    
    if((index->total_records % index->settings->max_total_buffer_size) == 0) {
        FLUSHES++;
        flush_fbl(index->fbl, index);
    }
    
    return first_bit_mask;
}


/**
 This function inserts an isax_node_record in an isax index.  
 @param isax_index *index
 @param isax_node_record *record
 */
enum response isax_index_insert(isax_index *index, sax_type *sax, file_position_type *pos)
{
    // Create mask for the first bit of the sax representation

    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    
    // TODO: Create INSERTION SHORT AND BINARY SEARCH METHODS.
    root_mask_type first_bit_mask = 0x00;
    CREATE_MASK(first_bit_mask, index, sax);

    isax_node *curr_node = index->first_node;
    isax_node *last_node = NULL;
    while (curr_node != NULL) {
        if(first_bit_mask == curr_node->mask) 
        {
           break;
        }
        last_node = curr_node;
        curr_node = curr_node->next;
    }
   
    
    // No appropriate root node has been found for this record
    if(curr_node == NULL) {
#ifdef DEBUG
        // Create node
        printf("*** Creating new root node. ***\n\n");
#endif
        isax_node *new_node = isax_root_node_init(first_bit_mask, 
                                                  index->settings->initial_leaf_buffer_size);
        index->root_nodes++;
    
        new_node->is_leaf = 1;
        
        if(index->first_node == NULL)
        {
            index->first_node = new_node;
        }
        else
        {
            last_node->next = new_node;
            new_node->previous = last_node;
        }
        
        curr_node = new_node;
    }
    isax_node_record * record = isax_node_record_init(index->settings->paa_segments, 
                                                      index->settings->timeseries_size,
                                                      NO_TMP | PARTIAL);
    
    memcpy((void *)record->sax, sax, index->settings->sax_byte_size);
    memcpy((void *)record->position, pos, index->settings->position_byte_size);
    
    add_record_to_node(index, curr_node, record, 1);
    
    free(record);
    
    index->total_records++;
    
    
    // TODO: use total buffered records, also increased from LEAF LOADS and SPLITS
    // and flush using this one as a flush limit counter. Reset to 0 when flushed. 
    
    // Check if flush to disk is needed
    if((index->total_records % index->settings->max_total_buffer_size) == 0) {
        FLUSHES++;
        flush_all_leaf_buffers(index, 1);
    }
    
    return SUCCESS;
}


enum response create_node_filename(isax_index *index,
                                   isax_node *node,
                                   isax_node_record *record)
{
    int i;

    node->filename = malloc(sizeof(char) * index->settings->max_filename_size);
    sprintf(node->filename, "%s", index->settings->root_directory);
    int l = (int) strlen(index->settings->root_directory);
    
    // If this has a parent then it is not a root node and as such it does have some 
    // split data on its parent about the cardinalities.
    node->isax_values = malloc(sizeof(sax_type) * index->settings->paa_segments);
    node->isax_cardinalities = malloc(sizeof(sax_type) * index->settings->paa_segments);
    
    if (node->parent) {
        for (i=0; i<index->settings->paa_segments; i++) {
            root_mask_type mask = 0x00;
            int k; 
            for (k=0; k <= node->parent->split_data->split_mask[i]; k++) 
            {
                mask |= (index->settings->bit_masks[index->settings->sax_bit_cardinality - 1 - k] & 
                         record->sax[i]);
            }
            mask = mask >> index->settings->sax_bit_cardinality - node->parent->split_data->split_mask[i] - 1;
            
            node->isax_values[i] = (int) mask;
            node->isax_cardinalities[i] = node->parent->split_data->split_mask[i]+1;
            
            if (i==0) {
                l += sprintf(node->filename+l ,"%d.%d", node->isax_values[i], node->isax_cardinalities[i]);
            }
            else {
                l += sprintf(node->filename+l ,"_%d.%d", node->isax_values[i], node->isax_cardinalities[i]);
            } 
            
        }
    }
    // If it has no parent it is root node and as such it's cardinality is 1.
    else
    {
        root_mask_type mask = 0x00;
        
        for (i=0; i<index->settings->paa_segments; i++) {
            
            mask = (index->settings->bit_masks[index->settings->sax_bit_cardinality - 1] & record->sax[i]);
            mask = mask >> index->settings->sax_bit_cardinality - 1;
            
            node->isax_values[i] = (int) mask;
            node->isax_cardinalities[i] = 1;
            
            if (i==0) {
                l += sprintf(node->filename+l ,"%d.1", (int) mask);
            }
            else {
                l += sprintf(node->filename+l ,"_%d.1", (int) mask);
            } 
        }
    }
    
#ifdef DEBUG
    printf("\tCreated filename:\t\t %s\n\n", node->filename);
#endif
    
    return SUCCESS;
}


isax_node * add_record_to_node(isax_index *index, 
                                 isax_node *tree_node, 
                                 isax_node_record *record,
                                 const char leaf_size_check) {
#ifdef DEBUG
    printf("*** Adding to node ***\n\n");
#endif
    isax_node *node = tree_node;
    
    // Traverse tree
    while (!node->is_leaf) {
        int location = index->settings->sax_bit_cardinality - 1 -
        node->split_data->split_mask[node->split_data->splitpoint];
        
        root_mask_type mask = index->settings->bit_masks[location];
        
        if(record->sax[node->split_data->splitpoint] & mask) 
        {
            node = node->right_child;
        }
        else
        {
            node = node->left_child;
        }
    }
    
    // Check if split needed
    if ((node->leaf_size) >= index->settings->max_leaf_size && leaf_size_check) {
#ifdef DEBUG
        printf(">>> %s leaf size: %d\n\n", node->filename, node->leaf_size);
#endif
        split_node(index, node);
        add_record_to_node(index, node, record, leaf_size_check);
    }
    else
    {
        if (node->filename == NULL) {
            create_node_filename(index,node,record);
        }
        add_to_node_buffer(node->buffer, record, index->settings->paa_segments, 
                           index->settings->timeseries_size);
        node->leaf_size++;
    }
    
    return node;
}

/**
 Flushes all leaf buffers of a tree to disk.
 */
enum response flush_all_leaf_buffers(isax_index *index, enum buffer_cleaning_mode buffer_clean_mode)
{
#ifndef DEBUG
    //printf("\n");
    fflush(stdout);
#else
    //printf("*** FLUSHING ***\n\n");
#endif
    // TODO: OPTIMIZE TO FLUSH WITHOUT TRAVERSAL!
    isax_node *subtree_root = index->first_node;
    
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
    int i=1;
    //fprintf(stdout, "\x1b[31mFlushing: \x1b[36m00.00%%\x1b[0m");
#endif
#if VERBOSE_LEVEL == 1
    //fprintf(stdout, "Flushing...\n");
#endif
#endif
    
    while (subtree_root != NULL) 
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
      //fprintf(stdout,"\r\x1b[31mFlushing: \x1b[36m%2.2lf%%\x1b[0m", ((float)i/(float)index->root_nodes)*100);
        i++;
        fflush(stdout);
#endif
#endif
        if(flush_subtree_leaf_buffers(index, subtree_root) == FAILURE)
            return FAILURE;                                              
        subtree_root = subtree_root->next;
    }
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
    //printf("\n");
#endif
#endif
    
    // *** FREE MEMORY OF NODES BUFFERS ***
    // Now that we have flushed its contents
    // there is no need to keep memory allocated.
    isax_index_clear_node_buffers(index, NULL, 1, buffer_clean_mode);
    
    return SUCCESS;
}


/**
 Flushes the leaf buffers of a sub-tree to disk.
 */
enum response flush_subtree_leaf_buffers (isax_index *index, isax_node *node)
{
    
    if (node->is_leaf && node->filename != NULL) {
        // Set that unloaded data exist in disk
        if (node->buffer->partial_buffer_size > 0 
            || node->buffer->tmp_partial_buffer_size > 0) {
            node->has_partial_data_file = 1;
        }
        // Set that the node has flushed full data in the disk
        if (node->buffer->full_buffer_size > 0 
            || node->buffer->tmp_full_buffer_size > 0) {
            node->has_full_data_file = 1;
        }

        if(node->has_full_data_file) {
            int prev_rec_count = node->leaf_size - (node->buffer->full_buffer_size + node->buffer->tmp_full_buffer_size);
            
            int previous_page_size =  ceil((float) (prev_rec_count * index->settings->full_record_size) / (float) PAGE_SIZE);   
            int current_page_size =   ceil((float) (node->leaf_size * index->settings->full_record_size) / (float) PAGE_SIZE);
            
            index->memory_info.disk_data_full += (current_page_size - previous_page_size);
        }
        if(node->has_partial_data_file) {
            int prev_rec_count = node->leaf_size - (node->buffer->partial_buffer_size + node->buffer->tmp_partial_buffer_size);
            
            int previous_page_size =  ceil((float) (prev_rec_count * index->settings->partial_record_size) / (float) PAGE_SIZE);   
            int current_page_size =   ceil((float) (node->leaf_size * index->settings->partial_record_size) / (float) PAGE_SIZE);
            
            index->memory_info.disk_data_partial += (current_page_size - previous_page_size);
        }
        if(node->has_full_data_file && node->has_partial_data_file) {
             printf("WARNING: (Mem size counting) this leaf has both partial and full data.\n");
        }
        index->memory_info.disk_data_full += (node->buffer->full_buffer_size + 
                                              node->buffer->tmp_full_buffer_size);

        index->memory_info.disk_data_partial += (node->buffer->partial_buffer_size + 
                                                 node->buffer->tmp_partial_buffer_size); 

        flush_node_buffer(node->buffer, index->settings->paa_segments, 
                          index->settings->timeseries_size,
                          node->filename);
    }
    else if (!node->is_leaf)
    {
        flush_subtree_leaf_buffers(index, node->left_child);
        flush_subtree_leaf_buffers(index, node->right_child);
    }
    
    return SUCCESS;
}

void isax_index_clear_node_buffers(isax_index *index, isax_node *node, 
                                   enum node_cleaning_mode node_cleaning_mode,
                                   enum buffer_cleaning_mode buffer_clean_mode) {
    if (node == NULL) {
        // TODO: OPTIMIZE TO FLUSH WITHOUT TRAVERSAL!
        isax_node *subtree_root = index->first_node;
        
        while (subtree_root != NULL) 
        {
            isax_index_clear_node_buffers(index, subtree_root, node_cleaning_mode, buffer_clean_mode);
            subtree_root = subtree_root->next;
        }
    }
    else {
        // Traverse tree
        if(!node->is_leaf && node_cleaning_mode == INCLUDE_CHILDREN) {
            isax_index_clear_node_buffers(index, node->right_child, node_cleaning_mode, buffer_clean_mode);
            isax_index_clear_node_buffers(index, node->left_child, node_cleaning_mode, buffer_clean_mode);
        }
        else if(node->is_leaf && node->buffer != NULL)
        {
            clear_node_buffer(node->buffer, buffer_clean_mode);
        }
    }
}


int comp(const void * a, const void * b) 
{
    isax_node_record *ra = (isax_node_record*) a;
    isax_node_record *rb = (isax_node_record*) b;
    
    if (*ra->position==*rb->position)
        return 0;
    else
        if (*ra->position < *rb->position)
            return -1;
        else
            return 1;
}

void find_empty_children(isax_node *start_node, isax_node **nodes_to_load,
                         int * number, int * offset) {
    
    
    if (*number == 0 || start_node == NULL) {
        return;
    }
    //printf("*** Loading all children of node: %p\n", start_node);
    
    
    if (start_node->is_leaf && (start_node->has_partial_data_file || 
                                start_node->buffer->partial_buffer_size > 0 ||
                                start_node->buffer->tmp_partial_buffer_size > 0
                                )) {
        nodes_to_load[*offset] = start_node;
        (*offset)++;
        (*number)--;
#ifdef DEBUG
        //printf("*** %pis a leaf, loaded.\n", start_node);
#endif
    }
    else {
        if (start_node->left_child != NULL) {
            find_empty_children(start_node->left_child, nodes_to_load, 
                                number, offset);
        }
        if(start_node->right_child != NULL) {
            find_empty_children(start_node->right_child, nodes_to_load, 
                                number, offset);
        }
    }
}



int find_siblings(isax_node *c_node, isax_node **nodes_to_load,
                  int *number_of_leaves, int *offset) {
    
    find_empty_children(c_node, nodes_to_load, number_of_leaves, offset);
    
    // Current node is already loaded.
    if (offset == 0) {
        return 0;
    }
    
    // In-subtree siblings...
    while (c_node->parent != NULL && *number_of_leaves > 0) {
        isax_node *parent = NULL;
        isax_node *sibling = NULL;
        
        parent = c_node->parent;
        if (parent->left_child == c_node) 
            sibling = parent->right_child;
        if (parent->right_child == c_node)
            sibling = parent->left_child;

#ifdef DEBUG
        //printf("Found sibling: %p\n", sibling);
#endif
        find_empty_children(sibling, nodes_to_load, number_of_leaves, offset);
        
        c_node = c_node->parent;
    }
    
#ifndef CLUSTERED
    // In different sub-tree siblings... only if it is not clustered.
    isax_node *ptr_left = c_node->previous;
    isax_node *ptr_right = c_node->next;
    while (*number_of_leaves > 0 && 
           !(ptr_left == NULL && ptr_right == NULL)) {
        if(ptr_left != NULL)
        {
            find_empty_children(ptr_left, nodes_to_load, 
                                number_of_leaves, offset);
            ptr_left = ptr_left->previous;
            
        }
        if(ptr_right != NULL)
        {
            find_empty_children(ptr_right, nodes_to_load, 
                                number_of_leaves, offset);
            ptr_right = ptr_right->next;
        }
    }
#endif
    return SUCCESS;
}

//float isax_index_load_node(isax_index *index, isax_node *c_node, ts_type * query, float bsf) {
float isax_index_load_node(isax_index *index, isax_node *c_node, ts_type * query_ts_reordered, int offset_q,float bsf, int * query_order) {
	isax_node *fbl_node = c_node;
#ifdef CLUSTERED
    while (fbl_node->parent != NULL){
        fbl_node = fbl_node->parent;
    }
    char* pfilename = malloc(255);
    snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename, fbl_node->mask);
    COUNT_PARTIAL_RAND_OUTPUT
    COUNT_PARTIAL_OUTPUT_TIME_START
    FILE * raw_file = fopen(pfilename, "r");
    COUNT_PARTIAL_OUTPUT_TIME_END    
#else
    COUNT_PARTIAL_RAND_OUTPUT
    COUNT_PARTIAL_OUTPUT_TIME_START
    FILE * raw_file = fopen(index->settings->raw_filename, "r");
    COUNT_PARTIAL_OUTPUT_TIME_END        
#endif
    
    int total_loaded_leaves = index->settings->total_loaded_leaves;
    // *** Step 1 *** Decide nodes to load
    int leaves_to_load = total_loaded_leaves; 
    int offset = 0;
    isax_node **nodes_to_load = malloc(sizeof(isax_node *) * leaves_to_load);
    find_siblings(c_node, nodes_to_load, &leaves_to_load, &offset);
    

    // Current node is already loaded.
    if (offset == 0) {
        return FLT_MAX;
    }
    COUNT_LOADED_NODE
    //COUNT_LOADED_TS(node->leaf_size)
    
    // *** Step 2 *** Prepare load buffers for all nodes
    isax_node_record *load_buffer = NULL;
    int total_records = 0;
    int node_id=0;
    int load_buffer_index = 0;
    
    for (node_id = 0; node_id < offset; node_id++) {
        isax_node * node = nodes_to_load[node_id];
            
        FILE * partial_file;
        int file_records = 0;
        int buffer_records = 0;
        
        if (node->buffer)
        {
            buffer_records = node->buffer->partial_buffer_size 
            + node->buffer->tmp_partial_buffer_size;
            total_records += buffer_records;
        }
        
        if(node->has_partial_data_file) {
        	char * partial_fname = malloc(sizeof(char) * (strlen(node->filename) + 6));
            strcpy(partial_fname, node->filename);
            strcat(partial_fname, ".part");
            COUNT_PARTIAL_INPUT_TIME_START
            COUNT_PARTIAL_SEQ_INPUT
            partial_file = fopen(partial_fname, "r");
            COUNT_PARTIAL_INPUT_TIME_END
	    if(partial_file != NULL) {
                COUNT_PARTIAL_INPUT_TIME_START	      
                COUNT_PARTIAL_RAND_INPUT
		fseek(partial_file, 0L, SEEK_END);
                size_t file_size = ftell(partial_file);
                COUNT_PARTIAL_RAND_INPUT
		fseek(partial_file, 0L, SEEK_SET);
                COUNT_PARTIAL_INPUT_TIME_END		
                file_records = (int) file_size / (int)index->settings->partial_record_size;
                total_records += file_records;
            }
            else {
            	fprintf(stderr, "[ERROR] LEAF FILE \"%s\" DOES NOT EXIST.\n", partial_fname);
            	system("ls ");
            }
            COUNT_INPUT_TIME_END
            free(partial_fname);
        }
         
        if (load_buffer == NULL)
            load_buffer = malloc(sizeof(isax_node_record) * (total_records));
        else
            load_buffer = realloc(load_buffer, sizeof(isax_node_record) * 
                                  (total_records));
        
        // PARTIAL DATA THAT ARE LOCATED IN BUFFERS
        if (node->buffer != NULL) {
            int i;
            for (i=0; i<node->buffer->partial_buffer_size; i++) {
                // CREATE RECORD AND SET AS NO_TMP
                // NO MALLOC JUST REFERENCE
                load_buffer[load_buffer_index].position = node->buffer->partial_position_buffer[i];
                load_buffer[load_buffer_index].sax = node->buffer->partial_sax_buffer[i];
                load_buffer[load_buffer_index].ts = malloc(index->settings->ts_byte_size);
                load_buffer[load_buffer_index].insertion_mode = FULL | NO_TMP;
                load_buffer[load_buffer_index].destination = node;
                load_buffer_index++;
            }
            node->buffer->partial_buffer_size = 0;
            
            for (i=0; i<node->buffer->tmp_partial_buffer_size; i++) {
                // CREATE RECORD AND SET AS TMP
                // NO MALLOC JUST REFERENCE
                load_buffer[load_buffer_index].position = node->buffer->tmp_partial_position_buffer[i];
                load_buffer[load_buffer_index].sax = node->buffer->tmp_partial_sax_buffer[i];
                load_buffer[load_buffer_index].ts = malloc(index->settings->ts_byte_size);
                load_buffer[load_buffer_index].insertion_mode = FULL | TMP;
                load_buffer[load_buffer_index].destination = node;
                load_buffer_index++;
            }
            node->buffer->tmp_partial_buffer_size = 0;
            // Count memory that was allocated here
            index->allocated_memory += index->settings->ts_byte_size * (node->buffer->tmp_partial_buffer_size +
                                                                        node->buffer->partial_buffer_size);
        }
        
        // PARTIAL DATA THAT ARE IN FILE ON DISK
        if (file_records > 0) {
            int i;
            for (i=0; i<file_records; i++) {
                // CREATE RECORD AND SET AS TMP
                // MALLOC FOR SAX, TS, LOCATION
                load_buffer[load_buffer_index].position = malloc(index->settings->position_byte_size);
                load_buffer[load_buffer_index].sax = malloc(index->settings->sax_byte_size);
                load_buffer[load_buffer_index].ts = malloc(index->settings->ts_byte_size);
                load_buffer[load_buffer_index].insertion_mode = FULL | TMP;
                load_buffer[load_buffer_index].destination = node;
                COUNT_PARTIAL_SEQ_INPUT                
                COUNT_PARTIAL_SEQ_INPUT
		COUNT_PARTIAL_INPUT_TIME_START
                fread(load_buffer[load_buffer_index].position, sizeof(file_position_type), 1, partial_file);
                fread(load_buffer[load_buffer_index].sax, sizeof(sax_type), index->settings->paa_segments, partial_file);
                COUNT_PARTIAL_INPUT_TIME_END
                load_buffer_index++;
            }


            // Count memory that was allocated here
            index->allocated_memory += (file_records * index->settings->full_record_size);
            
            // Remove memory that is deallocated from disk
            index->memory_info.disk_data_partial -= ceil((float)(file_records * index->settings->partial_record_size) / (float) PAGE_SIZE);
            
            COUNT_PARTIAL_INPUT_TIME_START
            fclose(partial_file);
            COUNT_PARTIAL_INPUT_TIME_END
        }
        #ifdef BENCHMARK
        if (total_records > 0) {
            COUNT_LOADED_NODE
	    //COUNT_LOADED_TS(node->leaf_size)
        }
        #endif
        
        load_buffer_index = total_records;
    }
    
    
    // *** Step 3 *** Sort array of data to load
    qsort(load_buffer,total_records,sizeof(isax_node_record), comp);
    
    
    // *** Step 4 *** Read node data from raw file and store in tree
    //float bsf = FLT_MAX;
    int i;
    for (i=0; i<total_records; i++) {
        COUNT_LOADED_RECORD()

        index->loaded_records++;
        
        //printf("LOADING POSITION: %d\n", *load_buffer[i].position);
	COUNT_PARTIAL_RAND_INPUT  
	COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START
        fseek(raw_file, *load_buffer[i].position, SEEK_SET);
        fread(load_buffer[i].ts, sizeof(ts_type), index->settings->timeseries_size, raw_file);
        COUNT_PARTIAL_INPUT_TIME_END
        

        if(query_ts_reordered != NULL)
        {
    	   COUNT_CHECKED_TS(1);
    	   COUNT_LOADED_TS(1);
           ts_type dist = ts_euclidean_distance_reordered(query_ts_reordered,
						load_buffer[i].ts,
						offset_q,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf,
						query_order);     

            // Also calculate distance
            /*float dist = ts_euclidean_distance(query, load_buffer[i].ts, 
                                               index->settings->timeseries_size, bsf);*/
            if (dist < bsf) {
                bsf = dist;
                #ifdef STORE_ANSWER
                ts_type *ts = load_buffer[i].ts;
                memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }
        
        //printf("Distance: %lf\n", dist);
        
        add_to_node_buffer(((isax_node *)load_buffer[i].destination)->buffer, 
                           &load_buffer[i], 
                           index->settings->paa_segments, 
                           index->settings->timeseries_size);
    }
    free(load_buffer);
    COUNT_PARTIAL_INPUT_TIME_START
    fclose(raw_file);
    COUNT_PARTIAL_INPUT_TIME_END
    
    // Set nodes as loaded.
    for (node_id = 0; node_id < offset; node_id++) {
        nodes_to_load[node_id]->has_partial_data_file = 0;
    }
    
    if (index->allocated_memory > index->settings->max_total_full_buffer_size)
    {
        FLUSHES++;
        flush_all_leaf_buffers(index, TMP_ONLY_CLEAN);
        index->allocated_memory = 0;
    }
    free(nodes_to_load);
    return bsf;
}

float calculate_minimum_distance (isax_index *index, isax_node *node, ts_type *raw_query, ts_type *query) {
	//printf("Calculating minimum distance...\n");
	float bsfLeaf =   minidist_paa_to_isax(query, node->isax_values, 
                                                                     node->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,  
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
	float bsfRecord = FLT_MAX;																 
	//printf("---> Distance: %lf\n", bsfLeaf);
    //sax_print(node->isax_values, 1,  index->settings->sax_bit_cardinality);

	if(!index->has_wedges) {
//		printf("--------------\n");
		int i = 0;
		if(node->has_partial_data_file) {
			char * partial_fname = malloc(sizeof(char) * (strlen(node->filename) + 6));
			strcpy(partial_fname, node->filename);
			strcat(partial_fname, ".part");
			COUNT_PARTIAL_RAND_INPUT
			COUNT_PARTIAL_INPUT_TIME_START
			FILE * partial_file = fopen(partial_fname, "r");
			COUNT_PARTIAL_INPUT_TIME_END			
			sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
			file_position_type *pos = malloc(sizeof(file_position_type));
			while(!feof(partial_file)) {
       		            COUNT_PARTIAL_SEQ_INPUT
       		            COUNT_PARTIAL_INPUT_TIME_START			      
			    size_t bytes_read = fread(pos, sizeof(file_position_type), 1, partial_file);
       		            COUNT_PARTIAL_INPUT_TIME_END			      
 			    if(bytes_read) {
			      		COUNT_PARTIAL_SEQ_INPUT
             		                COUNT_PARTIAL_INPUT_TIME_START			      
					  bytes_read = fread(sax, sizeof(sax_type), index->settings->paa_segments, partial_file);
             		                COUNT_PARTIAL_INPUT_TIME_END  			      
					if(bytes_read) {
						float mindist = minidist_paa_to_isax_raw(query, sax, index->settings->max_sax_cardinalities,
															 index->settings->sax_bit_cardinality,
															 index->settings->sax_alphabet_cardinality,
															 index->settings->paa_segments, MINVAL, MAXVAL,
															 index->settings->mindist_sqrt);
//			printf("+[FILE] %lf\n", mindist);

						if(mindist < bsfRecord) {
							bsfRecord = mindist;
						}
					}
				}
			}
			COUNT_PARTIAL_INPUT_TIME_START			
			fclose(partial_file);
			COUNT_PARTIAL_INPUT_TIME_END
			free(sax);
			free(pos);
			free(partial_fname);
		}

		if (node->buffer != NULL) {
			for (i=0; i<node->buffer->partial_buffer_size; i++) {
				float mindist = minidist_paa_to_isax_raw(query, node->buffer->partial_sax_buffer[i], index->settings->max_sax_cardinalities,
													 index->settings->sax_bit_cardinality,
													 index->settings->sax_alphabet_cardinality,
													 index->settings->paa_segments, MINVAL, MAXVAL,
													 index->settings->mindist_sqrt);
//				printf("+[PARTIAL] %lf\n", mindist);
				if(mindist < bsfRecord) {
					bsfRecord = mindist;
				}
			}

			for (i=0; i<node->buffer->tmp_partial_buffer_size; i++) {
				float mindist = minidist_paa_to_isax_raw(query, node->buffer->tmp_partial_sax_buffer[i], index->settings->max_sax_cardinalities,
													 index->settings->sax_bit_cardinality,
													 index->settings->sax_alphabet_cardinality,
													 index->settings->paa_segments, MINVAL, MAXVAL,
													 index->settings->mindist_sqrt);
//				printf("+[TMP_PARTIAL] %lf\n", mindist);
				if(mindist < bsfRecord) {
					bsfRecord = mindist;
				}
			}
		}
	}
	else {
		int i=0;
		if(node->wedges[0] == FLT_MIN) {
			bsfRecord = FLT_MAX;
		}
		else {
			bsfRecord = 0;
			ts_type *min_wedge = &node->wedges[0];
			ts_type *max_wedge = &node->wedges[index->settings->timeseries_size];
			if(raw_query[i] > max_wedge[i]) {
				bsfRecord += (raw_query[i] - max_wedge[i]) * (raw_query[i] - max_wedge[i]);
			}
			else if(raw_query[i] < max_wedge[i] && raw_query[i] > min_wedge[i]) {
				//bound += 0;
			}
			else {
				bsfRecord += (min_wedge[i] - raw_query[i]) * (min_wedge[i] - raw_query[i]);
			}
			//bsfRecord = sqrtf(bsfRecord);
		}

	}
	float bsf = (bsfRecord == FLT_MAX) ? bsfLeaf : bsfRecord;
//	printf("\t%.2lf - %d [%d] : %s.%s\n",bsfRecord, node->leaf_size, node->is_leaf, node->filename, node->has_full_data_file ? ".full" : ".part");

		
	//printf("---> Final: %lf\n", bsf);
	return  bsf;
}

//float calculate_node_distance (isax_index *index, isax_node *node, ts_type *query, float bsf) {
ts_type calculate_node_distance (isax_index *index, isax_node *node,
				 ts_type *query_ts_reordered, int *query_order,
				 unsigned int offset, ts_type bsf)

{
    
    // If node has buffered data
    if (node->buffer != NULL) {
        int i;
        /*
        for (i=0; i<node->buffer->full_buffer_size; i++) {
            float dist = ts_euclidean_distance(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
                #ifdef STORE_ANSWER
                ts_type *ts = node->buffer->full_ts_buffer[i];
                memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }
        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
                #ifdef STORE_ANSWER
                ts_type *ts = node->buffer->tmp_full_ts_buffer[i];
                memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }
        */
        COUNT_CHECKED_NODE
        COUNT_CHECKED_TS(node->buffer->full_buffer_size)
        for (i=0; i<node->buffer->full_buffer_size; i++) {

            ts_type dist = ts_euclidean_distance_reordered(query_ts_reordered,
						node->buffer->full_ts_buffer[i],
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf,
						query_order);     

            if (dist < bsf) {
                bsf = dist;
                #ifdef STORE_ANSWER
                ts_type *ts = node->buffer->full_ts_buffer[i];
                memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }
        COUNT_CHECKED_TS(node->buffer->tmp_full_buffer_size)
        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            ts_type dist = ts_euclidean_distance_reordered(query_ts_reordered,
						node->buffer->tmp_full_ts_buffer[i],
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf,
						query_order);     
            if (dist < bsf) {
                bsf = dist;
                #ifdef STORE_ANSWER
                ts_type *ts = node->buffer->tmp_full_ts_buffer[i];
                memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }

    }
    
    if (node->has_full_data_file) {
    	char * full_fname = malloc(sizeof(char) * (strlen(node->filename) + 6));
        strcpy(full_fname, node->filename);
        strcat(full_fname, ".full");
        COUNT_PARTIAL_INPUT_TIME_START
	COUNT_PARTIAL_RAND_INPUT
	FILE * full_file = fopen(full_fname, "r");
        COUNT_PARTIAL_INPUT_TIME_END
        
        if(full_file != NULL) {
            COUNT_PARTIAL_INPUT_TIME_START
	    COUNT_PARTIAL_RAND_INPUT
	    fseek(full_file, 0L, SEEK_END);
            size_t file_size = ftell(full_file);
	    COUNT_PARTIAL_RAND_INPUT
	    fseek(full_file, 0L, SEEK_SET);
            COUNT_PARTIAL_INPUT_TIME_END
            int file_records = (int) file_size / (int)index->settings->full_record_size;
            ts_type *ts = malloc(index->settings->ts_byte_size);
            COUNT_LOADED_NODE
            COUNT_CHECKED_TS(file_records)
            COUNT_LOADED_TS(file_records)

            while (file_records > 0) {

                COUNT_PARTIAL_INPUT_TIME_START
	        COUNT_PARTIAL_RAND_INPUT
	        fseek(full_file, index->settings->sax_byte_size, SEEK_CUR);
       	        COUNT_PARTIAL_SEQ_INPUT		
                fread(ts, sizeof(ts_type), index->settings->timeseries_size, full_file);
                COUNT_PARTIAL_INPUT_TIME_END
                ts_type dist = ts_euclidean_distance_reordered(query_ts_reordered,
						ts,
						offset,  //offset is 0 for whole matching
						index->settings->timeseries_size,
						bsf,
						query_order);     

                if (dist < bsf) {
                    bsf = dist;
                    #ifdef STORE_ANSWER
                    memcpy(index->answer, ts, index->settings->timeseries_size * sizeof(ts_type));
                    #endif
                }
                file_records--;
            } 
            
            free(ts);
        }

        COUNT_PARTIAL_INPUT_TIME_START
        fclose(full_file);
        COUNT_PARTIAL_INPUT_TIME_END
        free(full_fname);
    }
    

    //////// ADAPTIVE ////////////////////
    
    if (node->is_leaf && (node->has_partial_data_file || 
                    node->buffer->partial_buffer_size > 0 ||
                    node->buffer->tmp_partial_buffer_size > 0
                    )) {
        //COUNT_LOADED_NODE
        //COUNT_LOADED_TS(node->leaf_size)

        COUNT_PARTIAL_LOAD_NODE_TIME_START

    	//float partial_bsf = isax_index_load_node(index, node, query, bsf);
        ts_type partial_bsf = isax_index_load_node(index, node, query_ts_reordered, offset,bsf, query_order);
        COUNT_PARTIAL_LOAD_NODE_TIME_END        
        if (partial_bsf < bsf)
            bsf = partial_bsf;
    }
    
    //////////////////////////////////////
    
    return bsf;
}

void load_random_leaf(isax_index *index) {
    isax_node *rnode = index->first_node;
    
    while (rnode->next != NULL) {
        rnode = rnode->next;
        isax_node *curr = rnode;
        while (!curr->is_leaf) {
            curr = curr->left_child;
        }
        COUNT_PARTIAL_LOAD_NODE_TIME_START
        isax_index_load_node(index, curr,NULL, 0,FLT_MAX, NULL);
	//isax_index_load_node(index, curr, NULL, FLT_MAX);
        COUNT_PARTIAL_LOAD_NODE_TIME_END	
    }
}

void complete_index_leafs(isax_index *index) 
{
    isax_node *subtree_root = index->first_node;
    while (subtree_root != NULL) {
        complete_subtree_leafs(index, subtree_root);
        subtree_root = subtree_root->next;
    }
    // TODO: CHECK THAT THIS IS NEEDED!
    FLUSHES++;
    flush_all_leaf_buffers(index, TMP_ONLY_CLEAN);
    index->allocated_memory = 0;
}

void complete_subtree_leafs(isax_index *index, isax_node *node) 
{
    if(node->is_leaf)
    {
        COUNT_PARTIAL_LOAD_NODE_TIME_START      
        isax_index_load_node(index, node, NULL, 0,FLT_MAX,NULL);
        COUNT_PARTIAL_LOAD_NODE_TIME_END	
    }
    else 
    {
        complete_subtree_leafs(index, node->left_child);
        complete_subtree_leafs(index, node->right_child);
    }
}

void complete_index(isax_index *index, int ts_num) { 
    
    FILE * ifile;
    
    COUNT_PARTIAL_INPUT_TIME_START
    COUNT_PARTIAL_RAND_INPUT
    ifile = fopen (index->settings->raw_filename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END
    
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", index->settings->raw_filename);
        exit(-1);
    }
    
    COUNT_PARTIAL_RAND_INPUT    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    COUNT_PARTIAL_RAND_INPUT
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", 
                index->settings->raw_filename, total_records);
        exit(-1);
    }
    
    
    unsigned long long ts_loaded = 0;    
    isax_node_record *record = malloc(sizeof(isax_node_record));   
    
    while (ts_loaded<ts_num)
    {
        ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
        sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
        
#ifndef DEBUG  
#if VERBOSE_LEVEL == 2
        //printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        COUNT_PARTIAL_INPUT_TIME_START
        COUNT_PARTIAL_SEQ_INPUT	  
        fread(ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END
        
        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment, 
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {

            root_mask_type first_bit_mask = 0x00;
            
            CREATE_MASK(first_bit_mask, index, sax);
            record->sax = sax;
            record->position = malloc(sizeof(file_position_type));
            record->ts = ts;
            *(record->position) =  ts_loaded * index->settings->ts_byte_size;
            record->insertion_mode = FULL | TMP;
            if (index->fbl->soft_buffers[first_bit_mask].initialized == 0) {
                index->fbl->soft_buffers[first_bit_mask].initialized = 1;
                index->fbl->soft_buffers[first_bit_mask].node = isax_root_node_init(first_bit_mask, 
                                                                                    index->settings->initial_leaf_buffer_size);
                index->root_nodes++;
                index->fbl->soft_buffers[first_bit_mask].node->is_leaf = 1;
                
                if(index->first_node == NULL)
                {
                    index->first_node = index->fbl->soft_buffers[first_bit_mask].node;
                    index->fbl->soft_buffers[first_bit_mask].node->next = NULL;
                    index->fbl->soft_buffers[first_bit_mask].node->previous = NULL;
                }
                else
                {
                    isax_node * prev_first = index->first_node;
                    index->first_node = index->fbl->soft_buffers[first_bit_mask].node;
                    index->first_node->next = prev_first;
                    prev_first->previous = index->fbl->soft_buffers[first_bit_mask].node;
                }
                
            }
            isax_node * dest = add_record_to_node(index, index->fbl->soft_buffers[first_bit_mask].node, record, 0);
            
            // Define leaf as loaded.
            if(!dest->has_full_data_file)
            {
                dest->has_full_data_file = 1;
                dest->has_partial_data_file = 0;
            }
            ts_loaded++;
        }
        else
        {
            fprintf(stderr, "error: SAX representation failed to be created.");
        }
        COUNT_INPUT_TIME_START

       
        if (ts_loaded % index->settings->max_total_full_buffer_size == 0)
        {
            FLUSHES++;
            flush_all_leaf_buffers(index, TMP_ONLY_CLEAN);
        }
       
    }
    free(record);
    FLUSHES++;
    
    // TODO: CHECK THAT THIS IS NEEDED!
    flush_all_leaf_buffers(index, TMP_ONLY_CLEAN);
    COUNT_PARTIAL_INPUT_TIME_START
    fclose(ifile);
    COUNT_PARTIAL_INPUT_TIME_END
    
}

void cache_sax_file(isax_index *index) {
        COUNT_PARTIAL_OUTPUT_TIME_START
	fflush(index->sax_file);
	COUNT_PARTIAL_OUTPUT_TIME_END
	COUNT_PARTIAL_INPUT_TIME_START
	COUNT_PARTIAL_RAND_INPUT	  
	fseek(index->sax_file, 0L, SEEK_END);
	unsigned long size = ftell(index->sax_file);
        COUNT_PARTIAL_RAND_INPUT
        fseek(index->sax_file, 0L, SEEK_SET);
	index->sax_cache = malloc(size);
	index->sax_cache_size = size / index->settings->sax_byte_size;
        COUNT_PARTIAL_SEQ_INPUT
	fread(index->sax_cache, index->settings->sax_byte_size, index->sax_cache_size, index->sax_file);
	COUNT_PARTIAL_INPUT_TIME_END

    index->memory_info.mem_summaries = index->sax_cache_size;
}

void node_write(isax_index *index, isax_node *node, FILE *file) {

        COUNT_PARTIAL_OUTPUT_TIME_START
	COUNT_PARTIAL_SEQ_OUTPUT
	COUNT_PARTIAL_SEQ_OUTPUT
	fwrite(&(node->is_leaf), sizeof(unsigned char), 1, file);
	fwrite(&(node->mask), sizeof(root_mask_type), 1, file);
        COUNT_PARTIAL_OUTPUT_TIME_END
	  
	if(node->is_leaf) {
		if(node->filename != NULL) {
			int filename_size = strlen(node->filename);
          	        COUNT_PARTIAL_SEQ_OUTPUT
          	        COUNT_PARTIAL_SEQ_OUTPUT			  
          	        COUNT_PARTIAL_SEQ_OUTPUT
          	        COUNT_PARTIAL_SEQ_OUTPUT
          	        COUNT_PARTIAL_SEQ_OUTPUT			  
       	                COUNT_PARTIAL_SEQ_OUTPUT
			COUNT_PARTIAL_OUTPUT_TIME_START
		        fwrite(&filename_size, sizeof(int), 1, file);
			fwrite(node->filename, sizeof(char), filename_size, file);
			fwrite(&(node->leaf_size), sizeof(int), 1, file);
	                fwrite(&(node->level), sizeof(unsigned int), 1, file);
			fwrite(&(node->has_full_data_file), sizeof(char), 1, file);
			fwrite(&(node->has_partial_data_file), sizeof(char), 1, file);
                        COUNT_PARTIAL_OUTPUT_TIME_END			
			COUNT_LEAF_NODE
			  //collect stats while traversing the tree
			update_index_stats(index, node);
		}
		else {
			int filename_size = 0;
          	        COUNT_PARTIAL_SEQ_OUTPUT	  
                        COUNT_PARTIAL_OUTPUT_TIME_START
			fwrite(&filename_size, sizeof(int), 1, file);
                        COUNT_PARTIAL_OUTPUT_TIME_END			
		}
	}
	else {
          	COUNT_PARTIAL_SEQ_OUTPUT
          	COUNT_PARTIAL_SEQ_OUTPUT
                COUNT_PARTIAL_OUTPUT_TIME_START		  
		fwrite(&(node->split_data->splitpoint), sizeof(int), 1, file);
		fwrite(node->split_data->split_mask, sizeof(sax_type), index->settings->paa_segments, file);
                COUNT_PARTIAL_OUTPUT_TIME_END		
	}

	if(node->isax_cardinalities != NULL) {
		char has_isax_data = 1;
          	COUNT_PARTIAL_SEQ_OUTPUT
          	COUNT_PARTIAL_SEQ_OUTPUT
          	COUNT_PARTIAL_SEQ_OUTPUT
                COUNT_PARTIAL_OUTPUT_TIME_START		  
		fwrite(&has_isax_data, sizeof(char), 1, file);
		fwrite(node->isax_cardinalities, sizeof(sax_type), index->settings->paa_segments, file);
		fwrite(node->isax_values, sizeof(sax_type), index->settings->paa_segments, file);
                COUNT_PARTIAL_OUTPUT_TIME_END		
	}
	else {
		char has_isax_data = 0;
          	COUNT_PARTIAL_SEQ_OUTPUT
                COUNT_PARTIAL_OUTPUT_TIME_START		  
		fwrite(&has_isax_data, sizeof(char), 1, file);
                COUNT_PARTIAL_OUTPUT_TIME_END		
	}

	if(!node->is_leaf) {
		node_write(index, node->left_child, file);
		node_write(index, node->right_child, file);
	}
}

isax_node *node_read(isax_index *index, FILE *file) {
        COUNT_NEW_NODE
	isax_node *node;
	unsigned char is_leaf = 0;
	root_mask_type mask = 0;
	
	COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START

	fread(&is_leaf, sizeof(unsigned char), 1, file);
	fread(&(mask), sizeof(root_mask_type), 1, file);
	COUNT_PARTIAL_INPUT_TIME_END
	  
    index->memory_info.mem_tree_structure++;

	if(is_leaf) {
		node = malloc(sizeof(isax_node));
		node->is_leaf = 1;
		node->has_partial_data_file = 0;
		node->has_full_data_file = 0;
		node->right_child = NULL;
		node->left_child = NULL;
		node->parent = NULL;
		node->next = NULL;
		node->leaf_size = 0;
		node->filename = NULL;
		node->isax_values = NULL;
		node->isax_cardinalities = NULL;
		node->previous = NULL;
		node->split_data = NULL;
		node->buffer = init_node_buffer(index->settings->initial_leaf_buffer_size);
		node->mask = 0;

		node->level = 0;
           
		int filename_size = 0;
     	        COUNT_PARTIAL_INPUT_TIME_START
 	        COUNT_PARTIAL_SEQ_INPUT		  
		fread(&filename_size, sizeof(int), 1, file);
     	        COUNT_PARTIAL_INPUT_TIME_END		
		if(filename_size > 0) {
			node->filename = malloc(sizeof(char) * (filename_size + 1));
      	                COUNT_PARTIAL_INPUT_TIME_START
 	                COUNT_PARTIAL_SEQ_INPUT
 	                COUNT_PARTIAL_SEQ_INPUT
 	                COUNT_PARTIAL_SEQ_INPUT
 	                COUNT_PARTIAL_SEQ_INPUT			  
 	                COUNT_PARTIAL_SEQ_INPUT
			fread(node->filename, sizeof(char), filename_size, file);
			fread(&(node->leaf_size), sizeof(int), 1, file);
	                fread(&(node->level), sizeof(unsigned int), 1, file);			
			//printf ("leaf_size = %d\n", node->leaf_size);
			fread(&(node->has_full_data_file), sizeof(char), 1, file);
			fread(&(node->has_partial_data_file), sizeof(char), 1, file);
      	                COUNT_PARTIAL_INPUT_TIME_END			
                        node->filename[filename_size] = '\0';			
			//COUNT_NEW_NODE
			COUNT_LEAF_NODE

   	                index->stats->leaves_heights[index->stats->leaves_counter] = node->level + 1;
			index->stats->leaves_sizes[index->stats->leaves_counter] = node->leaf_size;
			++(index->stats->leaves_counter);
			  
			update_index_stats(index, node);

            if(node->has_full_data_file) {
                float number_of_bytes = (float) (node->leaf_size * index->settings->full_record_size);
                int number_of_pages = ceil(number_of_bytes / (float) PAGE_SIZE);
                index->memory_info.disk_data_full += number_of_pages;
            }
            else if(node->has_partial_data_file) {
                float number_of_bytes = (float) (node->leaf_size * index->settings->partial_record_size);
                int number_of_pages = ceil(number_of_bytes / (float) PAGE_SIZE);
                index->memory_info.disk_data_partial += number_of_pages;
            }
            if(node->has_full_data_file && node->has_partial_data_file) {
                printf("WARNING: (Mem size counting) this leaf has both partial and full data.\n");
            }
		}
		else {
			node->filename = NULL;
			node->leaf_size = 0;
			node->has_full_data_file = 0;
			node->has_partial_data_file = 0;
		}
	}
	else {
		node = malloc(sizeof(isax_node));
		node->buffer = NULL;
		node->is_leaf = 0;
		node->filename = NULL;
		node->has_full_data_file = 0;
		node->has_partial_data_file = 0;
		node->leaf_size = 0;
		node->split_data = malloc(sizeof(isax_node_split_data));
		node->split_data->split_mask = malloc(sizeof(sax_type) * index->settings->paa_segments);
 	        COUNT_PARTIAL_SEQ_INPUT
 	        COUNT_PARTIAL_SEQ_INPUT		  
      	        COUNT_PARTIAL_INPUT_TIME_START
		fread(&(node->split_data->splitpoint), sizeof(int), 1, file);
		fread(node->split_data->split_mask, sizeof(sax_type), index->settings->paa_segments, file);
      	        COUNT_PARTIAL_INPUT_TIME_END		
	}
	node->mask = mask;

	char has_isax_data = 0;
	COUNT_PARTIAL_SEQ_INPUT
      	COUNT_PARTIAL_INPUT_TIME_START
	fread(&has_isax_data, sizeof(char), 1, file);
      	COUNT_PARTIAL_INPUT_TIME_END
	  
	if(has_isax_data) {
		node->isax_cardinalities = malloc(sizeof(sax_type) * index->settings->paa_segments);
		node->isax_values = malloc(sizeof(sax_type) * index->settings->paa_segments);
                COUNT_PARTIAL_SEQ_INPUT		  
      	        COUNT_PARTIAL_INPUT_TIME_START			
		fread(node->isax_cardinalities, sizeof(sax_type), index->settings->paa_segments, file);
		fread(node->isax_values, sizeof(sax_type), index->settings->paa_segments, file);
      	        COUNT_PARTIAL_INPUT_TIME_END					
	}
	else {
		node->isax_cardinalities = NULL;
		node->isax_values = NULL;
	}

	if(!is_leaf) {
		node->left_child = node_read(index, file);
		node->right_child = node_read(index, file);
	}

	return node;
}


void index_write(isax_index *index)
{
        fprintf(stderr, ">>> Storing index: %s\n", index->settings->root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
	filename = strcpy(filename, index->settings->root_directory);
	filename = strcat(filename, "/index.idx");
      	COUNT_PARTIAL_RAND_OUTPUT			       
      	COUNT_PARTIAL_OUTPUT_TIME_START				
	FILE *file = fopen(filename, "wb");
      	COUNT_PARTIAL_OUTPUT_TIME_END					
	free(filename);

	int raw_filename_size = strlen(index->settings->raw_filename);
	int timeseries_size = index->settings->timeseries_size;
	int paa_segments = index->settings->paa_segments;
	int sax_bit_cardinality = index->settings->sax_bit_cardinality;
	int max_leaf_size = index->settings->max_leaf_size;
	int min_leaf_size = index->settings->min_leaf_size;
	int initial_leaf_buffer_size = index->settings->initial_leaf_buffer_size;
	unsigned long long max_total_buffer_size = index->settings->max_total_buffer_size;
	int initial_fbl_buffer_size = index->settings->initial_fbl_buffer_size;
	int total_loaded_leaves = index->settings->total_loaded_leaves;
	int tight_bound = index->settings->tight_bound;
	int aggressive_check = index->settings->aggressive_check;
	int new_index = 0;

	// SETTINGS DATA
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
	  
	COUNT_PARTIAL_OUTPUT_TIME_START
	fwrite(&leaf_nodes_count, sizeof(unsigned long), 1, file);	  
	fwrite(&raw_filename_size, sizeof(int), 1, file);
	fwrite(index->settings->raw_filename, sizeof(char), raw_filename_size, file);
	fwrite(&timeseries_size, sizeof(int), 1, file);
	fwrite(&paa_segments, sizeof(int), 1, file);
	fwrite(&sax_bit_cardinality, sizeof(int), 1, file);
	fwrite(&max_leaf_size, sizeof(int), 1, file);
	fwrite(&min_leaf_size, sizeof(int), 1, file);
	fwrite(&initial_leaf_buffer_size, sizeof(int), 1, file);
	fwrite(&max_total_buffer_size, sizeof(unsigned long long), 1, file);
	fwrite(&initial_fbl_buffer_size, sizeof(int), 1, file);
	fwrite(&total_loaded_leaves, sizeof(int), 1, file);
	fwrite(&tight_bound, sizeof(int), 1, file);
	fwrite(&aggressive_check, sizeof(int), 1, file);
      	COUNT_PARTIAL_OUTPUT_TIME_END
	  
	// FBL DATA AND NODES
	int j;
	for (j=0; j<index->fbl->number_of_buffers; j++) {
		fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
		if (current_fbl_node->initialized && current_fbl_node->node != NULL) {
			COUNT_PARTIAL_OUTPUT_TIME_START
			COUNT_PARTIAL_SEQ_OUTPUT
			fwrite(&j, sizeof(int), 1, file);
			COUNT_PARTIAL_OUTPUT_TIME_END			
			node_write(index, current_fbl_node->node, file);
		}
	}
	COUNT_PARTIAL_OUTPUT_TIME_START
        fseek(file, 0L, SEEK_SET);
        fwrite(&leaf_nodes_count, sizeof(unsigned long), 1, file);	  	  	  
	fclose(file);
	COUNT_PARTIAL_OUTPUT_TIME_END
    const char *filename_adpt = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
    filename_adpt = strcpy(filename_adpt, index->settings->root_directory);
    filename_adpt = strcat(filename_adpt, "/adaptive");
    COUNT_PARTIAL_RAND_OUTPUT
    COUNT_PARTIAL_OUTPUT_TIME_START    
    FILE *file_adpt = fopen(filename_adpt, "wb");
    COUNT_PARTIAL_RAND_OUTPUT
    COUNT_PARTIAL_RAND_OUTPUT      
    fwrite(&(index->total_records), sizeof(unsigned long long), 1, file_adpt);
    fwrite(&(index->loaded_records), sizeof(unsigned long long), 1, file_adpt);
    fclose(file_adpt);    
    COUNT_PARTIAL_OUTPUT_TIME_END
    free(filename_adpt);

}

isax_index * index_read(const char* root_directory) {
        fprintf(stderr, ">>> Loading index: %s\n", root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(root_directory) + 15));
	filename = strcpy(filename, root_directory);
	filename = strcat(filename, "/index.idx");
        COUNT_PARTIAL_RAND_INPUT
        COUNT_PARTIAL_INPUT_TIME_START    	
	FILE *file = fopen(filename, "rb");
        COUNT_PARTIAL_INPUT_TIME_END    		
	free(filename);

	int raw_filename_size = 0;
	char *raw_filename = NULL;
	unsigned long count_leaves = 0;
	int timeseries_size = 0;
	int paa_segments = 0;
	int sax_bit_cardinality = 0;
	int max_leaf_size = 0;
	int min_leaf_size = 0;
	int initial_leaf_buffer_size = 0;
	unsigned long long max_total_buffer_size = 0;
	int initial_fbl_buffer_size = 0;
	int total_loaded_leaves = 0;
	int tight_bound = 0;
	int aggressive_check = 0;
	int new_index = 0;
	
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_INPUT_TIME_START
        fread(&count_leaves, sizeof(unsigned long), 1, file);	  
	fread(&raw_filename_size, sizeof(int), 1, file);
        COUNT_PARTIAL_INPUT_TIME_END   		
	raw_filename = malloc(sizeof(char) * (raw_filename_size+1));
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_INPUT_TIME_START
	fread(raw_filename, sizeof(char), raw_filename_size, file);
        COUNT_PARTIAL_INPUT_TIME_END   			
	raw_filename[raw_filename_size] = '\0';

      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT
      	COUNT_PARTIAL_SEQ_OUTPUT			       
      	COUNT_PARTIAL_SEQ_OUTPUT

        COUNT_PARTIAL_INPUT_TIME_START	
        fread(&timeseries_size, sizeof(int), 1, file);
	fread(&paa_segments, sizeof(int), 1, file);
	fread(&sax_bit_cardinality, sizeof(int), 1, file);
	fread(&max_leaf_size, sizeof(int), 1, file);
	fread(&min_leaf_size, sizeof(int), 1, file);
	fread(&initial_leaf_buffer_size, sizeof(int), 1, file);
	fread(&max_total_buffer_size, sizeof(unsigned long long), 1, file);
	fread(&initial_fbl_buffer_size, sizeof(int), 1, file);
	fread(&total_loaded_leaves, sizeof(int), 1, file);
	fread(&tight_bound, sizeof(int), 1, file);
	fread(&aggressive_check, sizeof(int), 1, file);
        COUNT_PARTIAL_INPUT_TIME_END
	  
	isax_index_settings *idx_settings = isax_index_settings_init(root_directory,
																timeseries_size,
																paa_segments,
																sax_bit_cardinality,
																max_leaf_size,
																min_leaf_size,
																initial_leaf_buffer_size,
																max_total_buffer_size,
																initial_fbl_buffer_size,
																total_loaded_leaves,
																tight_bound,
																aggressive_check,
																0);
	idx_settings->raw_filename = malloc(sizeof(char) * 256);
	strcpy(idx_settings->raw_filename, raw_filename);
	free(raw_filename);

	isax_index *index = isax_index_init(idx_settings);
	
	index->stats->leaves_heights = calloc(count_leaves, sizeof(int));
	index->stats->leaves_sizes = calloc(count_leaves, sizeof(int));
	index->stats->leaves_counter = 0;

	
	while(!feof(file)) {
		int j = 0;
         	COUNT_PARTIAL_SEQ_INPUT			       
        	COUNT_PARTIAL_INPUT_TIME_START
		size_t bytes_read = fread(&j, sizeof(int), 1, file);
        	COUNT_PARTIAL_INPUT_TIME_END		
		if(bytes_read) {
			fbl_soft_buffer *current_buffer = &index->fbl->soft_buffers[j];
			current_buffer->initialized = 1;
			current_buffer->node = node_read(index, file);
			if(index->first_node == NULL)
			{
				index->first_node = current_buffer->node;
				current_buffer->node->next = NULL;
				current_buffer->node->previous = NULL;
			}
			else
			{
				isax_node * prev_first = index->first_node;
				index->first_node = current_buffer->node;
				index->first_node->next = prev_first;
				prev_first->previous = current_buffer->node;
			}
		}


	}
        COUNT_PARTIAL_INPUT_TIME_START
	fclose(file);
        COUNT_PARTIAL_INPUT_TIME_END
	  
    const char *filename_adpt = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
    filename_adpt = strcpy(filename_adpt, index->settings->root_directory);
    filename_adpt = strcat(filename_adpt, "/adaptive");
    COUNT_PARTIAL_SEQ_INPUT
    COUNT_PARTIAL_INPUT_TIME_START    
    FILE *file_adpt = fopen(filename_adpt, "rb");
    COUNT_PARTIAL_INPUT_TIME_END    
    free(filename_adpt);

    if(file_adpt) {
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_SEQ_INPUT      
        COUNT_PARTIAL_INPUT_TIME_START          
        fread(&index->total_records, sizeof(unsigned long long), 1, file_adpt);
        fread(&index->loaded_records, sizeof(unsigned long long), 1, file_adpt);
        fclose(file_adpt);
        COUNT_PARTIAL_INPUT_TIME_END
    }
    
	return index;
}

void create_wedges(isax_index *index, isax_node *node) {
	if(node == NULL) {
		int j;
		for (j=0; j<index->fbl->number_of_buffers; j++) {
			fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
			if (current_fbl_node->initialized && current_fbl_node->node != NULL) {
				isax_node *curr_node = current_fbl_node->node;
				create_wedges(index, curr_node);
			}
		}
		index->has_wedges = 1;
	}
	else {
		if(node->is_leaf) {
			node->wedges = malloc(sizeof(ts_type) * index->settings->timeseries_size * 2);
			int i;
			for(i=0; i<index->settings->timeseries_size; i++) {
				node->wedges[i] = MINVAL;
				node->wedges[index->settings->timeseries_size + i] = MAXVAL;
			}
			if(node->has_partial_data_file) {
				char *filename;
				file_position_type *positions;
				ts_type *tmp_ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);

				filename = malloc((strlen(node->filename) + 10) * sizeof(char));
				strcpy(filename, node->filename);
				strcat(filename, ".part");
				filename[strlen(node->filename)+5] = '\0';
                                COUNT_PARTIAL_RAND_INPUT
                                COUNT_PARTIAL_RAND_INPUT
                                COUNT_PARTIAL_RAND_INPUT
                                COUNT_PARTIAL_RAND_INPUT				  
                                COUNT_PARTIAL_INPUT_TIME_START				  
				FILE *leaf_file = fopen(filename, "rb");
				FILE *raw_file = fopen(index->settings->raw_filename, "rb");
				fseeko(leaf_file,0L, SEEK_END);
				unsigned long size = ftello(leaf_file) / index->settings->partial_record_size;
				fseeko(leaf_file, 0L, SEEK_SET);
                                COUNT_PARTIAL_INPUT_TIME_END				  

				positions = malloc(index->settings->position_byte_size * size);

				for(i=0; i<size; i++) {
                                        COUNT_PARTIAL_SEQ_INPUT
                                        COUNT_PARTIAL_RAND_INPUT
                                        COUNT_PARTIAL_INPUT_TIME_START					  
					fread(&positions[i], index->settings->position_byte_size, 1, leaf_file);
					fseeko(leaf_file, index->settings->sax_byte_size, SEEK_CUR);
                                        COUNT_PARTIAL_INPUT_TIME_END					  					
				}

				qsort(positions, size, index->settings->position_byte_size, compare_file_positions);

				for(i=0; i<size; i++) {
                                        COUNT_PARTIAL_SEQ_INPUT
                                        COUNT_PARTIAL_RAND_INPUT
                                        COUNT_PARTIAL_INPUT_TIME_START					  				  
					fseek(raw_file, positions[i] * index->settings->ts_byte_size, SEEK_SET);
					fread(tmp_ts, index->settings->ts_byte_size, 1, raw_file);
                                        COUNT_PARTIAL_INPUT_TIME_END					  					
					int j;
					for(j=0; j<index->settings->timeseries_size; j++) {
						if(tmp_ts[j] < node->wedges[index->settings->timeseries_size + j]) {
							node->wedges[index->settings->timeseries_size + j] = tmp_ts[j];
						}
						if(tmp_ts[j] > node->wedges[j]) {
							node->wedges[j] = tmp_ts[j];
						}
					}
				}
                                COUNT_PARTIAL_INPUT_TIME_START					  					
				fclose(raw_file);
				fclose(leaf_file);
                                COUNT_PARTIAL_INPUT_TIME_END					  									
				free(filename);
				free(positions);
				free(tmp_ts);
			}
			if(node->has_full_data_file) {

				char *filename;

				filename = malloc((strlen(node->filename) + 10) * sizeof(char));
				strcpy(filename, node->filename);
				strcat(filename, ".full");
				filename[strlen(node->filename)+5] = '\0';
                                COUNT_PARTIAL_RAND_INPUT
                                COUNT_PARTIAL_RAND_INPUT
                                COUNT_PARTIAL_RAND_INPUT				  
				COUNT_PARTIAL_INPUT_TIME_START
				FILE *leaf_file = fopen(filename, "rb");
				fseeko(leaf_file,0L, SEEK_END);
				unsigned long size = ftello(leaf_file) / index->settings->full_record_size;
				fseeko(leaf_file, 0L, SEEK_SET);
                                COUNT_PARTIAL_INPUT_TIME_END
				

				for(i=0; i<size; i++) {
                                        COUNT_PARTIAL_RAND_INPUT
 				        COUNT_PARTIAL_INPUT_TIME_START					  
					fseeko(leaf_file, index->settings->sax_byte_size + index->settings->position_byte_size, SEEK_CUR);
 				        COUNT_PARTIAL_INPUT_TIME_END					  					
					int j=0;
					for(j=0; j<index->settings->timeseries_size; j++) {
						ts_type tmp_val;
                                                COUNT_PARTIAL_SEQ_INPUT
 	   			                COUNT_PARTIAL_INPUT_TIME_START					  						
						fread(&tmp_val, sizeof(ts_type), 1, leaf_file);
 	   			                COUNT_PARTIAL_INPUT_TIME_END					  												
						if(tmp_val < node->wedges[index->settings->timeseries_size + j]) {
							node->wedges[index->settings->timeseries_size + j] = tmp_val;
						}
						if(tmp_val > node->wedges[j]) {
							node->wedges[j] = tmp_val;
						}
					}
				}



				///// PRINT
				/*char s[256];
				sprintf(s, "%p.WEDGE.txt", node);
				FILE *op = fopen(s, "w");

				for(i=0; i<index->settings->timeseries_size; i++) {
					fprintf(op, "%lf ", node->wedges[i]);
				}
				fprintf(op, "\n");
				fseeko(leaf_file, 0L, SEEK_SET);
				for(i=0; i<size; i++) {
					fseeko(leaf_file, index->settings->sax_byte_size + index->settings->position_byte_size, SEEK_CUR);
					fread(tmp_ts, index->settings->ts_byte_size, 1, leaf_file);
					int j;
					for(j=0; j<index->settings->timeseries_size; j++) {
						fprintf(op, "%lf ", tmp_ts[j]);
					}
					fprintf(op, "\n");
				}
				for(i=0; i<index->settings->timeseries_size; i++) {
					fprintf(op, "%lf ", node->wedges[index->settings->timeseries_size + i]);
				}

				fprintf(op, "\n");
				fclose(op);*/
				///// END PRINT

 	   			COUNT_PARTIAL_INPUT_TIME_START					  						
				fclose(leaf_file);
 	   			COUNT_PARTIAL_INPUT_TIME_END					  										
				free(filename);
			}
		}
		else {
			create_wedges(index, node->left_child);
			create_wedges(index, node->right_child);
		}
	}
}

int compare_file_positions (const void * a, const void * b)
{
   return ( *(file_position_type*)a - *(file_position_type*)b );
}

void clear_wedges(isax_index *index, isax_node *node) {
	if(node == NULL) {
		int j;
		for (j=0; j<index->fbl->number_of_buffers; j++) {
			fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
			if (current_fbl_node->initialized && current_fbl_node->node != NULL) {
				isax_node *curr_node = current_fbl_node->node;
				clear_wedges(index, curr_node);
			}
		}
		index->has_wedges = 0;
	}
	else {
		if(node->is_leaf) {
			free(node->wedges);
			node->wedges = NULL;
		}
		else {
			clear_wedges(index, node->left_child);
			clear_wedges(index, node->right_child);
		}
	}
}

void print_settings(isax_index_settings *settings) {
	fprintf(stderr,"############ ADS SETTINGS ############\n");
	fprintf(stderr,"## [FILE SETTINGS]\n");
	fprintf(stderr,"## raw_filename:\t%s\n",settings->raw_filename);
	fprintf(stderr,"## root_directory:\t%s\n",settings->root_directory);

	fprintf(stderr,"## \n## [DATA TYPE SETTINGS]\n");
	fprintf(stderr,"## timeseries_size:\t%d\n",settings->timeseries_size);
	fprintf(stderr,"## partial_record_size:\t%d\n",settings->partial_record_size);
	fprintf(stderr,"## full_record_size:\t%d\n",settings->full_record_size);
	fprintf(stderr,"## position_byte_size:\t%d\n",settings->position_byte_size);
	fprintf(stderr,"## sax_byte_size:\t%d\n",settings->sax_byte_size);
	fprintf(stderr,"## ts_byte_size:\t%d\n",settings->ts_byte_size);


	fprintf(stderr,"## \n## [BUFFER SETTINGS]\n");
	fprintf(stderr, "## initial_fbl_buffer_size:\t%d\n", settings->initial_fbl_buffer_size);
	fprintf(stderr, "## initial_leaf_buffer_size:\t%d\n", settings->initial_leaf_buffer_size);
	fprintf(stderr,"## max_total_buffer_size:\t%llu\n",settings->max_total_buffer_size);
	fprintf(stderr,"## max_total_full_buffer_size:\t%llu\n",settings->max_total_full_buffer_size);

	fprintf(stderr,"## \n## [LEAF SETTINGS]\n");
	fprintf(stderr, "## max_leaf_size:\t%d\n", settings->max_leaf_size);
	fprintf(stderr, "## min_leaf_size:\t%d\n",settings->min_leaf_size);

	fprintf(stderr,"## \n## [SAX SETTINGS]\n");
	fprintf(stderr,"## paa_segments:\t%d\n",settings->paa_segments);
	fprintf(stderr,"## sax_alphabet_card.:\t%d\n",settings->sax_alphabet_cardinality);
	fprintf(stderr,"## sax_bit_cardinality:\t%d\n",settings->sax_bit_cardinality);

	fprintf(stderr,"## \n## [QUERY ANSWERING SETTINGS]\n");
	fprintf(stderr, "## aggressive_check:\t%d\n", settings->aggressive_check);
	fprintf(stderr,"## tight_bound:\t%d\n",settings->tight_bound);
	fprintf(stderr,"## total_loaded_leaves:\t%d\n",settings->total_loaded_leaves);
	fprintf(stderr,"######################################\n");

	fflush(stderr);
}

meminfo get_memory_utilization_info(isax_index *index) {
    meminfo bytes_mem_info;
    memcpy(&bytes_mem_info, 
           &(index->memory_info), sizeof(meminfo));

    bytes_mem_info.mem_data = index->allocated_memory;
    bytes_mem_info.mem_tree_structure *= sizeof(isax_node);
    bytes_mem_info.mem_summaries *= (index->settings->paa_segments * sizeof(sax_type));
    bytes_mem_info.disk_data_full *= PAGE_SIZE;
    bytes_mem_info.disk_data_partial *= PAGE_SIZE;

    return bytes_mem_info;
}

void print_mem_info(isax_index *index) {
    meminfo bytes_mem_info = get_memory_utilization_info(index);
    printf("######################### MEMORY INFO ##########################\n");
    printf("# (MEM)     TREE STRUCTURE:     %ld\n",    bytes_mem_info.mem_tree_structure);
    printf("# (MEM)     SUMMARIZED DATA:    %ld\n",    bytes_mem_info.mem_summaries);
    printf("# (MEM)     RAW DATA:           %ld\n",    bytes_mem_info.mem_data);
    printf("# (DISK)    MATERIALIZED:       %ld\n",    bytes_mem_info.disk_data_full);
    printf("# (DISK)    PARTIAL:            %ld\n",    bytes_mem_info.disk_data_partial);
    printf("################################################################\n"); 
}



void get_query_stats(isax_index * index, ts_type * ts, ts_type * paa, int query_num)
{

  if (total_ts_count != 0)
    {
        index->stats->query_pruning_ratio = 1.0 - ((double)index->stats->query_total_checked_ts_count/
						 index->stats->total_ts_count);
        double ratio = index->stats->query_pruning_ratio;
        index->stats->queries_sum_pruning_ratio += ratio;
        index->stats->queries_sum_squares_pruning_ratio += pow(ratio,2);

        if (ratio < index->stats->queries_min_pruning_ratio)
        {
  	   index->stats->queries_min_pruning_ratio = ratio;
        }
        if (ratio > index->stats->queries_max_pruning_ratio)
        {
  	   index->stats->queries_max_pruning_ratio = ratio;
        }
    }
    

    if (index->stats->query_exact_distance != 0)
    {
        index->stats->query_eff_epsilon =   (index->stats->query_approx_distance
					    -index->stats->query_exact_distance
					    )
					    / index->stats->query_exact_distance;

        double eff_epsilon = index->stats->query_eff_epsilon;
	
        index->stats->queries_sum_eff_epsilon += eff_epsilon;

        index->stats->queries_sum_squares_eff_epsilon += pow(eff_epsilon,2);

	
        if (eff_epsilon < index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_min_eff_epsilon = eff_epsilon;
        }
        if (eff_epsilon > index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_max_eff_epsilon = eff_epsilon;
        }	
    }
    else
    {  
       ++(index->stats->eff_epsilon_queries_count);
    }


    if (index->stats->tlb_ts_count != 0)
    {
 	index->stats->query_tlb = index->stats->query_sum_tlb/index->stats->tlb_ts_count; //the average over all the data set
  
        double tlb  = index->stats->query_tlb;

        index->stats->queries_sum_tlb += tlb;
        index->stats->queries_sum_squares_tlb += pow(tlb,2);	

        if (tlb < index->stats->queries_min_tlb)
        {
  	   index->stats->queries_min_tlb = tlb;
        }
        if (tlb > index->stats->queries_min_tlb)
        {
  	   index->stats->queries_max_tlb = tlb;
        }			
    }


 /*
    unsigned long i;
    unsigned long long ts_count = 0;

    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);

    index->stats->query_tlb = 0;

    for(int i=0; i<index->sax_cache_size; i++) {
    	sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
    	float mindist = minidist_paa_to_isax_raw(paa, sax, index->settings->max_sax_cardinalities,index->settings->sax_bit_cardinality,
    						 index->settings->sax_alphabet_cardinality, index->settings->paa_segments, MINVAL, MAXVAL,
    						 index->settings->mindist_sqrt);

        fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
    	fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);

	float exact_dist = ts_euclidean_distance_non_opt(ts, ts_buffer, index->settings->timeseries_size);
        if (exact_dist != 0 )
        {
       	    index->stats->query_tlb += ((double)sqrtf(mindist)/(double)sqrtf(exact_dist));
            ++ts_count;
        }
     }
      
     if (ts_count != 0)
     {
 	index->stats->query_tlb /= ts_count; 
        index->stats->query_eff_epsilon =   (index->stats->query_approx_distance
					    -index->stats->query_exact_distance
					    )
					    / index->stats->query_exact_distance;

        double eff_epsilon = index->stats->query_eff_epsilon;
        double tlb  = index->stats->query_tlb;
	
        index->stats->queries_sum_eff_epsilon += eff_epsilon;
        index->stats->queries_sum_squares_eff_epsilon += pow(eff_epsilon,2);

        index->stats->queries_sum_tlb += tlb;
        index->stats->queries_sum_squares_tlb += pow(tlb,2);	
	
        if (eff_epsilon < index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_min_eff_epsilon = eff_epsilon;
        }
        if (eff_epsilon > index->stats->queries_min_eff_epsilon)
        {
  	   index->stats->queries_max_eff_epsilon = eff_epsilon;
        }	

        if (tlb < index->stats->queries_min_tlb)
        {
  	   index->stats->queries_min_tlb = tlb;
        }
        if (tlb > index->stats->queries_min_tlb)
        {
  	   index->stats->queries_max_tlb = tlb;
        }			
     }
*/
}

void dump_query_answer(isax_index *index, unsigned int query_num)
{
  
        //fprintf(stderr, ">>> Storing index: %s\n", index->settings->root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
	filename = strcpy(filename, index->settings->root_directory);
	filename = strcat(filename, "/answers.dump");
      	COUNT_PARTIAL_RAND_OUTPUT			       
      	COUNT_PARTIAL_OUTPUT_TIME_START				
	FILE *file = fopen(filename, "ab");
      	COUNT_PARTIAL_OUTPUT_TIME_END					
	free(filename);
	fwrite(index->answer, sizeof(ts_type), index->settings->timeseries_size, file);
	fclose(file);
}

void print_query_stats(isax_index * index, unsigned int query_num, char * queries)
{
        /*
        printf("-----------------\n");
        printf("QUERY STATISTICS \n");
        printf("QUERY: %d \n", query_num);
        printf("-----------------\n");
	*/

        printf("Query_filter_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_filter_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_filter_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_filter_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_filter_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_rand_output_count,
	       queries,
	       query_num
	       );
        printf("Query_filter_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_filter_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_filter_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_filter_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_filter_loaded_ts_count,
	       queries,
	       query_num
	       );
	

        printf("Query_refine_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_refine_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_refine_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_refine_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_refine_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_rand_output_count,
	       queries,
	       query_num
	       );

	
        printf("Query_refine_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_refine_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_refine_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_refine_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_refine_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_load_node_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_load_node_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       index->stats->query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_rand_output_count,
	       queries,
	       query_num
	       );


        printf("Query_total_checked_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_total_checked_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_checked_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_checked_ts_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_nodes_count\t%u\t%s\t%d\n",
	       index->stats->query_total_loaded_nodes_count,
	       queries,
	       query_num
	       );	

        printf("Query_total_loaded_ts_count\t%llu\t%s\t%d\n",
	       index->stats->query_total_loaded_ts_count,
	       queries,
	       query_num
	       );

        printf("Query_approx_distance\t%f\t%s\t%d\n",
	       index->stats->query_approx_distance,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_approx_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_approx_node_size\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_approx_node_level\t%u\t%s\t%d\n",
	       index->stats->query_approx_node_level,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       index->stats->query_exact_distance,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_filename\t%s\t%s\t%d\n",
	       index->stats->query_exact_node_filename,
	       queries,
	       query_num
	       );
	
        printf("Query_exact_node_size\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_size,
	       queries,
	       query_num
	       );	

        printf("Query_exact_node_level\t%u\t%s\t%d\n",
	       index->stats->query_exact_node_level,
	       queries,
	       query_num
	       );
        printf("Query_lb_distance\t%f\t%s\t%d\n",
	       index->stats->query_lb_distance,
	       queries,
	       query_num
	       );
	
        printf("Query_tlb\t%f\t%s\t%d\n",
	       index->stats->query_tlb,	       
	       queries,
	       query_num
	       );	
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	       index->stats->query_pruning_ratio,	       
	       queries,
	       query_num
	       );

        printf("Query_eff_epsilon\t%f\t%s\t%d\n",
	       index->stats->query_eff_epsilon,	       
	       queries,
	       query_num
	       );		

}


void init_stats(isax_index * index)
{
    index->stats = malloc(sizeof(struct stats_info));
    if(index->stats == NULL) {
        fprintf(stderr,"Error in dstree_index.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    /*INDEX STATISTICS*/
    index->stats->idx_building_input_time = 0;  
    index->stats->idx_building_output_time = 0;
    index->stats->idx_building_cpu_time = 0;
    index->stats->idx_building_total_time = 0;

    index->stats->idx_building_seq_input_count = 0;
    index->stats->idx_building_seq_output_count = 0;
    index->stats->idx_building_rand_input_count = 0;
    index->stats->idx_building_rand_output_count = 0;    

    index->stats->idx_writing_input_time = 0;  
    index->stats->idx_writing_output_time = 0;
    index->stats->idx_writing_cpu_time = 0;
    index->stats->idx_writing_total_time = 0;

    index->stats->idx_writing_seq_input_count = 0;
    index->stats->idx_writing_seq_output_count = 0;
    index->stats->idx_writing_rand_input_count = 0;
    index->stats->idx_writing_rand_output_count = 0;

    index->stats->idx_reading_input_time = 0;  
    index->stats->idx_reading_output_time = 0;
    index->stats->idx_reading_cpu_time = 0;
    index->stats->idx_reading_total_time = 0;
  
    index->stats->idx_reading_seq_input_count = 0;
    index->stats->idx_reading_seq_output_count = 0;
    index->stats->idx_reading_rand_input_count = 0;
    index->stats->idx_reading_rand_output_count = 0;
    
    index->stats->idx_total_input_time = 0;
    index->stats->idx_total_output_time = 0;
    index->stats->idx_total_cpu_time = 0;
    index->stats->idx_total_time = 0;

    index->stats->idx_total_seq_input_count = 0;
    index->stats->idx_total_seq_output_count = 0;
    index->stats->idx_total_rand_input_count = 0;
    index->stats->idx_total_rand_output_count = 0;    

    index->stats->total_nodes_count = 0;
    index->stats->leaf_nodes_count = 0;
    index->stats->empty_leaf_nodes_count = 0;
  
    index->stats->idx_size_bytes = 0;
    index->stats->idx_size_blocks = 0;
    
    index->stats->min_fill_factor = FLT_MAX;
    index->stats->max_fill_factor = 0;
    index->stats->sum_fill_factor = 0;
    index->stats->sum_squares_fill_factor = 0;
    index->stats->avg_fill_factor = 0;
    index->stats->sd_fill_factor = 0;

    index->stats->min_height = FLT_MAX;
    index->stats->max_height = 0;
    index->stats->sum_height = 0;
    index->stats->sum_squares_height = 0;    
    index->stats->avg_height = 0;
    index->stats->sd_height = 0;
    
    /*PER QUERY STATISTICS*/    
    index->stats->query_filter_input_time = 0;
    index->stats->query_filter_output_time = 0;
    index->stats->query_filter_load_node_time = 0;
    index->stats->query_filter_cpu_time = 0;    
    index->stats->query_filter_total_time = 0;    

    index->stats->query_filter_seq_input_count = 0;
    index->stats->query_filter_seq_output_count = 0;
    index->stats->query_filter_rand_input_count = 0;
    index->stats->query_filter_rand_output_count = 0;    

    index->stats->query_filter_loaded_nodes_count = 0;
    index->stats->query_filter_checked_nodes_count = 0;    
    index->stats->query_filter_loaded_ts_count = 0;
    index->stats->query_filter_checked_ts_count = 0;

    index->stats->query_refine_input_time = 0;
    index->stats->query_refine_output_time = 0;
    index->stats->query_refine_load_node_time = 0;
    index->stats->query_refine_cpu_time = 0;
    index->stats->query_refine_total_time = 0;    

    index->stats->query_refine_seq_input_count = 0;
    index->stats->query_refine_seq_output_count = 0;
    index->stats->query_refine_rand_input_count = 0;
    index->stats->query_refine_rand_output_count = 0;    

    index->stats->query_refine_loaded_nodes_count = 0;
    index->stats->query_refine_checked_nodes_count = 0;    
    index->stats->query_refine_loaded_ts_count = 0;
    index->stats->query_refine_checked_ts_count = 0;

    index->stats->query_total_input_time = 0;
    index->stats->query_total_output_time = 0;
    index->stats->query_total_load_node_time = 0;
    index->stats->query_total_cpu_time = 0;
    index->stats->query_total_time = 0;    

    index->stats->query_total_seq_input_count = 0;
    index->stats->query_total_seq_output_count = 0;
    index->stats->query_total_rand_input_count = 0;
    index->stats->query_total_rand_output_count = 0;
    
    index->stats->query_total_loaded_nodes_count = 0;
    index->stats->query_total_checked_nodes_count = 0;    
    index->stats->query_total_loaded_ts_count = 0;
    index->stats->query_total_checked_ts_count = 0;

    
    index->stats->query_exact_distance = 0;
    index->stats->query_exact_node_filename = NULL;
    index->stats->query_exact_node_size = 0;    
    index->stats->query_exact_node_level = 0;

    index->stats->query_approx_distance = 0;
    index->stats->query_approx_node_filename = NULL;
    index->stats->query_approx_node_size = 0;
    index->stats->query_approx_node_level = 0;

    index->stats->query_lb_distance = 0;

    index->stats->query_tlb = 0;        
    index->stats->query_eff_epsilon = 0;    
    index->stats->query_pruning_ratio = 0;

    
    /*SUMMARY STATISTICS FOR ALL QUERIES*/        
    index->stats->queries_refine_input_time = 0;
    index->stats->queries_refine_output_time = 0;
    index->stats->queries_refine_load_node_time = 0;
    index->stats->queries_refine_cpu_time = 0;
    index->stats->queries_refine_total_time = 0;    

    index->stats->queries_refine_seq_input_count = 0;
    index->stats->queries_refine_seq_output_count = 0;
    index->stats->queries_refine_rand_input_count = 0;
    index->stats->queries_refine_rand_output_count = 0;        
    
    index->stats->queries_filter_input_time = 0;
    index->stats->queries_filter_output_time = 0;
    index->stats->queries_filter_load_node_time = 0;
    index->stats->queries_filter_cpu_time = 0;
    index->stats->queries_filter_total_time = 0;    

    index->stats->queries_filter_seq_input_count = 0;
    index->stats->queries_filter_seq_output_count = 0;
    index->stats->queries_filter_rand_input_count = 0;
    index->stats->queries_filter_rand_output_count = 0;    

    index->stats->queries_total_input_time = 0;
    index->stats->queries_total_output_time = 0;
    index->stats->queries_total_load_node_time = 0;    
    index->stats->queries_total_cpu_time = 0;
    index->stats->queries_total_time = 0;    

    index->stats->queries_total_seq_input_count = 0;
    index->stats->queries_total_seq_output_count = 0;
    index->stats->queries_total_rand_input_count = 0;
    index->stats->queries_total_rand_output_count = 0;        

    index->stats->queries_min_eff_epsilon = FLT_MAX;
    index->stats->queries_max_eff_epsilon = 0;
    index->stats->queries_sum_eff_epsilon = 0;
    index->stats->queries_sum_squares_eff_epsilon = 0;
    index->stats->queries_avg_eff_epsilon = 0;
    index->stats->queries_sd_eff_epsilon = 0;
    
    index->stats->queries_min_pruning_ratio =  FLT_MAX;
    index->stats->queries_max_pruning_ratio = 0;
    index->stats->queries_sum_pruning_ratio = 0;
    index->stats->queries_sum_squares_pruning_ratio = 0;
    index->stats->queries_avg_pruning_ratio = 0;
    index->stats->queries_sd_pruning_ratio = 0;

    index->stats->queries_min_tlb =  FLT_MAX;
    index->stats->queries_max_tlb = 0;
    index->stats->queries_sum_tlb = 0;
    index->stats->queries_sum_squares_tlb = 0;
    index->stats->queries_avg_tlb = 0;
    index->stats->queries_sd_tlb = 0;    
    
    index->stats->tlb_ts_count = 0;
    index->stats->eff_epsilon_queries_count = 0;

    
    /*COMBINED STATISTICS FOR INDEXING AND QUERY WORKLOAD*/            
    index->stats->total_input_time = 0;
    index->stats->total_output_time = 0;
    index->stats->total_load_node_time = 0;
    index->stats->total_cpu_time = 0;
    index->stats->total_time = 0;
    index->stats->total_time_sanity_check = 0;
    
    index->stats->total_seq_input_count = 0;
    index->stats->total_seq_output_count = 0;
    index->stats->total_rand_input_count = 0;
    index->stats->total_rand_output_count = 0;
    
    index->stats->total_parse_time = 0;	
    index->stats->total_ts_count = 0;	
    
    return SUCCESS;
}

void get_index_stats(isax_index * index)
{
  index->stats->total_seq_input_count = index->stats->idx_building_seq_input_count
                                      + index->stats->idx_writing_seq_input_count
                                      + index->stats->idx_reading_seq_input_count;

  index->stats->total_seq_output_count = index->stats->idx_building_seq_output_count
                                      + index->stats->idx_writing_seq_output_count
                                      + index->stats->idx_reading_seq_output_count;

  index->stats->total_rand_input_count = index->stats->idx_building_rand_input_count
                                      + index->stats->idx_writing_rand_input_count
                                      + index->stats->idx_reading_rand_input_count;

  index->stats->total_rand_output_count = index->stats->idx_building_rand_output_count
                                      + index->stats->idx_writing_rand_output_count
                                      + index->stats->idx_reading_rand_output_count;

  
  index->stats->total_input_time = index->stats->idx_building_input_time
                                 + index->stats->idx_writing_input_time
                                  + index->stats->idx_reading_input_time;

  index->stats->total_output_time = index->stats->idx_building_output_time
                                  + index->stats->idx_writing_output_time
                                  + index->stats->idx_reading_output_time;

  index->stats->total_cpu_time    = index->stats->idx_building_cpu_time
                                  + index->stats->idx_writing_cpu_time
                                  + index->stats->idx_reading_cpu_time;


  index->stats->total_time    = index->stats->total_input_time
                              + index->stats->total_output_time
                              + index->stats->total_cpu_time;

  index->stats->total_parse_time = total_parse_time;


  index->stats->leaf_nodes_count = leaf_nodes_count;
  index->stats->empty_leaf_nodes_count = empty_leaf_nodes_count;
  

  index->stats->total_nodes_count = total_nodes_count;
  index->stats->total_ts_count = total_ts_count;

  get_index_footprint(index);

}


void get_index_footprint(isax_index * index)
{

    const char *filename = malloc(sizeof(char) * (strlen(index->settings->root_directory) + 15));
    filename = strcpy(filename, index->settings->root_directory);
    filename = strcat(filename, "/index.idx");

    struct stat st;
    unsigned int  count_leaves;

    if (stat(filename, &st) == 0)
    {
      index->stats->idx_size_bytes = (long long) st.st_size;
      index->stats->idx_size_blocks = (long long) st.st_blksize;
    }

    count_leaves = index->stats->leaf_nodes_count;
    
    index->stats->avg_fill_factor =  ((double) index->stats->sum_fill_factor) / count_leaves  ;
    index->stats->sum_squares_fill_factor -= (pow( index->stats->sum_fill_factor,2) / count_leaves);
    index->stats->sd_fill_factor =  sqrt(((double) index->stats->sum_squares_fill_factor) / count_leaves);

    index->stats->avg_height     =  ((double) index->stats->sum_height) / count_leaves;
    index->stats->sum_squares_height -= (pow( index->stats->sum_height,2) / count_leaves);
    index->stats->sd_height =  sqrt(((double) index->stats->sum_squares_height) / count_leaves);

    free(filename);
}

  
void print_index_stats(isax_index * index, char * dataset)
{
      //  id = -1 for index and id = query_id for queries
        int id = -1;
        printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_output_time/1000000,
	       dataset,
	       id);
        printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_building_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_building_rand_output_count,
	       dataset,
	       id); 

        printf("Index_writing_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_writing_input_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_writing_output_time/1000000,
	       dataset,
	       id);

        printf("Index_writing_cpu_time_secs\t%lf\t%s\t%d\n",
               index->stats->idx_writing_cpu_time/1000000,
	       dataset,
	       id);
	
        printf("Index_writing_total_time_secs\t%lf\t%s\t%d\n",
               index->stats->idx_writing_total_time/1000000,
	       dataset,
	       id);	

        printf("Index_writing_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_seq_input_count,
	       dataset,
	       id);

        printf("Index_writing_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_seq_output_count,
	       dataset,
	       id);

        printf("Index_writing_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_rand_input_count,
	       dataset,
	       id);

        printf("Index_writing_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_writing_rand_output_count,
	       dataset,
	       id);	

        printf("Index_reading_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_reading_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_output_time/1000000,
	       dataset,
	       id);
        printf("Index_reading_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_reading_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->idx_reading_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_reading_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_reading_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->idx_reading_rand_output_count,
	       dataset,
	       id);
	
        printf("Index_total_input_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_total_output_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_total_time_secs\t%lf\t%s\t%d\n",
	       index->stats->total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_total_seq_input_count\t%llu\t%s\t%d\n",
	       index->stats->total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_total_seq_output_count\t%llu\t%s\t%d\n",
	       index->stats->total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_input_count\t%llu\t%s\t%d\n",
	       index->stats->total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_output_count\t%llu\t%s\t%d\n",
	       index->stats->total_rand_output_count,
	       dataset,
	       id); 

        printf("Internal_nodes_count\t%lu\t%s\t%d\n",
	       (index->stats->total_nodes_count - index->stats->leaf_nodes_count),
	       dataset,
	       id);

        printf("Leaf_nodes_count\t%lu\t%s\t%d\n",
	       index->stats->leaf_nodes_count,
	       dataset,
	       id);

	printf("Empty_leaf_nodes_count\t%lu\t%s\t%d\n",
               index->stats->empty_leaf_nodes_count,	       
	       dataset,
	       id);

	printf("Total_nodes_count\t%lu\t%s\t%d\n",
	       index->stats->total_nodes_count,
	       dataset,
	       id);

	double size_MB =  (index->stats->idx_size_bytes)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);

	printf("Minimum_fill_factor\t%f\t%s\t%d\n",
	       index->stats->min_fill_factor,	       
	       dataset,
	       id);	

	printf("Maximum_fill_factor\t%f\t%s\t%d\n",
	       index->stats->max_fill_factor,	       
	       dataset,
	       id);

	printf("Average_fill_factor\t%f\t%s\t%d\n",
	       index->stats->avg_fill_factor,	       
	       dataset,
	       id);	

	printf("SD_fill_factor\t%f\t%s\t%d\n",
	       index->stats->sd_fill_factor,	       
	       dataset,
	       id);		

	printf("Minimum_height\t%u\t%s\t%d\n",
	       index->stats->min_height,	       
	       dataset,
	       id);	

	printf("Maximum_height\t%u\t%s\t%d\n",
	       index->stats->max_height,	       
	       dataset,
	       id);

	printf("Average_height\t%f\t%s\t%d\n",
	       index->stats->avg_height,	       
	       dataset,
	       id);	

	printf("SD_height\t%f\t%s\t%d\n",
	       index->stats->sd_height,	       
	       dataset,
	       id);			

	printf("Total_ts_count\t%u\t%s\t%d\n",
	       index->stats->total_ts_count,	       
	       dataset,
	       id);

	for (int i = 0; i < index->stats->leaves_counter; ++i)	  
	{
 	    double fill_factor = ((double) index->stats->leaves_sizes[i])/index->settings->max_leaf_size;
  	    printf("Leaf_report_node_%d \t Height  %d  \t%s\t%d\n",
		   (i+1),
		   index->stats->leaves_heights[i],
	           dataset,
	           id);
  	    printf("Leaf_report_node_%d \t Fill_Factor  %f \t%s\t%d\n",
		   (i+1),
		   fill_factor,	       
	           dataset,
	           id);	    
	}	

}


void get_queries_stats(isax_index * index, unsigned int queries_count)
{

  //total_non_zero_queries_count can be different from total_queries_count
  //since the former only includes queries with non zero distance so
  //we can calculate the effective epsilon
  
  if (queries_count != 0)
  {
    index->stats->queries_avg_pruning_ratio =  ((double) index->stats->queries_sum_pruning_ratio)
                                               / queries_count ;
    index->stats->queries_sum_squares_pruning_ratio -= (pow(index->stats->queries_sum_pruning_ratio,2)
							/queries_count);
    index->stats->queries_sd_pruning_ratio =  sqrt(((double) index->stats->queries_sum_squares_pruning_ratio)
						   / queries_count);
    index->stats->queries_avg_checked_nodes_count /= queries_count; 
    index->stats->queries_avg_checked_ts_count /= queries_count;
    index->stats->queries_avg_loaded_nodes_count /= queries_count; 
    index->stats->queries_avg_loaded_ts_count /= queries_count;     

    index->stats->queries_avg_tlb =  ((double) index->stats->queries_sum_tlb)
                                               / queries_count  ;
    index->stats->queries_sum_squares_tlb -= (pow(index->stats->queries_sum_tlb,2)
							/queries_count );
    index->stats->queries_sd_tlb =  sqrt(((double) index->stats->queries_sum_squares_tlb)
						   / queries_count);            
  }
  
  
  if ((queries_count - index->stats->eff_epsilon_queries_count) != 0)
  {
    index->stats->queries_avg_eff_epsilon =  ((double) index->stats->queries_sum_eff_epsilon)
                                               / (queries_count - index->stats->eff_epsilon_queries_count)  ;
    index->stats->queries_sum_squares_eff_epsilon -= (pow(index->stats->queries_sum_eff_epsilon,2)
							/(queries_count - index->stats->eff_epsilon_queries_count) );
    index->stats->queries_sd_eff_epsilon =  sqrt(((double) index->stats->queries_sum_squares_eff_epsilon)
						   / (queries_count - index->stats->eff_epsilon_queries_count));
  }
   
}


void print_queries_stats(isax_index * index, int count_queries)
{

        printf("Queries_filter_input_time_secs \t  %f \n", index->stats->queries_filter_input_time/1000000); 
        printf("Queries_filter_output_time_secs \t  %f \n", index->stats->queries_filter_output_time/1000000); 
        printf("Queries_filter_load_node_time_secs \t  %f \n", index->stats->queries_filter_load_node_time/1000000);
        printf("Queries_filter_cpu_time_secs \t  %f \n", index->stats->queries_filter_cpu_time/1000000);
        printf("Queries_filter_total_time_secs \t  %f \n", index->stats->queries_filter_total_time/1000000);

        printf("Queries_filter_seq_input_count \t  %llu  \n", index->stats->queries_filter_seq_input_count); 
        printf("Queries_filter_seq_output_count \t  %llu \n", index->stats->queries_filter_seq_output_count);
        printf("Queries_filter_rand_input_count \t  %llu  \n", index->stats->queries_filter_rand_input_count); 
        printf("Queries_filter_rand_onput_count \t  %llu \n", index->stats->queries_filter_rand_output_count);
	
        printf("Queries_refine_input_time_secs \t  %f \n", index->stats->queries_refine_input_time/1000000); 
        printf("Queries_refine_output_time_secs \t  %f \n", index->stats->queries_refine_output_time/1000000); 
        printf("Queries_refine_load_node_time_secs \t  %f \n", index->stats->queries_refine_load_node_time/1000000);
        printf("Queries_refine_cpu_time_secs \t  %f \n", index->stats->queries_refine_cpu_time/1000000);
        printf("Queries_refine_total_time_secs \t  %f \n", index->stats->queries_refine_total_time/1000000);

        printf("Queries_refine_seq_input_count \t  %llu  \n", index->stats->queries_refine_seq_input_count); 
        printf("Queries_refine_seq_output_count \t  %llu \n", index->stats->queries_refine_seq_output_count);
        printf("Queries_refine_rand_input_count \t  %llu  \n", index->stats->queries_refine_rand_input_count); 
        printf("Queries_refine_rand_onput_count \t  %llu \n", index->stats->queries_refine_rand_output_count);

        printf("Queries_total_input_time_secs \t  %f \n", index->stats->queries_total_input_time/1000000); 
        printf("Queries_total_output_time_secs \t  %f \n", index->stats->queries_total_output_time/1000000); 
        printf("Queries_total_Load_node_time_secs \t  %f \n", index->stats->total_load_node_time/1000000);
        printf("Queries_total_cpu_time_secs \t  %f \n", index->stats->queries_total_cpu_time/1000000);
        printf("Queries_total_Total_time_secs \t  %f \n", index->stats->queries_total_time/1000000);	

        printf("Queries_total_seq_input_count \t  %llu  \n", index->stats->queries_total_seq_input_count); 
        printf("Queries_total_seq_output_count \t  %llu \n", index->stats->queries_total_seq_output_count);
        printf("Queries_total_rand_input_count \t  %llu  \n", index->stats->queries_total_rand_input_count); 
        printf("Queries_total_rand_onput_count \t  %llu \n", index->stats->queries_total_rand_output_count);

        printf("Queries_avg_checked_nodes_count \t  %f \n", index->stats->queries_avg_checked_nodes_count);  
        printf("Queries_avg_checked_ts_count \t  %f \n", index->stats->queries_avg_checked_ts_count);
        printf("Queries_avg_loaded_nodes_count \t  %f \n", index->stats->queries_avg_loaded_nodes_count);  
        printf("Queries_avg_loaded_ts_count \t  %f \n", index->stats->queries_avg_loaded_ts_count);
	
	printf("Queries_min_tlb \t  %f \n", index->stats->queries_min_tlb);
	printf("Queries_max_tlb \t  %f \n", index->stats->queries_max_tlb);
	printf("Queries_avg_tlb \t  %f \n", index->stats->queries_avg_tlb);
	printf("Queries_sd_tlb \t  %f \n", index->stats->queries_sd_tlb);
	
	printf("Queries_min_pruning_ratio \t  %f \n", index->stats->queries_min_pruning_ratio);
	printf("Queries_max_pruning_ratio \t  %f \n", index->stats->queries_max_pruning_ratio);
	printf("Queries_avg_pruning_ratio \t  %f \n", index->stats->queries_avg_pruning_ratio);
	printf("Queries_sd_pruning_ratio \t  %f \n", index->stats->queries_sd_pruning_ratio);

	printf("Queries_min_eff_epsilon \t  %f \n", index->stats->queries_min_eff_epsilon);
	printf("Queries_max_eff_epsilon \t  %f \n", index->stats->queries_max_eff_epsilon);
	printf("Queries_avg_eff_epsilon \t  %f \n", index->stats->queries_avg_eff_epsilon);
	printf("Queries_sd_eff_epsilon \t  %f \n", index->stats->queries_sd_eff_epsilon);
	
        printf("Total Combined indexing and querying times \t  %f \n",
	       (index->stats->queries_total_time+index->stats->idx_total_time)/1000000); 

}


void update_index_stats(isax_index *index, isax_node *node)
{

  unsigned int height = node->level +1;
  unsigned int threshold = index->settings->max_leaf_size;
  //at this point all time series are on disk
  unsigned int node_size = node->leaf_size;

  double node_fill_factor = (node_size * 100.0)/threshold;  
    
  if (node_fill_factor < index->stats->min_fill_factor)
  {
    index->stats->min_fill_factor = node_fill_factor;
  }
  if (node_fill_factor > index->stats->max_fill_factor)
  {
    index->stats->max_fill_factor = node_fill_factor;
  }
  if (height < index->stats->min_height)
  {
    index->stats->min_height = height;    
  }
  if (height > index->stats->max_height)
  {
    index->stats->max_height = height;
  }
  
  if(node_size == 0)
  {
    COUNT_EMPTY_LEAF_NODE
   }
  index->stats->sum_fill_factor += node_fill_factor ;
  index->stats->sum_squares_fill_factor += pow(node_fill_factor,2) ;

  index->stats->sum_height += height;
  index->stats->sum_squares_height += pow(height,2) ;
     
  COUNT_TOTAL_TS(node_size)
  
}

void print_tlb_stats(isax_index * index, unsigned int query_num, char * queries)
{

        printf("Query_avg_data_tlb\t%lf\t%s\t%u\n",
	       total_data_tlb/total_ts_count,	       
	       queries,
	       query_num
	     );	
	printf("Total_ts_count\t%lu\t%s\t%u\n",
	       total_ts_count,	       
	       queries,
	       query_num);
	
}
