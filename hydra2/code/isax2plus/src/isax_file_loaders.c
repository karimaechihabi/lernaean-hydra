//
//  isax_file_loaders.c
//  isax
//
//  Created by Kostas Zoumpatianos on 4/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "config.h"
#include "globals.h"
#include <stdio.h>
#include "isax_file_loaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/isax_node.h"
#include "../include/isax_index.h"
#include "../include/isax_node_record.h"
#include "../include/isax_query_engine.h"

void isax_query_ascii_file(const char *ifilename, int q_num, 
                           const char delimiter, isax_index *index,
						   float minimum_distance) {
	FILE * ifile;
	ifile = fopen (ifilename,"r");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
	char *ts_str = NULL; //= malloc(sizeof(char) * 2000);
    size_t linecap = 0;
    ssize_t linelen;
    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    
    COUNT_INPUT_TIME_START
    while ((linelen = getline(&ts_str, &linecap, ifile)) > 0 && q_loaded < q_num)
    {
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        ts_parse_str(ts_str, ts, index->settings->timeseries_size, &delimiter);   
        paa_from_ts(ts, paa, index->settings->paa_segments, 
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
	  query_result result;
	//result = exact_search(ts, paa, index, minimum_distance);
        fprintf(stderr, "%lld\n", result.raw_file_position);
        COUNT_TOTAL_TIME_END
	  //PRINT_STATS(result.distance)
#if VERBOSE_LEVEL == 2
        printf("[%0#6X]: Distance: %lf\n", 
               (unsigned long int) result.node, 
               result.distance);
#endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}
        
        q_loaded++;
        COUNT_INPUT_TIME_START
	}
    free(paa);
    free(ts);
    free(ts_str);
	fclose(ifile);

}

void isax_knn_query_binary_file(const char *ifilename,
				int q_num,
				isax_index *index,
				float minimum_distance,
				ts_type epsilon,
				ts_type delta,
				unsigned int k,
				unsigned int nprobes,
				unsigned char incremental
				)
  
{
    RESET_PARTIAL_COUNTERS ()
    COUNT_PARTIAL_TIME_START

    FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START        
    
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    COUNT_PARTIAL_RAND_INPUT      
    COUNT_PARTIAL_RAND_INPUT    
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END
    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int offset = 0;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    //ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    ts_type * query_paa = calloc(1,sizeof(ts_type) * index->settings->paa_segments);
    ts_type * query_ts_reordered = calloc(1,sizeof(ts_type) * ts_length);    
    int * query_order = calloc(1,sizeof(int) * ts_length);
    if( query_order == NULL )
      return FAILURE;

    
    while (q_loaded < q_num)
    {
        RESET_QUERY_COUNTERS()

        COUNT_PARTIAL_SEQ_INPUT      	  
        COUNT_PARTIAL_INPUT_TIME_START

	  
	fread(query_ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END

	reorder_query(query_ts,query_ts_reordered,query_order,ts_length);               
        q_loaded++;
        paa_from_ts(query_ts, query_paa, index->settings->paa_segments, 
                    index->settings->ts_values_per_paa_segment);
	if (incremental)
	{
	  exact_incr_knn_search(query_ts,
				query_paa,
				query_ts_reordered,
				query_order,
				offset,
				index,
				minimum_distance,
				epsilon,
				delta,
				k,
				q_loaded,
				ifilename,
				nprobes
				);
        }
	else if (nprobes)
	{
	  exact_ng_knn_search(query_ts,
			   query_paa,
			   query_ts_reordered,
			   query_order,
			   offset,
			   index,
			   minimum_distance,
			   k,
			   q_loaded,
			   ifilename,
			   nprobes
			   );
	  
	}
	else
	{
	  exact_de_knn_search(query_ts,
			   query_paa,
			   query_ts_reordered,
			   query_order,
			   offset,
			   index,
			   minimum_distance,
			   epsilon,
			   delta,
			   k,
			   q_loaded,
			   ifilename
			   );
	  
	}
	
	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START	
	}
    free(query_paa);
    free(query_ts);
    free(query_ts_reordered);
    free(query_order);
	

    
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
    
      fclose(ifile);

    
}

void isax_query_binary_file(const char *ifilename, int q_num, isax_index *index,
						   float minimum_distance, ts_type epsilon, ts_type delta) {
    RESET_PARTIAL_COUNTERS ()
    COUNT_PARTIAL_TIME_START

    FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START        
    
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    COUNT_PARTIAL_RAND_INPUT      
    COUNT_PARTIAL_RAND_INPUT    
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END
    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int offset = 0;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    //ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    ts_type * query_paa = calloc(1,sizeof(ts_type) * index->settings->paa_segments);
    ts_type * query_ts_reordered = calloc(1,sizeof(ts_type) * ts_length);    

    
    while (q_loaded < q_num)
    {
        RESET_QUERY_COUNTERS()

        COUNT_PARTIAL_SEQ_INPUT      	  
        COUNT_PARTIAL_INPUT_TIME_START

	  
	fread(query_ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END

	int * query_order = calloc(1,sizeof(int) * ts_length);
        if( query_order == NULL )
           return FAILURE;

	reorder_query(query_ts,query_ts_reordered,query_order,ts_length);        

	  
        paa_from_ts(query_ts, query_paa, index->settings->paa_segments, 
                    index->settings->ts_values_per_paa_segment);
        //COUNT_TOTAL_TIME_START
	query_result result = exact_search(query_ts, query_paa,query_ts_reordered, query_order, offset, index, minimum_distance, epsilon, delta);
        //query_result result = exact_search(ts, paa, index, minimum_distance);
        //COUNT_TOTAL_TIME_END

       
        q_loaded++;


        get_query_stats(index,1);
        print_query_stats(index,q_loaded, 1,ifilename);

	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START	
	  free(query_order);
	
	}
    free(query_paa);
    free(query_ts);
    free(query_ts_reordered);

    
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
    
	fclose(ifile);

    get_queries_stats(index, q_loaded);
    print_queries_stats(index);    
    
}


void isax_tlb_binary_file(const char *ifilename, int q_num, isax_index *index) {
    FILE * ifile;
    
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END
    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int offset = 0;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    ts_type * query_paa = calloc(1,sizeof(ts_type) * index->settings->paa_segments);

    
    while (q_loaded < q_num)
    {
        total_data_tlb = 0;
        total_node_tlb = 0;	
	total_ts_count = 0;
	leaf_nodes_count = 0;
	  
	fread(query_ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
  
        paa_from_ts(query_ts, query_paa, index->settings->paa_segments, 
                    index->settings->ts_values_per_paa_segment);
	isax2plus_calc_tlb(query_ts, query_paa, index, NULL);
        q_loaded++;

	print_tlb_stats(index, q_loaded, ifilename);
	
    }
    
    free(query_paa);
    free(query_ts);
    fclose(ifile);
    
}

void isax_index_ascii_file(const char *ifilename, file_position_type ts_num, 
                           const char delimiter, isax_index *index)
{
    double parse_time = 0;
    
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
    
    isax_node_record *new_record = isax_node_record_init(index->settings->paa_segments, 
                                                         index->settings->timeseries_size);
#ifdef BENCHMARK
    int percentage = (int) (ts_num / (file_position_type) 100);    
#endif
    COUNT_INPUT_TIME_START
    COUNT_TOTAL_TIME_START
    while ((linelen = getline(&ts_str, &linecap, ifile)) > 0 && ts_loaded<ts_num)
    {
        COUNT_INPUT_TIME_END
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        new_record->position = ts_loaded+1;
        COUNT_PARSE_TIME_START
        ts_parse_str(ts_str, new_record->ts, index->settings->timeseries_size, &delimiter);
        COUNT_PARSE_TIME_END
        //	printf("Adding: %lf %lf\n", new_record->ts[0], new_record->ts[1]);
        
        if(sax_from_ts(new_record->ts, new_record->sax, index->settings->ts_values_per_paa_segment, 
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            isax_fbl_index_insert(index, new_record);
            ts_loaded++;
            
            //COUNT_LOADED_RECORD()
            
#ifdef BENCHMARK
            if(ts_loaded % percentage == 0)
            {
                float distance = 0;
                COUNT_TOTAL_TIME_END
                COUNT_TOTAL_TIME_START
		  //PRINT_STATS(distance);
            }
#endif
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
        COUNT_INPUT_TIME_START  
	}
    
    isax_node_record_destroy(new_record);
    free(ts_str);
    COUNT_INPUT_TIME_START
	fclose(ifile);
    COUNT_INPUT_TIME_END
}

void isax_index_binary_file(const char *ifilename, file_position_type ts_num, isax_index *index)
{
    double parse_time = 0;
    
	FILE * ifile;
    COUNT_PARTIAL_RAND_INPUT    
    COUNT_PARTIAL_INPUT_TIME_START

	ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    COUNT_PARTIAL_INPUT_TIME_END      
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START          
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    COUNT_PARTIAL_INPUT_TIME_END
    
    file_position_type total_records = sz/index->settings->ts_byte_size;

    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START                
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END          
    
    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    
	
    file_position_type ts_loaded = 0;    
    
    isax_node_record *new_record = isax_node_record_init(index->settings->paa_segments, 
                                                         index->settings->timeseries_size);
#ifdef BENCHMARK
    int percentage = (int) (ts_num / (file_position_type) 100);    
#endif

    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
	new_record->position = (file_position_type) ftell(ifile);
        //new_record->ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
	COUNT_PARTIAL_SEQ_INPUT		
        COUNT_PARTIAL_INPUT_TIME_START	  	
        fread(new_record->ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END
        
        if(sax_from_ts(new_record->ts, new_record->sax, index->settings->ts_values_per_paa_segment, 
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            //fprintf(stderr, "%lf %lf\n", new_record->ts[0], new_record->ts[0]);
            isax_fbl_index_insert(index, new_record);
            ts_loaded++;
            
            //COUNT_LOADED_RECORD()
     
#ifdef BENCHMARK
            if(ts_loaded % percentage == 0)
            {
                float distance = 0;
                //COUNT_TOTAL_TIME_END
                //COUNT_TOTAL_TIME_START
		  //PRINT_STATS(distance)
            }
#endif
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    isax_node_record_destroy(new_record);
    COUNT_PARTIAL_INPUT_TIME_START
	fclose(ifile);
    COUNT_PARTIAL_INPUT_TIME_END
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

    //    return abs(y->value) - abs(x->value);

    if (fabsf(y->value) > fabsf(x->value) )
       return 1;
    else if (fabsf(y->value) == fabsf(x->value))
      return 0;
    else
      return -1;

}

