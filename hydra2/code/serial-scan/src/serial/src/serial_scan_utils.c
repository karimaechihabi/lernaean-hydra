//
//  calc_utils.c
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
#include "../include/serial_scan_utils.h"
#include "math.h"
#include "float.h"

/*
void calculate_tlb(dataset,dataset_size,queries, queries_size, minimum_distance)
{
  
}
*/
enum response serial_scan(const char * dataset, int dataset_size,const char * queries, int queries_size, int ts_length, float minimum_distance, struct stats_info *stats, unsigned int k)
{

    RESET_PARTIAL_COUNTERS()
    COUNT_PARTIAL_TIME_START
      
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
      
    FILE * dataset_file; 
    FILE * queries_file; 
    file_position_type total_records;
    file_position_type sz;
       
    COUNT_PARTIAL_INPUT_TIME_START    
    dataset_file = fopen (dataset,"rb");
    queries_file = fopen (queries,"rb");
    COUNT_PARTIAL_INPUT_TIME_END
    
    if (dataset_file == NULL) {
        fprintf(stderr, "File %s not found!\n", dataset_file);
        return FAILURE;
    }

    if (queries_file == NULL) {
        fprintf(stderr, "File %s not found!\n", queries_file);
        return FAILURE;
    }
    
    fseek(dataset_file, 0L, SEEK_END);
    sz = (file_position_type) ftell(dataset_file);
    fseek(dataset_file, 0L, SEEK_SET);
//    COUNT_PARTIAL_INPUT_TIME_END

    total_records = sz/ts_length * sizeof(ts_type);
    
    if (total_records < dataset_size) {
        fprintf(stderr, "File %s has only %llu records!\n", dataset, total_records);
        return FAILURE;
    }

    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT

 //   COUNT_PARTIAL_INPUT_TIME_START        
    fseek(queries_file, 0L, SEEK_END);
    sz = (file_position_type) ftell(queries_file);
    fseek(queries_file, 0L, SEEK_SET);
  //  COUNT_PARTIAL_INPUT_TIME_END
      
    total_records = sz/ts_length * sizeof(ts_type);

      
    if (total_records < queries_size) {
        fprintf(stderr, "File %s has only %llu records!\n", queries, total_records);
        exit(-1);
    }

    
    unsigned int ts_loaded = 0;    
    unsigned int q_loaded = 0;
        

    ts_type * candidate_ts = NULL;

    candidate_ts = malloc (sizeof(ts_type) * ts_length); 
    ts_type * query_ts = malloc(sizeof(ts_type) * ts_length);

    if(candidate_ts == NULL)
    {
          fprintf(stderr,"Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    if(query_ts == NULL)
    {
          fprintf(stderr,"Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    struct query_result * knn_results = calloc(k,sizeof(struct query_result));
    struct query_result result;
    ts_type kth_bsf = FLT_MAX;
    unsigned int cur_size = 0;
    
    while (q_loaded < queries_size)
    {
       //RESET_QUERY_COUNTERS ()
	  
        COUNT_PARTIAL_SEQ_INPUT
        COUNT_PARTIAL_INPUT_TIME_START	  
   	fread(query_ts, sizeof(ts_type), ts_length, queries_file);
        COUNT_PARTIAL_INPUT_TIME_END
	  
	for (int idx = 0; idx < k; ++idx)
	{
	    knn_results[idx].file_position = -1;
	    knn_results[idx].distance = FLT_MAX;      
	}

	kth_bsf = FLT_MAX;

        ts_type dist;
	ts_loaded = 0;
        int offset = 0;
        int i;
  	
	int * query_order = malloc(sizeof(int) * ts_length);
        if( query_order == NULL )
           return FAILURE;

	reorder_query(query_ts,query_order,ts_length);
	/*
	q_index *q_tmp = malloc(sizeof(q_index) * ts_length);

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
	  query_ts[i] = q_tmp[i].value;
          query_order[i] = q_tmp[i].index;
        }
        free(q_tmp);
	*/

        q_loaded++;

	
        COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START	  	
	fseek(dataset_file, 0L, SEEK_SET);
        COUNT_PARTIAL_INPUT_TIME_END
	  
	while (ts_loaded < dataset_size)
        { 
           COUNT_PARTIAL_SEQ_INPUT	  
   	   COUNT_PARTIAL_INPUT_TIME_START	  		  
           fread(candidate_ts, sizeof(ts_type), ts_length, dataset_file);
           COUNT_PARTIAL_INPUT_TIME_END

	   result =  knn_results[k-1];	   
	   kth_bsf = result.distance;
	     
	   dist = ts_euclidean_distance_reordered(query_ts, candidate_ts, offset,ts_length, kth_bsf,query_order);
	   
	   //dist = ts_euclidean_distance(query_ts, candidate_ts, ts_length, bsf);       
           //printf ("dist  = %g\n", dist);
           if (dist < kth_bsf)
	   {
	     struct query_result object_result;// =  malloc(sizeof(struct query_result));
	     object_result.file_position = ts_loaded;
	     object_result.distance =  dist;

	     queue_bounded_sorted_insert(knn_results, object_result, &cur_size,k);

	   }
           ts_loaded++;
        }

	for (unsigned int idx = 1; idx <= k; ++idx)
	{
	    COUNT_PARTIAL_TIME_END	  
	    result = knn_results[idx-1];
	    update_query_stats(stats,q_loaded, idx, result);
	    print_query_stats(stats, q_loaded, idx,queries);
	    RESET_QUERY_COUNTERS()
	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START		    
	}
	
	fflush(stdout);
	
        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START
	  
    }

    free(knn_results);
    free(query_ts);
    free(candidate_ts);

    COUNT_PARTIAL_INPUT_TIME_START
    if(fclose(dataset_file))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", dataset);
        return FAILURE;
    }

    if(fclose(queries_file))
    {   
        fprintf(stderr, "Error in dstree_file_loaders.c: Could not close the query filename %s", queries);
        return FAILURE;
    }    
    COUNT_PARTIAL_INPUT_TIME_END
      
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()

    return SUCCESS;	
 }



void queue_bounded_sorted_insert(struct  query_result *q, struct query_result d, unsigned int *cur_size, unsigned int k)
{
    struct query_result  temp;
    size_t i;
    size_t newsize;

    bool is_duplicate = false;
    for (unsigned int itr = 0 ; itr < *cur_size ; ++itr)
      {
	if (q[itr].distance == d.distance)
	  is_duplicate = true;
      }
   
    if (!is_duplicate)
      {
    
	/* the queue is full, overwrite last element*/
	if (*cur_size == k) {      
	  q[k-1].distance = d.distance;
	  q[k-1].file_position = d.file_position;
	}
	else
	  {
	    q[*cur_size].distance = d.distance;
	    q[*cur_size].file_position = d.file_position;      
	    ++(*cur_size);
	  }

	unsigned int idx,j;

	idx = 1;
    
	while (idx < *cur_size)
	  {
	    j = idx;
	    while ( j > 0 &&  ( (q[j-1]).distance > q[j].distance)) 
	      {
		temp = q[j];
		q[j].distance = q[j-1].distance;
		q[j].file_position = q[j-1].file_position;	
		q[j-1].distance = temp.distance;
		q[j-1].file_position = temp.file_position;		
		--j;
	      }
	    ++idx;
	  }
      }

}

void update_query_stats(struct stats_info *stats,
			unsigned int query_id,
			unsigned int found_knn,
			struct query_result bsf_result)
{


  stats->query_total_time  = partial_time;
  stats->query_total_input_time  = partial_input_time;
  stats->query_total_output_time = partial_output_time;
  stats->query_total_cpu_time    = partial_time-partial_input_time-partial_output_time;

  stats->query_total_seq_input_count   = partial_seq_input_count;
  stats->query_total_seq_output_count  = partial_seq_output_count;
  stats->query_total_rand_input_count  = partial_rand_input_count;
  stats->query_total_rand_output_count = partial_rand_output_count;

  stats->query_exact_distance = sqrtf(bsf_result.distance);
  stats->query_exact_file_position = bsf_result.file_position;
  
}


void print_query_stats(struct stats_info *stats, unsigned int query_num,unsigned int found_knn, char * queries)
{

        printf("Query_total_input_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats->query_total_input_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats->query_total_output_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats->query_total_cpu_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\t%u\n",
	       stats->query_total_time/1000000,	       
	       queries,
	       query_num,
	       found_knn
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%u\t%u\n",
	       stats->query_total_seq_input_count,
	       queries,
	       query_num,
	       found_knn
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%u\t%u\n",
	       stats->query_total_seq_output_count,
	       queries,
	       query_num,
	       found_knn
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%u\t%u\n",
	       stats->query_total_rand_input_count,
	       queries,
	       query_num,
	       found_knn
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%u\t%u\n",
	       stats->query_total_rand_output_count,
	       queries,
	       query_num,
	       found_knn
	       );


        printf("Query_exact_distance\t%f\t%s\t%u\t%u\n",
	       stats->query_exact_distance,
	       queries,
	       query_num,
	       found_knn
	       );

        printf("Query_exact_file_position\t%f\t%s\t%u\t%u\n",
	       stats->query_exact_file_position,
	       queries,
	       query_num,
	       found_knn
	       );		

	
}


void init_serial_stats(struct stats_info * stats)
{
    stats = malloc(sizeof(struct stats_info));
    if(stats == NULL) {
        fprintf(stderr,"Error in dstree_index.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    
    /*PER QUERY STATISTICS*/    

    stats->query_total_input_time = 0;
    stats->query_total_output_time = 0;
    stats->query_total_load_node_time = 0;
    stats->query_total_cpu_time = 0;
    stats->query_total_time = 0;    

    stats->query_total_seq_input_count = 0;
    stats->query_total_seq_output_count = 0;
    stats->query_total_rand_input_count = 0;
    stats->query_total_rand_output_count = 0;
    
    stats->query_exact_distance = 0;
    stats->query_exact_file_position = 0;    

    stats->query_lb_distance = 0;

    stats->query_tlb = 0;        
    
    /*SUMMARY STATISTICS FOR ALL QUERIES*/        

    stats->queries_total_input_time = 0;
    stats->queries_total_output_time = 0;
    stats->queries_total_load_node_time = 0;    
    stats->queries_total_cpu_time = 0;
    stats->queries_total_time = 0;    

    stats->queries_total_seq_input_count = 0;
    stats->queries_total_seq_output_count = 0;
    stats->queries_total_rand_input_count = 0;
    stats->queries_total_rand_output_count = 0;        

    stats->queries_min_tlb =  FLT_MAX;
    stats->queries_max_tlb = 0;
    stats->queries_sum_tlb = 0;
    stats->queries_sum_squares_tlb = 0;
    stats->queries_avg_tlb = 0;
    stats->queries_sd_tlb = 0;    
    
    stats->tlb_ts_count = 0;
    
    stats->total_ts_count = 0;	
    
    return SUCCESS;
}

enum response reorder_query(ts_type * query_ts, int * query_order, int ts_length)
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
	  query_ts[i] = q_tmp[i].value;
          query_order[i] = q_tmp[i].index;
        }
        free(q_tmp);

	return SUCCESS;
}


int znorm_comp(const void *a, const void* b)
{
    q_index* x = (q_index*)a;
    q_index* y = (q_index*)b;

    //    return abs(y->value) - abs(x->value);

    if (fabsf(y->value) > fabsf(x->value) )
       return 1;
    else if (fabsf(y->value) == fabsf(x->value))
      return 0;
    else
      return -1;
}

