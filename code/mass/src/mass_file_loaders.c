//
//  mass_file_loaders.c
//
//  Created by Karima Echihabi on 03/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//
//  Based on code by Michele Linardi on 01/01/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//



#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include "../include/mass_file_loaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <fftw3.h>
#include "../include/mass_query_engine.h"
#include "../include/calc_utils.h"
#include "../include/mass_instance.h"

enum response mass_new_binary_file(const char *ifilename, unsigned long long query_size, unsigned int q_num,
				   struct mass_instance *mass_inst,float minimum_distance)
{

    FILE * data_ifile;
    FILE * query_ifile;

    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT      
    COUNT_PARTIAL_INPUT_TIME_START
    data_ifile = fopen (mass_inst->settings->data_filename,"rb");
    query_ifile = fopen (ifilename,"rb");    
    COUNT_PARTIAL_INPUT_TIME_END
      
    if (query_ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    if (data_ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", mass_inst->settings->data_filename);
        exit(-1);
    }    
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START
    fseek(query_ifile, 0L, SEEK_END);    
    file_position_type sz = (file_position_type) ftell(query_ifile);
    fseek(query_ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END        
    file_position_type total_records = sz/query_size * sizeof(ts_type);
    ts_type bsf = FLT_MAX;
    
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_RAND_INPUT
    COUNT_PARTIAL_INPUT_TIME_START      
    fseek(data_ifile, 0L, SEEK_END);
    sz = (file_position_type) ftell(data_ifile);
    fseek(data_ifile, 0L, SEEK_SET);
    COUNT_PARTIAL_INPUT_TIME_END        
    unsigned long long series_size = mass_inst->settings->timeseries_size;
    total_records = sz/series_size * sizeof(ts_type);

    
    if (total_records < mass_inst->settings->timeseries_count) {
        fprintf(stderr, "File %s has only %llu records!\n", mass_inst->settings->data_filename, total_records);
        exit(-1);
    }


    unsigned int q_loaded = 0;
    unsigned long long ts_loaded = 0;

    ts_type * series_ts = fftwf_malloc ( sizeof ( ts_type ) * (series_size*2) ); // long series 
    ts_type * query_ts = fftwf_malloc ( sizeof ( ts_type ) * (series_size*2) ); // query series
    int i;
    //initialize both with arrays zero for later dot product calculations
    //the actual query and time series will be read into these arrays
    for (i = 0; i < (series_size*2); i++ )
    {
  	query_ts[i] = 0;
	series_ts[i] = 0;	
    }
    
    fftwf_complex *out; // long series in the frequency domain 
    fftwf_complex *out2; // query in the frequency domain
    fftwf_complex *out3; // dot product in frequency domain

    ts_type * final_dot_product_sum = fftwf_malloc ( sizeof ( ts_type ) * (series_size*2) );
    ts_type * output_dp =  fftwf_malloc (sizeof(ts_type)*(series_size-query_size+1));    
        ts_type * qt;
    
    unsigned long long nc = series_size + 1; // size of the long series and query in frequency domain
    out = fftwf_malloc ( sizeof ( fftwf_complex ) * nc );
    out2 = fftwf_malloc ( sizeof ( fftwf_complex ) * nc );
    out3 = fftwf_malloc ( sizeof ( fftwf_complex ) * nc );

    
    unsigned long long chunk_size = 0;
    struct query_result bsf_result;
    ts_type * query_mean_std =  fftwf_malloc(sizeof(ts_type) * ((series_size-query_size+1)*2));
    ts_type * series_mean_std =  fftwf_malloc(sizeof(ts_type) * ((series_size-query_size+1)*2));
    
    fftwf_plan plan_forward2;
    fftwf_plan plan_forward;	
    fftwf_plan plan_backward;
    
    plan_forward2 = fftwf_plan_dft_r2c_1d ( (series_size*2), query_ts, out2, FFTW_ESTIMATE );
    plan_forward = fftwf_plan_dft_r2c_1d ( (series_size*2), series_ts, out, FFTW_ESTIMATE );
    plan_backward = fftwf_plan_dft_c2r_1d ( (2*series_size), out3, final_dot_product_sum, FFTW_ESTIMATE );
    	
    while (q_loaded < q_num)
    {
  
        reset_query_stats(mass_inst);
	  
        COUNT_PARTIAL_SEQ_INPUT      
        COUNT_PARTIAL_INPUT_TIME_START
	fread(query_ts, sizeof(ts_type), query_size, query_ifile);
        COUNT_PARTIAL_INPUT_TIME_END
	  
	bsf_result.distance = FLT_MAX;
	bsf_result.offset = -1;
	
	mass_compute_mean_std(query_ts, query_size, query_size, query_mean_std);	  

	reverse_signal(query_ts,query_size);    

        fftwf_execute ( plan_forward2);	
	
	while(ts_loaded < mass_inst->settings->timeseries_count)
	{
            COUNT_PARTIAL_SEQ_INPUT      
            COUNT_PARTIAL_INPUT_TIME_START
  	    fread(series_ts, sizeof(ts_type), mass_inst->settings->timeseries_size, data_ifile);
            COUNT_PARTIAL_INPUT_TIME_END
	      
	    fftwf_execute ( plan_forward);
            
	    COUNT_PARTIAL_TIME_END
            mass_inst->stats->query_calc_fft_total_time  += partial_time;	
            mass_inst->stats->query_calc_fft_input_time  += partial_input_time;
	    mass_inst->stats->query_calc_fft_output_time += partial_output_time;
	    mass_inst->stats->query_calc_fft_cpu_time    += partial_time-partial_input_time-partial_output_time;
	    
	    mass_inst->stats->query_calc_fft_seq_input_count   += partial_seq_input_count;
	    mass_inst->stats->query_calc_fft_seq_output_count  += partial_seq_output_count;
	    mass_inst->stats->query_calc_fft_rand_input_count  += partial_rand_input_count;
	    mass_inst->stats->query_calc_fft_rand_output_count += partial_rand_output_count;
	      
    	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START
	      
	    sliding_dot_product(series_ts, query_ts, series_size, query_size,
				      out, out2, out3,
				      final_dot_product_sum,
				output_dp,
				plan_backward);
	      
	      
	    COUNT_PARTIAL_TIME_END
            mass_inst->stats->query_dot_product_fft_total_time  += partial_time;	
            mass_inst->stats->query_dot_product_fft_input_time  += partial_input_time;
	    mass_inst->stats->query_dot_product_fft_output_time += partial_output_time;
	    mass_inst->stats->query_dot_product_fft_cpu_time    += partial_time-partial_input_time-partial_output_time;
	    
	    mass_inst->stats->query_dot_product_fft_seq_input_count   += partial_seq_input_count;
	    mass_inst->stats->query_dot_product_fft_seq_output_count  += partial_seq_output_count;
	    mass_inst->stats->query_dot_product_fft_rand_input_count  += partial_rand_input_count;
	    mass_inst->stats->query_dot_product_fft_rand_output_count += partial_rand_output_count;
	      
    	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START
	      
	    mass_compute_mean_std(series_ts, series_size, query_size, series_mean_std);
   	    reverse_signal(query_ts,query_size);
            update_bsf_distance(output_dp,query_mean_std, series_mean_std, 0, series_size, query_size, false, series_ts,query_ts,&bsf_result);   	    	

	    COUNT_PARTIAL_TIME_END
            mass_inst->stats->query_calc_dist_total_time  += partial_time;	
            mass_inst->stats->query_calc_dist_input_time  += partial_input_time;
	    mass_inst->stats->query_calc_dist_output_time += partial_output_time;
	    mass_inst->stats->query_calc_dist_cpu_time    += partial_time-partial_input_time-partial_output_time;
	    
	    mass_inst->stats->query_calc_dist_seq_input_count   += partial_seq_input_count;
	    mass_inst->stats->query_calc_dist_seq_output_count  += partial_seq_output_count;
	    mass_inst->stats->query_calc_dist_rand_input_count  += partial_rand_input_count;
	    mass_inst->stats->query_calc_dist_rand_output_count += partial_rand_output_count;
	    
  	    ++ts_loaded;
    	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START
	    
	}
	    COUNT_PARTIAL_TIME_END

	mass_inst->stats->query_total_time  =  mass_inst->stats->query_dot_product_fft_total_time  +
	                                       mass_inst->stats->query_calc_fft_total_time         + 
  	                                       mass_inst->stats->query_calc_dist_total_time;
	mass_inst->stats->query_total_input_time  =  mass_inst->stats->query_dot_product_fft_input_time  +
	                                       mass_inst->stats->query_calc_fft_input_time         + 	  
 	                                       mass_inst->stats->query_calc_dist_input_time;
	mass_inst->stats->query_total_output_time  =  mass_inst->stats->query_dot_product_fft_output_time  +
	                                       mass_inst->stats->query_calc_fft_output_time         + 	  	  
  	                                       mass_inst->stats->query_calc_dist_output_time;
	mass_inst->stats->query_total_cpu_time  =  mass_inst->stats->query_dot_product_fft_cpu_time  +
	                                       mass_inst->stats->query_calc_fft_cpu_time         + 	  	  
  	                                       mass_inst->stats->query_calc_dist_cpu_time;	

	mass_inst->stats->query_total_seq_input_count  =  mass_inst->stats->query_dot_product_fft_seq_input_count  +
  	                                       mass_inst->stats->query_calc_fft_seq_input_count                    +	  
  	                                       mass_inst->stats->query_calc_dist_seq_input_count;
	mass_inst->stats->query_total_seq_output_count  =  mass_inst->stats->query_dot_product_fft_seq_output_count  +
  	                                       mass_inst->stats->query_calc_fft_seq_output_count                     +	  	  
  	                                       mass_inst->stats->query_calc_dist_seq_output_count;
	mass_inst->stats->query_total_rand_input_count  =  mass_inst->stats->query_dot_product_fft_rand_input_count  +
  	                                       mass_inst->stats->query_calc_fft_rand_input_count                     +	  	  
  	                                       mass_inst->stats->query_calc_dist_rand_input_count;
	mass_inst->stats->query_total_rand_output_count  =  mass_inst->stats->query_dot_product_fft_rand_output_count  +	
  	                                       mass_inst->stats->query_calc_fft_rand_output_count                      +	    
  	                                       mass_inst->stats->query_calc_dist_rand_output_count;

	mass_inst->stats->total_ts_count  =  ts_loaded;	           
        q_loaded++;
	ts_loaded = 0;
        
	mass_inst->stats->query_exact_distance = sqrtf(bsf_result.distance);
        mass_inst->stats->query_exact_offset = bsf_result.offset;
	
        mass_print_query_stats(mass_inst,q_loaded, ifilename);
        fseek(data_ifile, 0L, SEEK_SET);	
	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START

    }
     COUNT_PARTIAL_TIME_END
       
     fftwf_destroy_plan ( plan_forward2);
     fftwf_destroy_plan ( plan_forward);
     fftwf_destroy_plan ( plan_backward );	
     
     fftwf_free ( out );
     fftwf_free ( out2);
     fftwf_free ( out3 );
     fftwf_free ( output_dp);     
     fftwf_free ( final_dot_product_sum );
     fftwf_free ( query_ts );
     fftwf_free ( series_ts );
       
     free(query_ts);
     free(series_ts);     

     free(query_mean_std);
     free(series_mean_std);
     
     if(fclose(query_ifile))
     {   
        fprintf(stderr, "Error in mass_file_loaders.c: Could not close the query filename %s", ifilename);
        return FAILURE;
     }
     if(fclose(data_ifile))
     {   
        fprintf(stderr, "Error in mass_file_loaders.c: Could not close the data filename %s", mass_inst->settings->data_filename);
        return FAILURE;
     }

    mass_print_preprocessing_stats(mass_inst,mass_inst->settings->data_filename);
     
    return SUCCESS;
}




void  reset_query_stats(struct mass_instance *mass_inst)
{
    mass_inst->stats->query_calc_fft_input_time = 0;  
    mass_inst->stats->query_calc_fft_output_time = 0;
    mass_inst->stats->query_calc_fft_cpu_time = 0;
    mass_inst->stats->query_calc_fft_total_time = 0;

    mass_inst->stats->query_calc_fft_seq_input_count = 0;
    mass_inst->stats->query_calc_fft_seq_output_count = 0;
    mass_inst->stats->query_calc_fft_rand_input_count = 0;
    mass_inst->stats->query_calc_fft_rand_output_count = 0;    


    mass_inst->stats->query_dot_product_fft_input_time = 0;  
    mass_inst->stats->query_dot_product_fft_output_time = 0;
    mass_inst->stats->query_dot_product_fft_cpu_time = 0;
    mass_inst->stats->query_dot_product_fft_total_time = 0;

    mass_inst->stats->query_dot_product_fft_seq_input_count = 0;
    mass_inst->stats->query_dot_product_fft_seq_output_count = 0;
    mass_inst->stats->query_dot_product_fft_rand_input_count = 0;
    mass_inst->stats->query_dot_product_fft_rand_output_count = 0;    

    mass_inst->stats->query_calc_dist_input_time = 0;  
    mass_inst->stats->query_calc_dist_output_time = 0;
    mass_inst->stats->query_calc_dist_cpu_time = 0;
    mass_inst->stats->query_calc_dist_total_time = 0;

    mass_inst->stats->query_calc_dist_seq_input_count = 0;
    mass_inst->stats->query_calc_dist_seq_output_count = 0;
    mass_inst->stats->query_calc_dist_rand_input_count = 0;
    mass_inst->stats->query_calc_dist_rand_output_count = 0;    

    mass_inst->stats->query_total_input_time = 0;  
    mass_inst->stats->query_total_output_time = 0;
    mass_inst->stats->query_total_cpu_time = 0;
    mass_inst->stats->query_total_time = 0;

    mass_inst->stats->query_total_seq_input_count = 0;
    mass_inst->stats->query_total_seq_output_count = 0;
    mass_inst->stats->query_total_rand_input_count = 0;
    mass_inst->stats->query_total_rand_output_count = 0;    

    mass_inst->stats->query_exact_distance = FLT_MAX;
    mass_inst->stats->query_exact_offset = -1;

  
}
