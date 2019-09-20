//
//  sfa_file_loaders.c
//  sfa C version
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
#include "../include/sfa_file_loaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include "../include/sfa_query_engine.h"
#include "../include/sfa_trie.h"


void sfa_tlb_binary_file(const char *ifilename, int q_num, struct sfa_trie *trie,
						   float minimum_distance) {

    unsigned int ts_length = trie->settings->timeseries_size;
    unsigned int transforms_size = trie->settings->fft_size; 

    struct sfa_record * record = malloc (sizeof(struct sfa_record));

    int q_loaded = 0;
    //ts_type * ts = malloc(sizeof(ts_type) * trie->settings->timeseries_size);
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);
    //ts_type * query_paa = calloc(1,sizeof(ts_type) * trie->settings->paa_segments);

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

    unsigned char * query_sfa = NULL;
    query_sfa = calloc(transforms_size, sizeof(unsigned char));
      
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
        total_sfa_tlb = 0;
        total_dft_tlb = 0;
	total_ts_count = 0;
	leaf_nodes_count = 0;
      
	fread(query_ts, sizeof(ts_type), trie->settings->timeseries_size, ifile);

	for (int i =0; i< ts_length; ++i)
	     ts[i] =query_ts[i];

        fft_from_ts(trie, ts,  ts_out, query_fft, plan_forward);

        sfa_from_fft(trie, query_fft, query_sfa);

	sfa_calc_tlb(query_ts, query_fft, query_sfa, trie, trie->first_node);
       
        q_loaded++;

	print_tlb_stats(trie, q_loaded, ifilename);

    }

    fftwf_free(query_fft);
    fftwf_free (ts);
    fftwf_free (ts_out);
   
    free(query_sfa);    
    free(query_ts);

    fftwf_destroy_plan (plan_forward);

    
    fclose(ifile);
    
}


void sfa_query_binary_file(const char *ifilename, int q_num, struct sfa_trie *trie,
						   float minimum_distance) {
    RESET_PARTIAL_COUNTERS ()
    COUNT_PARTIAL_TIME_START
      
    double parse_time = 0;

    unsigned int ts_length = trie->settings->timeseries_size;
    unsigned int transforms_size = trie->settings->fft_size; 

    struct sfa_record * record = malloc (sizeof(struct sfa_record));

    int q_loaded = 0;
    ts_type * query_ts = calloc(1,sizeof(ts_type) * ts_length);

    ts_type * ts= NULL;
    ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
    
    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *) fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;

    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * query_fft = NULL;
    query_fft = fftwf_malloc ( sizeof ( ts_type ) * ts_length);

    unsigned char * query_sfa = NULL;
    query_sfa = calloc(transforms_size, sizeof(unsigned char));

      
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

    if (trie->settings->lb_dist == 0)
    {
	fprintf(stderr, "Using the SFA lower bounding distance\n");      
    }
    else if (trie->settings->lb_dist == 1)
    {
	fprintf(stderr, "Using the MBR DFT lower bounding distance \n");      
    }
    else
    {
      	fprintf(stderr, "Wrong lower bounding distance \n");
	exit(-1);
    }

    
    while (q_loaded < q_num)
    {
        RESET_QUERY_COUNTERS()

        COUNT_PARTIAL_SEQ_INPUT      	  
        COUNT_PARTIAL_INPUT_TIME_START	  
	fread(query_ts, sizeof(ts_type), trie->settings->timeseries_size, ifile);
        COUNT_PARTIAL_INPUT_TIME_END

	int * query_order = calloc(1,sizeof(int) * ts_length);
        if( query_order == NULL )
           return FAILURE;

	reorder_query(query_ts,query_ts_reordered,query_order,ts_length);        

	for (int i =0; i< ts_length; ++i)
	     ts[i] =query_ts[i];

        fft_from_ts(trie, ts,  ts_out, query_fft, plan_forward);

        sfa_from_fft(trie, query_fft, query_sfa);

	record->timeseries = query_ts;
        record->dft_transform = query_fft;	
        record->sfa_word = query_sfa;

	struct query_result result = exact_search(query_ts, query_fft,query_sfa,query_ts_reordered, query_order, offset, trie, minimum_distance);
       
        q_loaded++;

        get_query_stats(trie, query_ts, query_fft, q_loaded);	
        print_query_stats(trie,q_loaded, ifilename);

	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START	
	  free(query_order);
	
	}
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
    fftwf_free(query_fft);
    fftwf_free (ts);
    fftwf_free (ts_out);
    free(record);
   
    free(query_sfa);    
    free(query_ts);
    free(query_ts_reordered);

    fftwf_destroy_plan (plan_forward);


    
    COUNT_PARTIAL_TIME_END
    RESET_PARTIAL_COUNTERS()
    
	fclose(ifile);

    
}



enum response sfa_trie_binary_file(const char *ifilename, file_position_type ts_num, struct sfa_trie *trie)
{
    double parse_time = 0;
    unsigned int ts_length = trie->settings->timeseries_size;
    unsigned int transforms_size = trie->settings->fft_size; 

    struct sfa_record * record = malloc (sizeof(struct sfa_record));
      
    ts_type * ts= NULL;
    ts = fftwf_malloc ( sizeof ( ts_type ) * ts_length); 
    //ts = malloc ( sizeof ( ts_type ) * ts_length);
    
    ts_type * ts_orig= NULL;
    ts_orig = malloc ( sizeof ( ts_type ) * ts_length);
    
    if(ts == NULL)
    {
          fprintf(stderr,"Error in sfa_file_loaders.c: Could not allocate memory for ts.\n");
          return FAILURE;	
    }
    
    FILE * ifile; 
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_PARTIAL_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "Error in sfa_file_loaders.c: File %s not found!\n", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END    

    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START    
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    COUNT_PARTIAL_INPUT_TIME_END    
    file_position_type total_records = sz/trie->settings->timeseries_size * sizeof(ts_type);

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

    fftwf_complex *ts_out=NULL; 
    ts_out = (fftwf_complex *) fftwf_malloc ( sizeof ( fftwf_complex ) * (ts_length/2+1) ) ;

    fftwf_plan plan_forward=NULL;	
    plan_forward = fftwf_plan_dft_r2c_1d (ts_length, ts, ts_out, FFTW_ESTIMATE );

    ts_type * transform = NULL;
    //transform =  calloc(transforms_size+2, sizeof(ts_type));
    transform = fftwf_malloc ( sizeof ( ts_type ) * ts_length);

    unsigned char * sfa_word = NULL;
    sfa_word = calloc(transforms_size, sizeof(unsigned char));
      
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

	  fft_from_ts(trie, ts,  ts_out, transform, plan_forward);
         sfa_from_fft(trie, transform, sfa_word);

	//although ts, dft and sfa are already in mem array
	//pass a copy to the insert function in case the buffer
	//get flushed before the insertion completes.
	
	record->timeseries = ts;
        record->dft_transform = transform;	
        record->sfa_word = sfa_word;
	
	sfa_trie_insert(trie,0,record);
	
        ts_loaded++;	    

     }


   fftwf_destroy_plan (plan_forward);
   fftwf_free (ts);
   free (ts_orig);
   fftwf_free (ts_out);
   
   fftwf_free(transform);
   free(sfa_word);
   free(record);
   
    COUNT_PARTIAL_INPUT_TIME_START    
    if(fclose(ifile))
    {   
        fprintf(stderr, "Error in sfa_file_loaders.c: Could not close the filename %s", ifilename);
        return FAILURE;
    }
    COUNT_PARTIAL_INPUT_TIME_END    
    return SUCCESS;      
}

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


