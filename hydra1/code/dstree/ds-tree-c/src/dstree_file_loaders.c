//
//  dstree_file_loaders.c
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
#include "../include/dstree_file_loaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include "../include/dstree_query_engine.h"

//CORRECT STATS FOR ASCII
enum response dstree_query_ascii_file(const char *ifilename, int q_num, 
                  const char delimiter, struct dstree_index *index,
		  float minimum_distance, ts_type epsilon, ts_type r_delta)
{
    FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    ifile = fopen (ifilename,"r");
    COUNT_PARTIAL_INPUT_TIME_END    
    if (ifile == NULL) {
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not open file %s!\n", ifilename);
        return FAILURE;
    }
    
    char *ts_str = NULL; 
    size_t linecap = 0;
    ssize_t linelen;
    unsigned int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    if (ts == NULL) {
        fprintf(stderr, "Error in dstree_file_loaders.c: Querying..\
                         Could not allocate memory for time series!\n");
        return FAILURE;
    }

    
    COUNT_PARTIAL_INPUT_TIME_START
    while ((linelen = getline(&ts_str, &linecap, ifile)) > 0 && q_loaded < q_num)
    {
        COUNT_PARTIAL_INPUT_TIME_END
	COUNT_PARTIAL_SEQ_INPUT
        if(ts_str == NULL)
        {
          fprintf(stderr,"Error in dstree_file_loaders.c: Querying..\
                         Could not get the time series from file %s.\n", ifilename);
          return FAILURE;	
        }
        if (!ts_parse_str(ts_str, ts, index->settings->timeseries_size, &delimiter))
        { 
           fprintf(stderr, "Error in dstree_file_loaders.c:  Querying..Could not parse the time series.\n");
           return FAILURE;              
        }
        
	  printf("\n\n");

        q_loaded++;
        COUNT_PARTIAL_INPUT_TIME_START    
    }



    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END
      
    free(ts);
    free(ts_str);    
    return SUCCESS;    
}

enum response dstree_query_binary_file(const char *ifilename, int q_num, struct dstree_index *index,
						   float minimum_distance, ts_type epsilon,
				       ts_type r_delta )
{
    RESET_PARTIAL_COUNTERS()
    COUNT_PARTIAL_TIME_START

    FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START    
    ifile = fopen (ifilename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END    
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END        
    unsigned int ts_length = index->settings->timeseries_size;
    file_position_type total_records = sz/ts_length * sizeof(ts_type);
    unsigned int offset = 0;

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    unsigned int q_loaded = 0;
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    ts_type * query_ts_reordered = calloc(1,sizeof(ts_type) * ts_length);
    int * query_order = calloc(1,sizeof(int) * ts_length);
        if( query_order == NULL)
           return FAILURE;

    while (q_loaded < q_num)
    {
  
        RESET_QUERY_COUNTERS ()
	  
        COUNT_PARTIAL_SEQ_INPUT      
        COUNT_PARTIAL_INPUT_TIME_START
	fread(query_ts, sizeof(ts_type), ts_length, ifile);
        COUNT_PARTIAL_INPUT_TIME_END


	reorder_query(query_ts,query_ts_reordered,query_order,ts_length);        

      
	struct query_result result = exact_search(query_ts, query_ts_reordered, query_order, offset, index, minimum_distance, epsilon, r_delta);	

        q_loaded++;
 
        get_query_stats(index,1);	
        print_query_stats(index,q_loaded, 1, ifilename);
	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START
	  
    }
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
      
     free(query_ts);
     free(query_ts_reordered);
     free(query_order);

    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", ifilename);
        return FAILURE;
    }

     return SUCCESS;
}

enum response dstree_knn_query_binary_file(const char *ifilename,
					   int q_num,
					   struct dstree_index *index,
					   float minimum_distance,
					   ts_type epsilon,
					   ts_type r_delta,
					   unsigned int k,
					   boolean track_bsf,
					   boolean track_pruning,
					   boolean all_mindists,
					   boolean max_policy,
					   unsigned int nprobes,
					   unsigned char incremental)					   
{

  struct bsf_snapshot ** bsf_snapshots = NULL;
  unsigned int max_bsf_snapshots;
  unsigned int cur_bsf_snapshot;
  if(track_bsf)
  {
    max_bsf_snapshots = 10000;
    cur_bsf_snapshot = 0;

    bsf_snapshots = calloc(k, sizeof(struct bsf_snapshot*));      
    for (unsigned int i = 0; i < k; ++i)
    {
      bsf_snapshots[i] = calloc(max_bsf_snapshots, sizeof(struct bsf_snapshot));
      for (unsigned int j = 0; j < max_bsf_snapshots; ++j)
      {
	bsf_snapshots[i][j].distance = FLT_MAX;
	bsf_snapshots[i][j].time = FLT_MAX;
      }      
    }
  }

  
    RESET_PARTIAL_COUNTERS()

    COUNT_PARTIAL_TIME_START

    FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START    
    ifile = fopen (ifilename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END    
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END        
    unsigned int ts_length = index->settings->timeseries_size;
    file_position_type total_records = sz/ts_length * sizeof(ts_type);
    unsigned int offset = 0;

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    unsigned int q_loaded = 0;
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    ts_type * query_ts_reordered = calloc(1,sizeof(ts_type) * ts_length);
    int * query_order = calloc(1,sizeof(int) * ts_length);
        if( query_order == NULL)
           return FAILURE;

    while (q_loaded < q_num)
    {
  
        RESET_QUERY_COUNTERS ()
	  
        COUNT_PARTIAL_SEQ_INPUT      
        COUNT_PARTIAL_INPUT_TIME_START
	fread(query_ts, sizeof(ts_type), ts_length, ifile);
        COUNT_PARTIAL_INPUT_TIME_END


	reorder_query(query_ts,query_ts_reordered,query_order,ts_length);        

        //answer each kNN query incrementally
        q_loaded++;
        if (track_bsf)
	{
	  cur_bsf_snapshot = 0;
	  exact_knn_search_track_bsf(query_ts, query_ts_reordered, query_order, offset,
				     index, minimum_distance, epsilon, r_delta, k, q_loaded, ifilename,
				     bsf_snapshots, &cur_bsf_snapshot);
	  
	  for (unsigned int i = 0; i < k; ++i)
	  {
	    for (unsigned int j = 0; j < max_bsf_snapshots; ++j)
	    {
	      bsf_snapshots[i][j].distance = FLT_MAX;
	      bsf_snapshots[i][j].time = FLT_MAX;	      
	    }      
	  }
	}
	else if (track_pruning)
	{
	  exact_knn_search_track_pruning(query_ts, query_ts_reordered, query_order, offset,
	  	  index, minimum_distance, epsilon, r_delta, k, q_loaded, ifilename);
	  if (all_mindists)
	  {
	    printf("\nmindist\tnum_segments\tlevel\tQoS\n");
	    dump_mindists(index,index->first_node, query_ts);
	  }	    
	}
	else if (max_policy)
	{
	  exact_knn_search_max_policy(query_ts, query_ts_reordered, query_order, offset,
			  index, minimum_distance, epsilon, r_delta, k, q_loaded, ifilename);		  
	}	
	else if (incremental)
	{
	  exact_incr_knn_search(query_ts, query_ts_reordered, query_order, offset,
			   index, minimum_distance, epsilon, r_delta, k,
			   q_loaded, ifilename, nprobes);		  
	}
	else if (nprobes)
	{
	  exact_ng_knn_search(query_ts, query_ts_reordered, query_order, offset,
			   index, minimum_distance, k,
			   q_loaded, ifilename, nprobes);		  
	}
	else 
	{
	  exact_de_knn_search(query_ts, query_ts_reordered, query_order, offset,
			   index, minimum_distance, epsilon, r_delta, k,
			   q_loaded, ifilename);		  
	}
  
	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START
	  
    }
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
      
     free(query_ts);
     free(query_ts_reordered);
     free(query_order);

    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", ifilename);
        return FAILURE;
    }

    if (track_bsf)
    {
      for (unsigned int i = 0; i < k; ++i)
	{
	  free(bsf_snapshots[i]);
	}
	free(bsf_snapshots);
    }
    return SUCCESS;
}

enum response dstree_tlb_binary_file(const char *ifilename, int q_num, struct dstree_index *index,
						   float minimum_distance)
{

    FILE * ifile;

    ifile = fopen (ifilename,"rb");

    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    fseek(ifile, 0L, SEEK_SET);
    unsigned int ts_length = index->settings->timeseries_size;
    file_position_type total_records = sz/ts_length * sizeof(ts_type);
    unsigned int offset = 0;

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    unsigned int q_loaded = 0;
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
      
    while (q_loaded < q_num)
    {
        total_tlb = 0;
	total_ts_count = 0;
	leaf_nodes_count = 0;

	fread(query_ts, sizeof(ts_type), ts_length, ifile);
      
	dstree_calc_tlb(query_ts, index, index->first_node);
	
        q_loaded++;
        print_tlb_stats(index,q_loaded, ifilename);

    }
      
     free(query_ts);

     if(fclose(ifile))
       {   
	 fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", ifilename);
	 return FAILURE;
       }
     
     return SUCCESS;
}

enum response dstree_index_ascii_file(const char *ifilename, file_position_type ts_num, 
                           const char delimiter,struct  dstree_index *index)
{
    double parse_time = 0;
    int ts_length = index->settings->timeseries_size;
    ts_type * ts = NULL;
    
    ts = NULL;
    ts = malloc (sizeof(ts_type) * ts_length); 

    if(ts == NULL)
    {
          fprintf(stderr,"Error in dstree_file_loaders.c: Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"r");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
	
    char *ts_str = NULL; //= malloc(sizeof(char) * 2000);
    size_t linecap = 0;
    ssize_t linelen;
    
    file_position_type ts_loaded = 0;    
    

    int percentage = 100;
    if (percentage > 100)
    {
       percentage = (int) (ts_num / (file_position_type) 100);
    }


    COUNT_INPUT_TIME_START
    while ((linelen = getline(&ts_str, &linecap, ifile)) > 0 && ts_loaded<ts_num)
    {
        COUNT_INPUT_TIME_END
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        COUNT_PARSE_TIME_START
        ts_parse_str(ts_str, ts, index->settings->timeseries_size, &delimiter);
        COUNT_PARSE_TIME_END


	if (!dstree_index_insert(index,ts))
	{
           fprintf(stderr, "Error in dstree_file_loaders.c:  Could not \
                           add the time series to the index.\n");
           return FAILURE;              
        }
    
        ts_loaded++;

	if(ts_loaded % percentage == 0)
            {
                float distance = 0;
            }

        COUNT_INPUT_TIME_START  
     }
    
    free(ts_str);
    free(ts);
    COUNT_INPUT_TIME_START
	fclose(ifile);
    COUNT_INPUT_TIME_END

      return SUCCESS;
}



enum response dstree_index_binary_file(const char *ifilename, file_position_type ts_num, struct dstree_index *index)
{
    double parse_time = 0;
    int ts_length = index->settings->timeseries_size;
    ts_type * ts = NULL;
    
    ts = NULL;
    ts = malloc (sizeof(ts_type) * ts_length); 

    if(ts == NULL)
    {
          fprintf(stderr,"Error in dstree_file_loaders.c: Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    FILE * ifile; 
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "Error in dstree_file_loaders.c: File %s not found!\n", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    COUNT_PARTIAL_INPUT_TIME_END    
    file_position_type total_records = sz/index->settings->timeseries_size * sizeof(ts_type);

    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END        
    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        return FAILURE;
    }
    	
    file_position_type ts_loaded = 0;    
    
    int percentage = 100;
    if (percentage > 100)
    {
       percentage = (int) (ts_num / (file_position_type) 100);
    }

    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START
        fread(ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END

	if (!dstree_index_insert(index,ts))
	  {
           fprintf(stderr, "Error in dstree_file_loaders.c:  Could not \
                           add the time series to the index.\n");
           return FAILURE;              
        }
            COUNT_PARTIAL_TIME_END

	    index->stats->idx_building_total_time  += partial_time;	
	    index->stats->idx_building_input_time  += partial_input_time;
	    index->stats->idx_building_output_time += partial_output_time;

	    index->stats->idx_building_seq_input_count  += partial_seq_input_count;
	    index->stats->idx_building_seq_output_count += partial_seq_output_count;
	    index->stats->idx_building_rand_input_count  += partial_rand_input_count;
	    index->stats->idx_building_rand_output_count += partial_rand_output_count;	    
 
	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START

        ts_loaded++;

     
        if(ts_loaded % percentage == 0)
        {
           float distance = 0;
	   //PRINT_STATS(distance)
        }

     }

    free(ts);
    COUNT_PARTIAL_INPUT_TIME_START    
    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the filename %s", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END    

            COUNT_PARTIAL_TIME_END
	    index->stats->idx_building_total_time  += partial_time;	
	    index->stats->idx_building_input_time  += partial_input_time;
	    index->stats->idx_building_output_time += partial_output_time;
	    index->stats->idx_building_seq_input_count  += partial_seq_input_count;
	    index->stats->idx_building_seq_output_count += partial_seq_output_count;
	    index->stats->idx_building_rand_input_count  += partial_rand_input_count;
	    index->stats->idx_building_rand_output_count += partial_rand_output_count;	    
 
	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START

      return SUCCESS;      
}

enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length)
{
  
        q_index *q_tmp = malloc(sizeof(q_index) * ts_length);
        int i;
	
        if( q_tmp == NULL )
	  return FAILURE;

	for( i = 0 ; i < ts_length ; i++ )
        {
          q_tmp[i].value = query_ts[i];
          q_tmp[i].index = i;
        }
	
        qsort(q_tmp, ts_length, sizeof(q_index),znorm_comp);

        for( i=0; i<ts_length; i++)
        {
          
	  query_ts_reordered[i] = q_tmp[i].value;
          query_order[i] = q_tmp[i].index;
        }
        free(q_tmp);

	return SUCCESS;
}


int znorm_comp(const void *a, const void* b)
{
    q_index* x = (q_index*)a;
    q_index* y = (q_index*)b;


    if (fabsf(y->value) > fabsf(x->value) )
       return 1;
    else if (fabsf(y->value) == fabsf(x->value))
      return 0;
    else
      return -1;

}


