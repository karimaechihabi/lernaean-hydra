
//
//  sfa_file_buffer_manager.c
//  sfa C version
//
//  Created by Karima Echihabi on 18/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#include "../include/sfa_file_buffer_manager.h"

 
enum response init_file_buffer_manager(struct sfa_trie *trie)
{

  trie->buffer_manager = NULL;

  trie->buffer_manager = malloc(sizeof(struct sfa_file_buffer_manager));
  if(trie->buffer_manager == NULL) {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Could not allocate memory for buffer manager.\n");
          return FAILURE;	
  }
    
  trie->buffer_manager->max_buffered_size = 1000*1000*100;

  trie->buffer_manager->current_count = 0;  //the number of time series points in memory

  trie->buffer_manager->batch_remove_size;

  trie->buffer_manager->file_map = NULL; //the file map is empty initially
  trie->buffer_manager->file_map_tail = NULL; //the file map is empty initially
  
  trie->buffer_manager->file_map_size = 0;  

  set_buffered_memory_size(trie);

  return SUCCESS;
}
enum response set_buffered_memory_size(struct sfa_trie * trie)
{
  if(trie == NULL)
  {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Cannot set the buffered memory \
                           for a NULL trie.\n");
          return FAILURE;	    
  }
  else
  {
        unsigned long num_bytes = trie->settings->buffered_memory_size * 1024 * 1024;
	
	trie->buffer_manager->max_buffered_size = (long) (trie->settings->buffered_memory_size * 1024 * 1024 / sizeof(ts_type));	
        trie->buffer_manager->batch_remove_size = trie->buffer_manager->max_buffered_size /2;

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
            This array is released after the trie is materialized to disk.  
	*/
        int max_leaf_size = trie->settings->max_leaf_size;
        unsigned long leaf_size = trie->settings->max_leaf_size;

	unsigned long ts_byte_size= sizeof(ts_type) * trie->settings->timeseries_size;
        unsigned int dft_length = trie->settings->fft_size;
	unsigned int dft_byte_size = sizeof(ts_type) * dft_length;
	unsigned int sfa_byte_size = sizeof(unsigned char) * dft_length;
	unsigned int sfa_record_size = sizeof(struct sfa_record *);
	
        unsigned long num_leaf_buffers = 2 * ((long) (num_bytes / (leaf_size * sfa_record_size)));

        unsigned long size_leaf_buffer = sizeof(struct sfa_file_buffer) +
	                                       (sizeof(ts_type*) * max_leaf_size);

	/*
	unsigned long mem_array_size =  (unsigned long)((num_bytes - size_leaf_buffer * num_leaf_buffers));
	*/
	unsigned long mem_array_size = num_bytes * 0.9  / (double)(ts_byte_size + dft_byte_size + sfa_byte_size);

        trie->buffer_manager->chunk_size = mem_array_size;
	
        trie->buffer_manager->ts_mem_array = calloc(mem_array_size+1,ts_byte_size);
	
        if(trie->buffer_manager->ts_mem_array == NULL)
        {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c:"
  		         "Cannot allocate ts buffer of size %lu MB.\n",
		         trie->settings->buffered_memory_size);
          return FAILURE;	    
        }



	//Initially, we use the dft_mem_array to hold the samples
	//So they array will be of this form:
	// dft_array[x][y] where x = dft_size and y = mem_array_size
	//whereas sfa and ts arrays are the opposite:
	// x= mem_array_size and y=sfa or ts
	//this is needed for sorting the bins
	//Once the bins are set, we revert the dft array to be of the
	//same form as ts and sfa arrays.


	//in the beginning, we set it to sample_size
	//after setting the bins we will use chunk_size
	
        trie->buffer_manager->dft_mem_array = calloc(dft_length+1,sizeof(ts_type)*trie->settings->sample_size);

        if(trie->buffer_manager->dft_mem_array == NULL)
        {
          fprintf(stderr,"Error in sfa_buffer_manager.c:"
  		         "Cannot allocate dft buffer of size %lu MB.\n",
		         mem_array_size);
          return FAILURE;	    
        }

	
        trie->buffer_manager->sfa_mem_array = calloc(mem_array_size+1,sfa_byte_size);

        if(trie->buffer_manager->sfa_mem_array == NULL)
        {
          fprintf(stderr,"Error in sfa_buffer_manager.c:"
  		         "Cannot allocate sfa buffer of size %lu MB.\n",
		         trie->settings->buffered_memory_size);
          return FAILURE;	    
        }

	trie->buffer_manager->current_record_index = 0;
	trie->buffer_manager->max_record_index = mem_array_size + 1 ;
	trie->buffer_manager->current_ts_record = trie->buffer_manager->ts_mem_array;
	trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
	trie->buffer_manager->current_sfa_record = trie->buffer_manager->sfa_mem_array;
	
	return SUCCESS;
  }
}

enum response reset_dft_memory_size(struct sfa_trie * trie)
{
  if(trie == NULL)
  {
          fprintf(stderr,"Error in sfa_buffer_manager.c: Cannot set the buffered memory \
                           for a NULL trie.\n");
          return FAILURE;	    
  }
  else
  {
	
        unsigned int dft_length = trie->settings->fft_size;
	unsigned int dft_byte_size = sizeof(ts_type) * dft_length;

	
	//Initially, we use the dft_mem_array to hold the samples
	//So they array will be of this form:
	// dft_array[x][y] where x = dft_size and y = mem_array_size
	//whereas sfa and ts arrays are the opposite:
	// x= mem_array_size and y=sfa or ts
	//this is needed for sorting the bins
	//Once the bins are set, we revert the dft array to be of the
	//same form as ts and sfa arrays.
	
        free (trie->buffer_manager->dft_mem_array);
             	
        trie->buffer_manager->dft_mem_array = calloc(trie->buffer_manager->chunk_size+1,dft_byte_size);

        if(trie->buffer_manager->dft_mem_array == NULL)
        {
          fprintf(stderr,"Error in sfa_buffer_manager.c:"
  		         "Cannot allocate dft buffer of size %lu MB.\n",
		         trie->settings->buffered_memory_size);
          return FAILURE;	    
        }
	

	trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
	
	return SUCCESS;
  }
}


/*
  Upon return, the node->file_buffer contains the file buffer of this node
  And trie->buffer_manager file map includes this new buffer
 */

enum response get_file_buffer(struct sfa_trie *trie, struct sfa_node *node)
{
  if (node->file_buffer == NULL) //node does not have a file buffer yet 
  {
    if (!sfa_file_buffer_init(node))
    {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Could not initialize the file buffer \
                           for the node.\n");
          return FAILURE;	    
    }
    
    if (!add_file_buffer_to_map(trie, node))    
    {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Could not add the file buffer \
                           to the map.\n");
          return FAILURE;	    
    }  
  }


  int buffer_limit = trie->buffer_manager->max_record_index - (2 * trie->settings->max_leaf_size); 

  if (trie->buffer_manager->current_record_index > buffer_limit)  
  {
      char * curr_time;
      curr_time= NULL;
      curr_time = malloc (sizeof(char) *26);
      get_current_time(curr_time);
      printf ("%s, batch remove ! %lu  \n", curr_time, trie->buffer_manager->current_record_index);
      unsigned long to_size = 0;
      free(curr_time);
       
      struct sfa_file_map * currP = NULL;
      currP = trie->buffer_manager->file_map;

      while (currP != NULL) {
        if(!flush_buffer_to_disk(trie, currP->file_buffer->node, true)) //flush the actual buffer of the node 
	{
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Could not flush the buffer \
                          for node %s to disk.\n",currP->file_buffer->node->filename);
          return FAILURE;	
        }
        currP = currP->next;
      }
      memset(trie->buffer_manager->ts_mem_array,0,trie->buffer_manager->max_record_index + 1 );
      memset(trie->buffer_manager->dft_mem_array,0,trie->buffer_manager->max_record_index + 1);
      memset(trie->buffer_manager->sfa_mem_array,0,trie->buffer_manager->max_record_index + 1 );

      trie->buffer_manager->current_record_index = 0;
      
      trie->buffer_manager->current_ts_record = trie->buffer_manager->ts_mem_array;
      trie->buffer_manager->current_dft_record = trie->buffer_manager->dft_mem_array;
      trie->buffer_manager->current_sfa_record = trie->buffer_manager->sfa_mem_array;      
  }
  return SUCCESS;
}


enum response add_file_buffer_to_map(struct sfa_trie * trie, struct sfa_node *node)
{
    int idx = trie->buffer_manager->file_map_size;
        
    if (idx == 0)
    {
      trie->buffer_manager->file_map = malloc(sizeof(struct sfa_file_map));
      if(trie->buffer_manager->file_map == NULL) {
        fprintf(stderr,"Error in sfa_file_buffer_manager.c: Could not allocate memory for the file map.\n");
	return FAILURE;	
      }
      trie->buffer_manager->file_map[idx].file_buffer = node->file_buffer;
      node->file_buffer->position_in_map = &trie->buffer_manager->file_map[idx];
      
      //trie->buffer_manager->file_map[idx].filename = node->filename;      
      trie->buffer_manager->file_map[idx].prev = NULL;
      trie->buffer_manager->file_map[idx].next = NULL;

      trie->buffer_manager->file_map_tail = trie->buffer_manager->file_map;
      
    }
    else
    {
      struct sfa_file_map * currP = trie->buffer_manager->file_map;
      struct sfa_file_map * lastP = trie->buffer_manager->file_map_tail;


      lastP->next = malloc(sizeof(struct sfa_file_map));

      if(lastP->next == NULL) {
        fprintf(stderr,"Error in sfa_file_buffer_manager: Could not allocate memory for new entry in the file map.\n");
	return FAILURE;	
      }
      
      lastP->next->file_buffer = node->file_buffer;
      node->file_buffer->position_in_map = lastP->next;
      
      lastP->next->prev = lastP;
      lastP->next->next = NULL;
      trie->buffer_manager->file_map_tail = lastP->next;
      
    }

    ++trie->buffer_manager->file_map_size;
   
  return SUCCESS;
}


enum response save_all_buffers_to_disk(struct sfa_trie *trie)
{
    
  struct sfa_file_map * currP = trie->buffer_manager->file_map;
      
  while(currP != NULL)
  {
    if(!flush_buffer_to_disk(trie, currP->file_buffer->node, true))
    {
          fprintf(stderr,"Error in sfa_file_buffer_manager.c: Saving all to disk..Could not flush the buffer \
                          for node %s to disk.\n", currP->file_buffer->node->filename);
          return FAILURE;	
    }
	
    currP = currP->next;
  }

  return SUCCESS;
     
}
