//
//  vaplus_file_buffer_manager.c
//  vaplus C version
//
//  Created by Karima Echihabi on 18/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#include "../include/vaplus_file_buffer_manager.h"

 
enum response init_file_buffer_manager(struct vaplus_index *index)
{

  index->buffer_manager = NULL;

  index->buffer_manager = malloc(sizeof(struct vaplus_file_buffer_manager));
  if(index->buffer_manager == NULL) {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Could not allocate memory for buffer manager.\n");
          return FAILURE;	
  }
    
  index->buffer_manager->max_buffered_size = 1000*1000*100;

  index->buffer_manager->current_count = 0;  //the number of time series points in memory

  index->buffer_manager->batch_remove_size;

  index->buffer_manager->file_map = NULL; //the file map is empty initially
  index->buffer_manager->file_map_tail = NULL; //the file map is empty initially
  
  index->buffer_manager->file_map_size = 0;  

  set_buffered_memory_size(index);

  return SUCCESS;
}
enum response set_buffered_memory_size(struct vaplus_index * index)
{
  if(index == NULL)
  {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Cannot set the buffered memory \
                           for a NULL index.\n");
          return FAILURE;	    
  }
  else
  {
        unsigned long num_bytes = index->settings->buffered_memory_size * 1024 * 1024;
	
	index->buffer_manager->max_buffered_size = (long) (index->settings->buffered_memory_size * 1024 * 1024 / sizeof(ts_type));	
        index->buffer_manager->batch_remove_size = index->buffer_manager->max_buffered_size /2;

        /*
            The mem_array is a giant memory array of size 
            buffered_memory_size in MB per user input. 

            It holds X number of ts buffers:
            X = (num_bytes)/(sizeof(ts_type) * ts_length * max_leaf_size)

            At initialization time, an entire array of memory is locked
            to contain the buffered time series. This has proven to be
            more efficient causing less fragmentation compared to
            a large number of allocations and deallocations of small 
            memory slots.
            This array is released after the index is materialized to disk.  
	*/
        int max_leaf_size = index->settings->max_leaf_size;
        unsigned long leaf_size = index->settings->max_leaf_size;
        unsigned int dataset_size = index->settings->dataset_size;

	unsigned long ts_byte_size= sizeof(ts_type) * index->settings->timeseries_size;
        unsigned int dft_length = index->settings->fft_size;
	unsigned int dft_byte_size = sizeof(ts_type) * dft_length;
	unsigned int vaplus_byte_size = sizeof(unsigned int) * dft_length;
	unsigned int vaplus_record_size = sizeof(struct vaplus_record *);
	

	//unsigned long mem_array_size = num_bytes * 0.9  / (double)(ts_byte_size + dft_byte_size + vaplus_byte_size);
	unsigned long mem_array_size = num_bytes   / (double)(dft_byte_size);

        index->buffer_manager->chunk_size = mem_array_size;
	
	/*
	index->buffer_manager->ts_mem_array = calloc(mem_array_size+1,ts_byte_size);
	
        if(index->buffer_manager->ts_mem_array == NULL)
        {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c:"
  		         "Cannot allocate ts buffer of size %lu MB.\n",
		         index->settings->buffered_memory_size);
          return FAILURE;	    
        }
        */


	//Initially, we use the dft_mem_array to hold the samples
	//So they array will be of this form:
	// dft_array[x][y] where x = dft_size and y = mem_array_size
	//whereas vaplus and ts arrays are the opposite:
	// x= mem_array_size and y=vaplus or ts
	//this is needed for sorting the bins
	//Once the bins are set, we revert the dft array to be of the
	//same form as ts and vaplus arrays.

	
	/*
        index->buffer_manager->dft_mem_array = calloc(mem_array_size,dft_byte_size);

        if(index->buffer_manager->dft_mem_array == NULL)
        {
          fprintf(stderr,"Error in vaplus_buffer_manager.c:"
  		         "Cannot allocate buffer of size %lu MB.\n",
		         index->settings->buffered_memory_size);
          return FAILURE;	    
        }
	*/

	//in the beginning, we set it to sample_size
	//after setting the bins we will use chunk_size
	
        if (index->settings->is_new)
        {
          index->buffer_manager->dft_mem_array = calloc(dft_length+1,sizeof(ts_type)*index->settings->dataset_size);

          if(index->buffer_manager->dft_mem_array == NULL)
          {
            fprintf(stderr,"Error in vaplus_buffer_manager.c:"
  		         "Cannot allocate dft buffer of size %lu MB.\n",
		         mem_array_size);
           return FAILURE;	    
          }

        	
          index->buffer_manager->approx_mem_array = calloc(dft_length+1,sizeof(unsigned int)*index->settings->dataset_size);
  
          if(index->buffer_manager->approx_mem_array == NULL)
          {
            fprintf(stderr,"Error in vaplus_buffer_manager.c:"
  		         "Cannot allocate approx buffer of size %lu MB.\n",
		         mem_array_size);
            return FAILURE;	    
          }
	}
        else
        {
          index->buffer_manager->approx_mem_array = calloc(dataset_size+1,sizeof(unsigned int)*dft_length);
  
          if(index->buffer_manager->approx_mem_array == NULL)
          {
            fprintf(stderr,"Error in vaplus_buffer_manager.c:"
  		         "Cannot allocate approx buffer of size %lu MB.\n",
		         mem_array_size);
            return FAILURE;	    
          }

        }
	
	index->buffer_manager->current_record_index = 0;
	index->buffer_manager->max_record_index = mem_array_size + 1 ;
	//index->buffer_manager->current_ts_record = index->buffer_manager->ts_mem_array;
	index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
	index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;
	
	return SUCCESS;
  }
}

enum response reset_dft_memory_size(struct vaplus_index * index)
{
  if(index == NULL)
  {
          fprintf(stderr,"Error in vaplus_buffer_manager.c: Cannot set the buffered memory \
                           for a NULL index.\n");
          return FAILURE;	    
  }
  else
  {
	
        unsigned int dft_length = index->settings->fft_size;
	unsigned int dft_byte_size = sizeof(ts_type) * dft_length;

	
	//Initially, we use the dft_mem_array to hold the samples
	//So they array will be of this form:
	// dft_array[x][y] where x = dft_size and y = mem_array_size
	//whereas vaplus and ts arrays are the opposite:
	// x= mem_array_size and y=vaplus or ts
	//this is needed for sorting the bins
	//Once the bins are set, we revert the dft array to be of the
	//same form as ts and vaplus arrays.
        /*
        for (unsigned long i= 0; i < index->dft_transform->fft_size; ++i)
        {
	  free(index->buffer_manager->dft_mem_array[i]);
	}
	*/
	
        free (index->buffer_manager->dft_mem_array);
             	
        index->buffer_manager->dft_mem_array = calloc(index->buffer_manager->chunk_size+1,dft_byte_size);

        if(index->buffer_manager->dft_mem_array == NULL)
        {
          fprintf(stderr,"Error in vaplus_buffer_manager.c:"
  		         "Cannot allocate dft buffer of size %lu MB.\n",
		         index->settings->buffered_memory_size);
          return FAILURE;	    
        }
	

	index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
	
	return SUCCESS;
  }
}


/*
  Upon return, the node->file_buffer contains the file buffer of this node
  And index->buffer_manager file map includes this new buffer
 */

enum response get_file_buffer(struct vaplus_index *index, struct vaplus_node *node)
{
  if (node->file_buffer == NULL) //node does not have a file buffer yet 
  {
    if (!vaplus_file_buffer_init(node))
    {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Could not initialize the file buffer \
                           for the node.\n");
          return FAILURE;	    
    }
    
    if (!add_file_buffer_to_map(index, node))    
    {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Could not add the file buffer \
                           to the map.\n");
          return FAILURE;	  
    }  
  }

//  to guard against the case that get_all_time_series is called while the buffer is almost full
//  we always flush the buffer before it is completely full since get_all_time_series loads
//  the full contents of a leaf size at once.

  int buffer_limit = index->buffer_manager->max_record_index - (2 * index->settings->max_leaf_size); 

  if (index->buffer_manager->current_record_index > buffer_limit)  
  {
      char * curr_time;
      curr_time= NULL;
      curr_time = malloc (sizeof(char) *26);
      get_current_time(curr_time);
      printf ("%s, batch remove ! %lu  \n", curr_time, index->buffer_manager->current_record_index);
      unsigned long to_size = 0;
      free(curr_time);
       
      struct vaplus_file_map * currP = NULL;
      currP = index->buffer_manager->file_map;

      while (currP != NULL) {
         //flush buffer with position idx in the file map of this index
	//fprintf(stderr,"Flushing the buffer"
	//                " for node %s to disk.\n",currP->file_buffer->node->filename);	
        if(!flush_buffer_to_disk(index, currP->file_buffer->node, true)) //flush the actual buffer of the node 
	{
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Could not flush the buffer \
                          for node %s to disk.\n",currP->file_buffer->node->filename);
          return FAILURE;	
        }
        currP = currP->next;
      }
      memset(index->buffer_manager->ts_mem_array,0,index->buffer_manager->max_record_index + 1 );
      memset(index->buffer_manager->dft_mem_array,0,index->buffer_manager->max_record_index + 1);
      memset(index->buffer_manager->approx_mem_array,0,index->buffer_manager->max_record_index + 1 );

      index->buffer_manager->current_record_index = 0;
      
      index->buffer_manager->current_ts_record = index->buffer_manager->ts_mem_array;
      index->buffer_manager->current_dft_record = index->buffer_manager->dft_mem_array;
      index->buffer_manager->current_approx_record = index->buffer_manager->approx_mem_array;      
  }
  return SUCCESS;
}


enum response add_file_buffer_to_map(struct vaplus_index * index, struct vaplus_node *node)
{
  //    fprintf(stderr,"Adding file_buffer of node %s to map.\n", node->file_buffer->node->filename);
    int idx = index->buffer_manager->file_map_size;
        
    if (idx == 0)
    {
      index->buffer_manager->file_map = malloc(sizeof(struct vaplus_file_map));
      if(index->buffer_manager->file_map == NULL) {
        fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Could not allocate memory for the file map.\n");
	return FAILURE;	
      }
      index->buffer_manager->file_map[idx].file_buffer = node->file_buffer;
      node->file_buffer->position_in_map = &index->buffer_manager->file_map[idx];
      
      //index->buffer_manager->file_map[idx].filename = node->filename;      
      index->buffer_manager->file_map[idx].prev = NULL;
      index->buffer_manager->file_map[idx].next = NULL;

      index->buffer_manager->file_map_tail = index->buffer_manager->file_map;
      
    }
    else
    {
      struct vaplus_file_map * currP = index->buffer_manager->file_map;
      struct vaplus_file_map * lastP = index->buffer_manager->file_map_tail;

      /*
      
      while(currP->next != NULL)
      {
	currP = currP->next;
      }
      */

      lastP->next = malloc(sizeof(struct vaplus_file_map));

      if(lastP->next == NULL) {
        fprintf(stderr,"Error in vaplus_file_buffer_manager: Could not allocate memory for new entry in the file map.\n");
	return FAILURE;	
      }
      
      lastP->next->file_buffer = node->file_buffer;
      node->file_buffer->position_in_map = lastP->next;
      
      lastP->next->prev = lastP;
      lastP->next->next = NULL;
      index->buffer_manager->file_map_tail = lastP->next;
      
    }

    ++index->buffer_manager->file_map_size;
   
  return SUCCESS;
}


enum response save_all_buffers_to_disk(struct vaplus_index *index)
{
    
  struct vaplus_file_map * currP = index->buffer_manager->file_map;
      
  while(currP != NULL)
  {
    if(!flush_buffer_to_disk(index, currP->file_buffer->node, true))
    {
          fprintf(stderr,"Error in vaplus_file_buffer_manager.c: Saving all to disk..Could not flush the buffer \
                          for node %s to disk.\n", currP->file_buffer->node->filename);
          return FAILURE;	
    }
	
    currP = currP->next;
  }

  return SUCCESS;
     
}
