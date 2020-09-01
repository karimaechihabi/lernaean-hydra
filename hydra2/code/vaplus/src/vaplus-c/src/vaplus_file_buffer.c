//
//
//  vaplus_file_buffer.c
//  vaplus C version
//
//  Created by Karima Echihabi on 18/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/vaplus_file_buffer.h"
#include "../include/vaplus_file_buffer_manager.h"
#include "../include/vaplus_index.h"
#include "../include/calc_utils.h"

enum response vaplus_file_buffer_init(struct vaplus_node *node)
{

  node->file_buffer = NULL;
  node->file_buffer = malloc(sizeof(struct vaplus_file_buffer));

  if(node->file_buffer == NULL) {
    fprintf(stderr,"Error in vaplus_file_buffer.c: Could not allocate memory for file buffer.\n");    
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


struct vaplus_record * get_all_time_series_in_node(struct vaplus_index * index, struct vaplus_node * node) 
{

  unsigned int ts_length = index->settings->timeseries_size;
  unsigned int max_leaf_size = index->settings->max_leaf_size;
  unsigned int transforms_size = index->settings->fft_size;
  
  struct vaplus_record * ret = NULL;
  
  ret = calloc(max_leaf_size, sizeof(struct vaplus_record) );
     
  if (node->file_buffer->disk_count > 0)
  {      
      if(node->filename == NULL)
      {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: This node has data on disk but"
                        "could not get its filename.\n");
        //return FAILURE;
      }

     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1; 
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);

     const char * ts_file_name = malloc(sizeof(char) * (full_size + 5) );;

     ts_file_name = strcpy(ts_file_name,full_filename);     
     ts_file_name = strcat(ts_file_name, ".ts\0");
                
     COUNT_PARTIAL_RAND_INPUT
     COUNT_PARTIAL_INPUT_TIME_START
     FILE *ts_file = fopen(ts_file_name, "r");
     COUNT_PARTIAL_INPUT_TIME_END
       
     if(ts_file == NULL)
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not open"
		"the filename %s. Reason = %s\n", ts_file_name, strerror(errno));
        //return FAILURE;
     }

     //in order to keep the same order that the data was inserted, we move the
     //time series that are in memory to allw the disk based time series to be
     //first in the buffer.
     
     
     for (int i=0; i<node->file_buffer->disk_count;++i ) 
     {       
       ret[i].timeseries = calloc(ts_length, sizeof(ts_type));
       
       COUNT_PARTIAL_SEQ_INPUT
       COUNT_PARTIAL_INPUT_TIME_START      
       fread(ret[i].timeseries,
	     sizeof(ts_type),ts_length, ts_file);
       COUNT_PARTIAL_INPUT_TIME_END       
     }

     //adding to ret the data in memory so that the
     //split data is re-inserted in the same order it was
     //inserted the first time
     
     int idx = node->file_buffer->buffered_list_size;
 
     for (int i = 0 ; i < idx; ++i)
     {
         int k = i+(node->file_buffer->disk_count);
	 
         ret[k].timeseries = calloc(ts_length, sizeof(ts_type));

	 for (int g = 0; g < ts_length;++g)
	 {
	   ret[k].timeseries[g] = node->file_buffer->buffered_list[i].timeseries[g];
	 }
     }

     //this should be equal to old size + disk_count
     //node->file_buffer->buffered_list_size = idx+node->file_buffer->disk_count;

      
     COUNT_PARTIAL_INPUT_TIME_START
     if(fclose(ts_file))
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not close"
		"the filename %s. Reason= %s.\n", ts_file_name, strerror(errno));
        //return FAILURE;
     } 
     COUNT_PARTIAL_INPUT_TIME_END    
     free(ts_file_name);
  }
  else
  {
     int idx = node->file_buffer->buffered_list_size;
 
     for (int i = 0 ; i < idx; ++i)
     {
       //int k = i+(node->file_buffer->disk_count);
	 
         ret[i].timeseries = calloc(ts_length, sizeof(ts_type));
	 
	 for (int g = 0; g < ts_length;++g)
	 {
	   ret[i].timeseries[g] = node->file_buffer->buffered_list[i].timeseries[g];
	 }
     }
  }
  return ret;
}

struct vaplus_record * get_all_records_in_node(struct vaplus_index * index, struct vaplus_node * node) 
{

  unsigned int ts_length = index->settings->timeseries_size;
  unsigned int max_leaf_size = index->settings->max_leaf_size;
  unsigned int transforms_size = index->settings->fft_size;
  
  struct vaplus_record * ret = NULL;
  
  ret = calloc(max_leaf_size, sizeof(struct vaplus_record) );
     
  if (node->file_buffer->disk_count > 0)
  {      
      if(node->filename == NULL)
      {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: This node has data on disk but"
                        "could not get its filename.\n");
        //return FAILURE;
      }

     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1; 
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);

     const char * ts_file_name = malloc(sizeof(char) * (full_size + 5) );;
     const char * fft_file_name = malloc(sizeof(char) * (full_size + 5) );; 
     const char * vaplus_file_name = malloc(sizeof(char) * (full_size + 5) );;

     ts_file_name = strcpy(ts_file_name,full_filename);     
     ts_file_name = strcat(ts_file_name, ".ts\0");

     fft_file_name = strcpy(fft_file_name,full_filename);     
     fft_file_name = strcat(fft_file_name, ".fft\0");

     vaplus_file_name = strcpy(vaplus_file_name,full_filename);     
     vaplus_file_name = strcat(vaplus_file_name, ".vaplus\0");
                
     COUNT_PARTIAL_RAND_INPUT
     COUNT_PARTIAL_RAND_INPUT
     COUNT_PARTIAL_RAND_INPUT       
     COUNT_PARTIAL_INPUT_TIME_START
     FILE *ts_file = fopen(ts_file_name, "r");
     FILE *fft_file = fopen(fft_file_name, "r");
     FILE *vaplus_file = fopen(vaplus_file_name, "r");       
     COUNT_PARTIAL_INPUT_TIME_END
       
     if(ts_file == NULL)
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not open"
		"the filename %s. Reason = %s\n", ts_file_name, strerror(errno));
        //return FAILURE;
     }
     if(fft_file == NULL)
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not open"
		"the filename %s. Reason = %s\n", fft_file_name, strerror(errno));
        //return FAILURE;
     }
     if(vaplus_file == NULL)
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not open"
		"the filename %s. Reason = %s\n", vaplus_file_name, strerror(errno));
        //return FAILURE;
     }     

     //in order to keep the same order that the data was inserted, we move the
     //time series that are in memory to allw the disk based time series to be
     //first in the buffer.
     
     
     for (int i=0; i<node->file_buffer->disk_count;++i ) 
     {       
       //node->file_buffer->buffered_list[idx] = malloc(sizeof(ts_type) * ts_length);
       ret[i].timeseries = calloc(ts_length, sizeof(ts_type));
       ret[i].dft_transform = calloc(transforms_size, sizeof(ts_type));
       ret[i].vaplus_word = calloc(transforms_size, sizeof(unsigned char));
       
       //ret[i] = (ts_type *) index->buffer_manager->current_record;
       //index->buffer_manager->current_record += sizeof(ts_type) * ts_length;
       //index->buffer_manager->current_record_index++;

       COUNT_PARTIAL_SEQ_INPUT
       COUNT_PARTIAL_SEQ_INPUT
       COUNT_PARTIAL_SEQ_INPUT	 
       COUNT_PARTIAL_INPUT_TIME_START      
       fread(ret[i].timeseries,
	     sizeof(ts_type),ts_length, ts_file);
       fread(ret[i].dft_transform,
	     sizeof(ts_type),transforms_size, fft_file);
       fread(ret[i].vaplus_word,
	     sizeof(unsigned char),transforms_size, vaplus_file);
       COUNT_PARTIAL_INPUT_TIME_END       
     }

     //adding to ret the data in memory so that the
     //split data is re-inserted in the same order it was
     //inserted the first time
     
     int idx = node->file_buffer->buffered_list_size;
 
     for (int i = 0 ; i < idx; ++i)
     {
         int k = i+(node->file_buffer->disk_count);
	 
         ret[k].timeseries = calloc(ts_length, sizeof(ts_type));
         ret[k].dft_transform = calloc(transforms_size, sizeof(ts_type));
         ret[k].vaplus_word = calloc(transforms_size, sizeof(unsigned char));

	 for (int g = 0; g < ts_length;++g)
	 {
	   ret[k].timeseries[g] = node->file_buffer->buffered_list[i].timeseries[g];
	 }
	 for (int g = 0; g < transforms_size;++g)
	 {
	   ret[k].dft_transform[g] = node->file_buffer->buffered_list[i].dft_transform[g];
	   ret[k].vaplus_word[g] = node->file_buffer->buffered_list[i].vaplus_word[g];
	 }  
         /*
	 memcpy(ret[k].timeseries, node->file_buffer->buffered_list[i].timeseries, ts_length*sizeof(ts_type));
         memcpy(ret[k].dft_transform, node->file_buffer->buffered_list[i].dft_transform, transforms_size*sizeof(ts_type));
         memcpy(ret[k].vaplus_word, node->file_buffer->buffered_list[i].vaplus_word, transforms_size*sizeof(char));
	 */
	 
     }

     //this should be equal to old size + disk_count
     //node->file_buffer->buffered_list_size = idx+node->file_buffer->disk_count;

      
     COUNT_PARTIAL_INPUT_TIME_START
     if(fclose(ts_file))
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not close"
		"the filename %s. Reason= %s.\n", ts_file_name, strerror(errno));
        //return FAILURE;
     } 
     if(fclose(fft_file))
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not close"
		"the filename %s. Reason= %s.\n", fft_file_name, strerror(errno));
        //return FAILURE;
     } 
     if(fclose(vaplus_file))
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not close"
		"the filename %s. Reason= %s.\n", vaplus_file_name, strerror(errno));
        //return FAILURE;
     } 
     COUNT_PARTIAL_INPUT_TIME_END    
     free(ts_file_name);
     free(fft_file_name);
     free(vaplus_file_name);     
  }
  else
  {
     int idx = node->file_buffer->buffered_list_size;
 
     for (int i = 0 ; i < idx; ++i)
     {
       //int k = i+(node->file_buffer->disk_count);
	 
         ret[i].timeseries = calloc(ts_length, sizeof(ts_type));
         ret[i].dft_transform = calloc(transforms_size, sizeof(ts_type));
         ret[i].vaplus_word = calloc(transforms_size, sizeof(unsigned char));
	 
	 for (int g = 0; g < ts_length;++g)
	 {
	   ret[i].timeseries[g] = node->file_buffer->buffered_list[i].timeseries[g];
	 }
	 for (int g = 0; g < transforms_size;++g)
	 {
	   ret[i].dft_transform[g] = node->file_buffer->buffered_list[i].dft_transform[g];
	   ret[i].vaplus_word[g] = node->file_buffer->buffered_list[i].vaplus_word[g];
	 }  

	 /*
         memcpy(ret[i].timeseries, node->file_buffer->buffered_list[i].timeseries, ts_length*sizeof(ts_type));
         memcpy(ret[i].dft_transform, node->file_buffer->buffered_list[i].dft_transform, transforms_size*sizeof(ts_type));
         memcpy(ret[i].vaplus_word, node->file_buffer->buffered_list[i].vaplus_word, transforms_size*sizeof(char));
	 */
     }

  }
  return ret;
}
/*
ts_type ** get_all_time_series_in_node(struct vaplus_index * index, struct vaplus_node * node) 
{

  int ts_length = index->settings->timeseries_size;
  int max_leaf_size = index->settings->max_leaf_size;
  ts_type ** ret = NULL;
   ret = calloc(max_leaf_size, sizeof(ts_type *));
     
  if (node->file_buffer->disk_count > 0)
  {
      
      if(node->filename == NULL)
      {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: This node has data on disk but"
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
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not open"
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
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not close"
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

*/

/*
 if full_flush, flush the raw, fft and vaplus data. This is necessary 
 when the index is still being built.
 Once the index is final, a full flush is not needed as only the raw
 ts files are kept as the index already encodes the necessary fft and vaplus data.
 needed during querying.
*/

enum response flush_buffer_to_disk(struct vaplus_index *index, struct vaplus_node *node, boolean full_flush)
{

  if (node->file_buffer->buffered_list_size > 0 )
  {

    if(node->filename == NULL)
    {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Cannot flush the node to disk. "
                        "It does not have a filename. \n");
        return FAILURE;
    }
  
     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1; 
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);

     const char *ts_file_name = malloc(sizeof(char) * (full_size + 5) );;
     ts_file_name = strcpy(ts_file_name,full_filename);     
     ts_file_name = strcat(ts_file_name, ".ts\0");

     COUNT_PARTIAL_RAND_OUTPUT
     COUNT_PARTIAL_OUTPUT_TIME_START
     FILE *ts_file = fopen(ts_file_name, "a");
     COUNT_PARTIAL_OUTPUT_TIME_END
     
     if(ts_file == NULL)
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.."
		"Could not open the filename %s. Reason= %s\n", ts_file_name, strerror(errno));
        //return FAILURE;
        return SUCCESS;
     }

     const char *fft_file_name;
     const char *vaplus_file_name;
     FILE *fft_file;
     FILE *vaplus_file;
       
     if (full_flush)
     {
       fft_file_name = malloc(sizeof(char) * (full_size + 5) );; 
       vaplus_file_name = malloc(sizeof(char) * (full_size + 5) );;     
       fft_file_name = strcpy(fft_file_name,full_filename);     
       fft_file_name = strcat(fft_file_name, ".fft\0");
       vaplus_file_name = strcpy(vaplus_file_name,full_filename);     
       vaplus_file_name = strcat(vaplus_file_name, ".vaplus\0");
       
       
       
       COUNT_PARTIAL_RAND_OUTPUT
       COUNT_PARTIAL_RAND_OUTPUT	 
       COUNT_PARTIAL_OUTPUT_TIME_START
	 
       fft_file = fopen(fft_file_name, "a");
       vaplus_file = fopen(vaplus_file_name, "a");
       COUNT_PARTIAL_OUTPUT_TIME_END
	 
       //fprintf(stderr, "In vaplus_file_buffer.c: Flushing node %s to disk.\n.", full_filename);
	 
       if(fft_file == NULL)
	   {   
	     fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.."
		     "Could not open the filename %s. Reason= %s\n", fft_file_name, strerror(errno));
	     //return FAILURE;
	     return SUCCESS;
	   }
       if(vaplus_file == NULL)
	 {   
	   fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.."
		   "Could not open the filename %s. Reason= %s\n", vaplus_file_name, strerror(errno));
	   //return FAILURE;
	   return SUCCESS;
	 }    
    }
     
    int num_ts =  node->file_buffer->buffered_list_size;

    COUNT_PARTIAL_OUTPUT_TIME_START	
    for (int idx = 0; idx < num_ts;++idx )
    {
      COUNT_PARTIAL_SEQ_OUTPUT
      if(!fwrite(node->file_buffer->buffered_list[idx].timeseries, sizeof(ts_type), index->settings->timeseries_size, ts_file))
      {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Could not "
                         "write the timeseries to file %s.\n", ts_file_name);
        return FAILURE;
      }
      if (full_flush)
	{
	  COUNT_PARTIAL_SEQ_OUTPUT
	    COUNT_PARTIAL_SEQ_OUTPUT
	    if(!fwrite(node->file_buffer->buffered_list[idx].dft_transform, sizeof(ts_type), index->settings->fft_size, fft_file))
	      {   
		fprintf(stderr, "Error in vaplus_file_buffer.c: Could not "
			"write the timeseries to file %s.\n", fft_file_name);
		return FAILURE;
	      }
	  if(!fwrite(node->file_buffer->buffered_list[idx].vaplus_word, sizeof(unsigned char), index->settings->fft_size, vaplus_file))
	    {   
	      fprintf(stderr, "Error in vaplus_file_buffer.c: Could not "
		      "write the timeseries to file %s.\n", vaplus_file_name);
	      return FAILURE;
	    }
	}
    }

    if(fclose(ts_file))
     {   
        fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.. "
		"Could not close the filename %s. Reason = %s.\n", ts_file_name, strerror(errno));
        return FAILURE;
     }
    if (full_flush)
    {
	if(fclose(fft_file))
	  {   
	    fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.. "
		    "Could not close the filename %s. Reason = %s.\n", fft_file_name, strerror(errno));
	    return FAILURE;
	  }
	if(fclose(vaplus_file))
	  {   
	    fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.. "
		    "Could not close the filename %s. Reason = %s.\n", vaplus_file_name, strerror(errno));
	    return FAILURE;
	  }
    }
    COUNT_PARTIAL_OUTPUT_TIME_END
      
    node->file_buffer->disk_count += num_ts;

    if (!clear_file_buffer(index, node))
    {
        fprintf(stderr, "Error in vaplus_file_buffer.c: Flushing node to disk.. "
                         "Could not clear the buffer for %s.\n", full_filename);
        return FAILURE;      
    }
    
    node->file_buffer->in_disk = true;
    
    free(full_filename);
    
    free(ts_file_name);
    if (full_flush)
    {
	free(fft_file_name);
	free(vaplus_file_name);
    }
    
  }

    return SUCCESS; 
    
}

enum response clear_file_buffer(struct vaplus_index *index, struct vaplus_node * node)
{

  if ((node->file_buffer) == NULL )
  {
      fprintf(stderr, "Error in vaplus_file_buffer.c: Cannot clear a NULL buffer.\n");
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


enum response delete_file_buffer(struct vaplus_index * index,struct vaplus_node * node)
{
  
  if (node->file_buffer->in_disk) //delete file if in disk
  {

     int full_size = strlen(index->settings->root_directory) + strlen(node->filename)+1; 
     
     const char *full_filename = malloc(sizeof(char) * full_size);
     full_filename = strcpy(full_filename, index->settings->root_directory);
     full_filename = strcat(full_filename, node->filename);

     const char *ts_file_name = malloc(sizeof(char) * (full_size + 5) );;
     const char *fft_file_name = malloc(sizeof(char) * (full_size + 5) );; 
     const char *vaplus_file_name = malloc(sizeof(char) * (full_size + 5) );;
     
     ts_file_name = strcpy(ts_file_name,full_filename);     
     ts_file_name = strcat(ts_file_name, ".ts\0");

     fft_file_name = strcpy(fft_file_name,full_filename);     
     fft_file_name = strcat(fft_file_name, ".fft\0");

     vaplus_file_name = strcpy(vaplus_file_name,full_filename);     
     vaplus_file_name = strcat(vaplus_file_name, ".vaplus\0");

     
     COUNT_PARTIAL_RAND_OUTPUT
     COUNT_PARTIAL_RAND_OUTPUT
     COUNT_PARTIAL_RAND_OUTPUT       
     COUNT_PARTIAL_OUTPUT_TIME_START

    
    //delete file from disk, return and error if not removed properly
    // fprintf(stderr, "In vaplus_file_buffer.c: Deleting node %s from disk.\n.", full_filename);
     
    if(!remove(ts_file_name) && !remove(fft_file_name) && !remove(vaplus_file_name)) //file deleted successfully
    {
      node->file_buffer->disk_count = 0;
      node->file_buffer->in_disk = false;
    }
    else 
    {
      fprintf(stderr, "Error in vaplus_file_buffer.c: Error deleting filename %s.\n", ts_file_name);
      return FAILURE;          
    }
    free(ts_file_name);  
    free(fft_file_name);
    free(vaplus_file_name);      
  }
  
    struct vaplus_file_map * res = node->file_buffer->position_in_map;
    
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
         fprintf(stderr, "Error in vaplus_file_buffer.c: Deleting node.. "
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
