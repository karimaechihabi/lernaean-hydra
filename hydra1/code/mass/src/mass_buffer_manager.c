//  mass_buffer_manager.c
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.

#include "../include/mass_buffer_manager.h"

 
enum response init_buffer_manager(struct mass_instance *mass_inst)
{

  mass_inst->buffer_manager = NULL;

  mass_inst->buffer_manager = malloc(sizeof(struct mass_buffer_manager));
  if(mass_inst->buffer_manager == NULL) {
          fprintf(stderr,"Error in dstree_file_buffer_manager.c: Could not allocate memory for buffer manager.\n");
          return FAILURE;	
  }
    

  mass_inst->buffer_manager->chunk_size = 0;
  mass_inst->buffer_manager->last_chunk_size = 0;
  mass_inst->buffer_manager->num_chunks = 0;
  mass_inst->buffer_manager->current_chunk_id = 0;
  
  set_buffered_memory_size(mass_inst);

  return SUCCESS;
}


/*
  initialze the buffer memory pool
  buffered_memory_size is the size of the buffer in MB
  max_buffered_size is the max number of floats that can fit in memory

 */

enum response set_buffered_memory_size(struct mass_instance * mass_inst)
{
  if(mass_inst == NULL)
  {
          fprintf(stderr,"Error in dstree_file_buffer_manager.c: Cannot set the buffered memory \
                           for a NULL mass_inst.\n");
          return FAILURE;	    
  }
  else
  {
        unsigned long long num_bytes = mass_inst->settings->buffered_memory_size * 1024 * 1024;
	

	unsigned long long ts_byte_size= mass_inst->settings->timeseries_byte_size;
	
        mass_inst->buffer_manager->chunk_size = num_bytes / ts_byte_size ;

	mass_inst->buffer_manager->num_chunks = mass_inst->settings->timeseries_count /
	                                            mass_inst->buffer_manager->chunk_size;
	unsigned long long remainder = mass_inst->settings->timeseries_count % mass_inst->buffer_manager->chunk_size;
        if (remainder == 0)
	{
            mass_inst->buffer_manager->last_chunk_size =  mass_inst->buffer_manager->chunk_size;	    
	}
        else
	{
  	    ++mass_inst->buffer_manager->num_chunks;	    
            mass_inst->buffer_manager->last_chunk_size = remainder ;	    	  
	}
	
        mass_inst->buffer_manager->mem_array = calloc(mass_inst->buffer_manager->chunk_size,ts_byte_size);
        if(mass_inst->buffer_manager->mem_array == NULL)
        {
          fprintf(stderr,"Error in mass_buffer_manager.c:"
  		         "Cannot allocate buffer of size %lu MB.\n",
		         mass_inst->settings->buffered_memory_size);
          return FAILURE;	    
        }
	mass_inst->buffer_manager->current_record_index = 0;
	mass_inst->buffer_manager->max_record_index = mass_inst->buffer_manager->chunk_size;
	mass_inst->buffer_manager->current_record = mass_inst->buffer_manager->mem_array;
	
	return SUCCESS;
  }
}

