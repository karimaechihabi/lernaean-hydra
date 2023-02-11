//
//
//  dstree_file_buffer.c
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/dstree_file_buffer.h"
#include "../include/dstree_file_buffer_manager.h"
#include "../include/dstree_index.h"
#include "../include/calc_utils.h"

enum response dstree_file_buffer_init(struct dstree_node *node)
{

  node->file_buffer = NULL;
  node->file_buffer = malloc(sizeof(struct dstree_file_buffer));

  if(node->file_buffer == NULL) {
    fprintf(stderr,"Error in dstree_file_buffer.c: Could not allocate memory for file buffer.\n");    
    return FAILURE;	
  }

  //  node->file_buffer->filename = NULL;
  
  node->file_buffer->in_disk = false;
  node->file_buffer->disk_count = 0;
  
  node->file_buffer->buffered_list = NULL;
  node->file_buffer->buffered_list_size = 0;

  node->file_buffer->node = node;
  node->file_buffer->position_in_map = NULL;
  
  node->file_buffer->do_not_flush = false;
  
  return SUCCESS;
  
}

ts_type ** get_all_time_series_in_node(struct dstree_index * index, struct dstree_node * node) 
{

  int ts_length = index->settings->timeseries_size;
  int max_leaf_size = index->settings->max_leaf_size;
  ts_type ** ret = NULL;
   ret = calloc(max_leaf_size, sizeof(ts_type *));
     
  if (node->file_buffer->disk_count > 0)
  {
      
      if(node->filename == NULL)
      {   
        fprintf(stderr, "Error in dstree_file_buffer.c: This node has data on disk but"
                        "could not get its filename.\n");
        //return FAILURE;
      }
       
     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1;
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);
     full_filename = strcat(full_filename, "\0");

     COUNT_PARTIAL_RAND_INPUT
     COUNT_PARTIAL_INPUT_TIME_START
     FILE *ts_file = fopen(full_filename, "r");
     COUNT_PARTIAL_INPUT_TIME_END
       
     if(ts_file == NULL)
     {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Could not open"
		"the filename %s. Reason = %s\n", full_filename, strerror(errno));
        //return FAILURE;
     }

     //in order to keep the same order that the data was inserted, we move the
     //time series that are in memory to allw the disk based time series to be
     //first in the buffer.
     

     
     for (int i=0; i<node->file_buffer->disk_count;++i ) 
     {       
       //node->file_buffer->buffered_list[idx] = malloc(sizeof(ts_type) * ts_length);
       ret[i] = calloc(ts_length, sizeof(ts_type));
       
       //ret[i] = (ts_type *) index->buffer_manager->current_record;
       //index->buffer_manager->current_record += sizeof(ts_type) * ts_length;
       //index->buffer_manager->current_record_index++;

       COUNT_PARTIAL_SEQ_INPUT
       COUNT_PARTIAL_INPUT_TIME_START      
       fread(ret[i],
	     sizeof(ts_type),ts_length, ts_file);
       COUNT_PARTIAL_INPUT_TIME_END       
     }
     
     int idx = node->file_buffer->buffered_list_size;
 
     for (int i = 0 ; i < idx; ++i)
     {
         ret[i+(node->file_buffer->disk_count)] = calloc(ts_length, sizeof(ts_type));
	 for(int j=0; j<ts_length; ++j)
         {
	   ret[i+(node->file_buffer->disk_count)][j] = node->file_buffer->buffered_list[i][j];
         }
     }

     //this should be equal to old size + disk_count
     //node->file_buffer->buffered_list_size = idx+node->file_buffer->disk_count;

      
     COUNT_PARTIAL_INPUT_TIME_START
     if(fclose(ts_file))
     {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Could not close"
		"the filename %s. Reason= %s.\n", full_filename, strerror(errno));
        //return FAILURE;
     } 
     COUNT_PARTIAL_INPUT_TIME_END    
     free(full_filename);

  }
  else
  {
     int idx = node->file_buffer->buffered_list_size;

     for (int i = 0 ; i < idx; ++i)
     {
         ret[i] = calloc(ts_length, sizeof(ts_type));
	 for(int j=0; j<ts_length; ++j)
         {
	   ret[i][j] = node->file_buffer->buffered_list[i][j];
         }
     }    
  }
  return ret;
}



enum response flush_buffer_to_disk(struct dstree_index *index, struct dstree_node *node)
{
  //is this file flush properly out1/06_R_0_(160,192,0.738156)_9
  if (node->file_buffer->buffered_list_size > 0 )
  {

    if(node->filename == NULL)
    {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Cannot flush the node to disk. "
                        "It does not have a filename. \n");
        return FAILURE;
    }
  
     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1;
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);
     full_filename = strcat(full_filename, "\0");

     COUNT_PARTIAL_RAND_OUTPUT
     COUNT_PARTIAL_OUTPUT_TIME_START
     FILE *ts_file = fopen(full_filename, "a");
     COUNT_PARTIAL_OUTPUT_TIME_END
       
    if(ts_file == NULL)
    {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Flushing node to disk.."
		"Could not open the filename %s. Reason= %s\n", node->filename, strerror(errno));
        //return FAILURE;
        return SUCCESS;
    }

    int num_ts =  node->file_buffer->buffered_list_size;

    COUNT_PARTIAL_OUTPUT_TIME_START	
    for (int idx = 0; idx < num_ts;++idx )
    {
      COUNT_PARTIAL_SEQ_OUTPUT
      if(!fwrite(node->file_buffer->buffered_list[idx], sizeof(ts_type), index->settings->timeseries_size, ts_file))
      {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Could not "
                         "write the timeseries to file %s.\n", full_filename);
        return FAILURE;
     }

    }

    if(fclose(ts_file))
     {   
        fprintf(stderr, "Error in dstree_file_buffer.c: Flushing node to disk.. "
		"Could not close the filename %s. Reason = %s.\n", full_filename, strerror(errno));
        return FAILURE;
     }
    COUNT_PARTIAL_OUTPUT_TIME_END
      
    node->file_buffer->disk_count += num_ts;

    if (!clear_file_buffer(index, node))
    {
        fprintf(stderr, "Error in dstree_file_buffer.c: Flushing node to disk.. "
                         "Could not clear the buffer for %s.\n", full_filename);
        return FAILURE;      
    }
    
    node->file_buffer->in_disk = true;
    
    free(full_filename);
  }

    return SUCCESS; 
    
}

enum response clear_file_buffer(struct dstree_index *index, struct dstree_node * node)
{

  if ((node->file_buffer) == NULL )
  {
      fprintf(stderr, "Error in dstree_file_buffer.c: Cannot clear a NULL buffer.\n");
      return FAILURE;          
  }
  else
  {
      if (node->file_buffer->buffered_list != NULL)
      {
         free(node->file_buffer->buffered_list);
      }
      
      node->file_buffer->buffered_list = NULL;
      node->file_buffer->buffered_list_size = 0;
 }

 return SUCCESS;  
}


enum response delete_file_buffer(struct dstree_index * index,struct dstree_node * node)
{
  
  if (node->file_buffer->in_disk) //delete file if in disk
  {    
    //delete file from disk, return and error if not removed properly
    int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1;
     
    const char *full_filename = malloc(sizeof(char) * full_size);
    full_filename = strcpy(full_filename, index->settings->root_directory);
    full_filename = strcat(full_filename, node->filename);
    full_filename = strcat(full_filename, "\0");

    if(!remove(full_filename)) //file deleted successfully
    {
      node->file_buffer->disk_count = 0;
      node->file_buffer->in_disk = false;
    }
    else 
    {
      fprintf(stderr, "Error in dstree_file_buffer.c: Error deleting filename %s.\n", full_filename);
      return FAILURE;          
    }
    free(full_filename);  
  }
  
    struct dstree_file_map * res = node->file_buffer->position_in_map;
    
    if (res != NULL)
    {
       if (res->prev == NULL) //first element in file map 
       { 
         index->buffer_manager->file_map = res->next;
         if(res->next != NULL) //deleting the first and there are others elements in map
         {
   	  res->next->prev = NULL;
         }
	 else  //deleting first and only element
	 {
	   index->buffer_manager->file_map_tail = NULL;
	 }
       }
       else if (res->next == NULL) //deleting the last element in the map
       {
         res->prev->next = NULL;
	 index->buffer_manager->file_map_tail = res->prev;   
       }
       else
       {
         res->prev->next = res->next;
         res->next->prev = res->prev;      
       }

       free(res);
       res = NULL;
       --index->buffer_manager->file_map_size;
    
       if (!clear_file_buffer(index, node))
       {
         fprintf(stderr, "Error in dstree_file_buffer.c: Deleting node.. "
                          "Could not clear the buffer for %s.\n", node->filename);
         return FAILURE;      
       }

   }

  free(node->filename);
  node->filename = NULL;  

  free(node->file_buffer);
  node->file_buffer = NULL;

  return SUCCESS;  
}
