//
//  vaplus_file_loaders.c
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
#include "../include/vaplus_file_loaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include "../include/vaplus_query_engine.h"
#include "../include/vaplus_index.h"
/*
void vaplus_tlb_binary_file(const char *ifilename, int q_num, struct vaplus_index *index,
						   float minimum_distance) {

    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int transforms_size = index->settings->fft_size; 

    struct vaplus_record * record = malloc (sizeof(struct vaplus_record));

    int q_loaded = 0;
    //ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    //ts_type * query_paa = calloc(1,sizeof(ts_type) * index->settings->paa_segments);

    ts_type * ts= NULL;
    ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
    //ts = malloc ( sizeof ( ts_type ) * ts_length);
    
    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *) fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;

    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * query_fft = NULL;
    //transform =  calloc(transforms_size+2, sizeof(ts_type));
    query_fft = fftwf_malloc ( sizeof ( ts_type ) * ts_length);

    unsigned char * query_vaplus = NULL;
    query_vaplus = calloc(transforms_size, sizeof(unsigned char));
      
    FILE * ifile;
    
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/(ts_length * sizeof(ts_type) );
    fseek(ifile, 0L, SEEK_SET);
    unsigned int offset = 0;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    
    
    while (q_loaded < q_num)
    {
        total_vaplus_tlb = 0;
        total_dft_tlb = 0;
	total_ts_count = 0;
	leaf_nodes_count = 0;
      
	fread(query_ts, sizeof(ts_type), index->settings->timeseries_size, ifile);

	for (int i =0; i< ts_length; ++i)
	     ts[i] =query_ts[i];

        fft_from_ts(index, ts,  ts_out, query_fft, plan_forward);

        vaplus_from_fft(index, query_fft, query_vaplus);

	vaplus_calc_tlb(query_ts, query_fft, query_vaplus, index, index->first_node);
       
        q_loaded++;

	print_tlb_stats(index, q_loaded, ifilename);

    }

    fftwf_free(query_fft);
    fftwf_free (ts);
    fftwf_free (ts_out);
   
    free(query_vaplus);    
    free(query_ts);

    fftwf_destroy_plan (plan_forward);

    
    fclose(ifile);
    
}
*/
void vaplus_query_binary_file(const char *ifilename, int q_num, struct vaplus_index *index,
			      float minimum_distance, unsigned int k,
			      float epsilon, float r_delta, unsigned int nprobes) {
    RESET_PARTIAL_COUNTERS ()
    COUNT_PARTIAL_TIME_START
      
    double parse_time = 0;

    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int transforms_size = index->settings->fft_size; 

    struct vaplus_record * record = malloc (sizeof(struct vaplus_record));

    int q_loaded = 0;

    ts_type * query_ts_orig= NULL;
    query_ts_orig = (ts_type*) malloc ( sizeof ( ts_type ) * ts_length); 

    ts_type * query_ts= NULL;
    query_ts = (ts_type*) fftwf_malloc ( sizeof ( ts_type ) * ts_length); 

    fftwf_complex *query_ts_out=NULL; 
    query_ts_out = (fftwf_complex *)fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;
 
    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, query_ts, query_ts_out, FFTW_ESTIMATE );

    ts_type * query_fft_orig = NULL;
    query_fft_orig = (ts_type*) malloc ( sizeof ( ts_type ) * ts_length);

    ts_type * query_fft = NULL;
    query_fft = (ts_type*) fftwf_malloc ( sizeof ( ts_type ) * ts_length);

    unsigned int * query_approx = NULL;
    query_approx = (unsigned int*) malloc(transforms_size * sizeof(unsigned int));
      
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
    file_position_type total_records = sz/(ts_length * sizeof(ts_type) );
    fseek(ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END
    unsigned int offset = 0;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    
    ts_type * query_ts_reordered = calloc(1,sizeof(ts_type) * ts_length);    

    while (q_loaded < q_num)
    {
        q_loaded++;
        RESET_QUERY_COUNTERS()

        COUNT_PARTIAL_SEQ_INPUT      	  
        COUNT_PARTIAL_INPUT_TIME_START	  
	fread(query_ts_orig, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END

	int * query_order = calloc(1,sizeof(int) * ts_length);
        if( query_order == NULL )
           return FAILURE; 

	reorder_query(query_ts_orig,query_ts_reordered,query_order,ts_length);        

        
	for (unsigned int i =0; i< ts_length; ++i)
	     query_ts[i] =query_ts_orig[i];
         

        fft_from_ts(index, query_ts,  query_ts_out, query_fft, plan_forward);

        for (int unsigned i = 0; i < transforms_size; ++i)	       
	{
	     query_fft_orig[i] = (ts_type) roundf(query_fft[i]*100.0)/100.0; 
	}

        vaplus_from_fft(index, query_fft_orig, query_approx);

        //COUNT_TOTAL_TIME_START

	record->timeseries = query_ts;
        record->dft_transform = query_fft;	
        record->vaplus_word = query_approx;

	struct query_result result;

	if (nprobes)
	{
	  result = exact_ng_knn_search(query_ts_orig, query_fft_orig,query_approx,
				       query_ts_reordered, query_order, offset,
				       index, minimum_distance, q_loaded, ifilename,
				       k,nprobes);	  
	}
	else
	{
	  result = exact_de_knn_search(query_ts_orig, query_fft_orig,query_approx,
				       query_ts_reordered, query_order, offset,
				       index, minimum_distance, q_loaded, ifilename,
				       k,epsilon, r_delta);	  	  
	}
       

	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START	
	  free(query_order);
	
	}
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()

    fftwf_free(query_fft);
    fftwf_free (query_ts);
    free (query_ts_orig);
    free (query_fft_orig);
    fftwf_free (query_ts_out);
   
    free(query_approx);    
    free(query_ts_reordered);

    fftwf_destroy_plan (plan_forward);

    free(record);

    
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
    
	fclose(ifile);

    
}

/*

enum response vaplus_index_binary_file(const char *ifilename, file_position_type ts_num, struct vaplus_index *index)
{
    double parse_time = 0;
    unsigned int ts_length = index->settings->timeseries_size;
    unsigned int transforms_size = index->settings->fft_size; 

    struct vaplus_record * record = malloc (sizeof(struct vaplus_record));
      
    ts_type * ts= NULL;
    ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
    //ts = malloc ( sizeof ( ts_type ) * ts_length);
    
    ts_type * ts_orig= NULL;
    ts_orig = malloc ( sizeof ( ts_type ) * ts_length);
    
    if(ts == NULL)
    {
          fprintf(stderr,"Error in vaplus_file_loaders.c: Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    FILE * ifile; 
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "Error in vaplus_file_loaders.c: File %s not found!\n", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END    

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
    
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
	
    file_position_type ts_loaded = 0;    
    
    int percentage = 100;
    if (percentage > 100)
    {
       percentage = (int) (ts_num / (file_position_type) 100);
    }

    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *) fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;

    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * transform = NULL;
    //transform =  calloc(transforms_size+2, sizeof(ts_type));
    transform = fftwf_malloc ( sizeof ( ts_type ) * ts_length);

    unsigned char * vaplus_word = NULL;
    vaplus_word = calloc(transforms_size, sizeof(unsigned char));
      
    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        COUNT_PARTIAL_SEQ_INPUT
	COUNT_PARTIAL_INPUT_TIME_START
        fread(ts_orig, sizeof(ts_type), ts_length, ifile);

	for (int i =0; i< ts_length; ++i)
	     ts[i] =ts_orig[i];
	  
        COUNT_PARTIAL_INPUT_TIME_END

	  fft_from_ts(index, ts,  ts_out, transform, plan_forward);
	  /*
	  fftwf_execute ( plan_forward);
        ts_out[0][1] = 0;
        int j = 0;
	   
       //since normalizing, ignore first coefficent because it is 0., start k =1
       for(int k = 1; k< transforms_size/2+1; ++k)
       {
	     transform[j] = ts_out[k][0];
	     transform[j+1] =  ts_out[k][1];
	     j +=2;
       }
	       //fft_from_ts(index, ts, ts_out, transform, plan_forward);
	
        vaplus_from_fft(index, transform, vaplus_word);

	//although ts, dft and vaplus are already in mem array
	//pass a copy to the insert function in case the buffer
	//get flushed before the insertion completes.
	
	record->timeseries = ts;
        record->dft_transform = transform;	
        record->vaplus_word = vaplus_word;
	
	//char * cur_vaplus_word = (char *) index->buffer_manager->current_vaplus_record;
        //memcpy(vaplus_word, cur_vaplus_word, transforms_size * sizeof(char));
       	
	//vaplus_index_insert(index,index->first_node,0,record);
	vaplus_index_insert(index,0,record);
	
        ts_loaded++;	    

     }


   fftwf_destroy_plan (plan_forward);
   fftwf_free (ts);
   //free(ts);
   free (ts_orig);
   fftwf_free (ts_out);
   
   fftwf_free(transform);
   free(vaplus_word);
   free(record);
   
    COUNT_PARTIAL_INPUT_TIME_START    
    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in vaplus_file_loaders.c: Could not close the filename %s", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END    
    return SUCCESS;      
}
*/

enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length)
{
  
        struct q_index *q_tmp = malloc(sizeof(struct q_index) * ts_length);
        int i;
	
        if( q_tmp == NULL )
	  return FAILURE;

	for( i = 0 ; i < ts_length ; i++ )
        {
          q_tmp[i].value = query_ts[i];
          q_tmp[i].index = i;
        }
	
        qsort(q_tmp, ts_length, sizeof(struct q_index),znorm_comp);

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
    struct q_index* x = (struct q_index*)a;
    struct q_index* y = (struct q_index*)b;


    if (fabsf(y->value) > fabsf(x->value) )
       return 1;
    else if (fabsf(y->value) == fabsf(x->value))
      return 0;
    else
      return -1;

}


