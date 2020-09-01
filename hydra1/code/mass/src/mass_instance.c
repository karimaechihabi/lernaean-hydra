//
//  mass_instance.c
//
//  Created by Karima Echihabi on 03/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//
//  Based on code by Michele Linardi on 01/01/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.



#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef VALUES
#include <values.h>
#endif

#include <sys/stat.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/mass_instance.h"
#include "../include/calc_utils.h"
#include "../include/mass_buffer_manager.h"


/**
 This function initializes the settings of a mass instance
 */

struct mass_settings * mass_settings_init(
					  char * dataset,
					  char * queries,
						 unsigned long long timeseries_size, 
						 unsigned long long timeseries_count,
						 unsigned long long subsequence_size, 					  
					         double buffered_memory_size,
						 boolean is_db_new)
{
    struct mass_settings *settings = malloc(sizeof(struct mass_settings));
    if(settings == NULL) {
        fprintf(stderr,"Error in dstree_index.c: could not allocate memory for index settings.\n");
        return NULL;
    }

    settings->data_filename = dataset;
    settings->queries_filename = queries;
    
    settings->timeseries_size = timeseries_size;
    settings->timeseries_byte_size = sizeof(ts_type) * timeseries_size;
    settings->timeseries_count = timeseries_count;
    settings->subsequence_size = subsequence_size;
    settings->subsequence_byte_size = sizeof(ts_type) * subsequence_size;    
    settings->buffered_memory_size = buffered_memory_size;

    return settings;
}



/**
 This function initializes a mass instance
 @param mass_settings *settings
 @return mass_instance
 */
struct mass_instance * mass_inst_init(struct mass_settings *settings)
{
    struct mass_instance *mass_inst = malloc(sizeof(struct mass_instance));
    if(mass_inst == NULL) {
        fprintf(stderr,"Error in dstree_index.c: Could not allocate memory for the mass instance.\n");
        return NULL;
    }

    
    mass_inst->settings = settings;
    mass_inst->total_processed_ts = 0;
    mass_inst->buffer_manager = NULL;   

    mass_init_stats(mass_inst);
    
    return mass_inst;
}

enum response mass_init_stats(struct mass_instance * mass_inst)
{
    mass_inst->stats = malloc(sizeof(struct stats_info));
    if(mass_inst->stats == NULL) {
        fprintf(stderr,"Error in dstree_index.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    /*STATISTICS PER QUERY*/
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


    /*SUMMARY STATISTICS FOR ALL QUERIES*/
    mass_inst->stats->queries_calc_fft_input_time = 0;  
    mass_inst->stats->queries_calc_fft_output_time = 0;
    mass_inst->stats->queries_calc_fft_cpu_time = 0;
    mass_inst->stats->queries_calc_fft_total_time = 0;

    mass_inst->stats->queries_calc_fft_seq_input_count = 0;
    mass_inst->stats->queries_calc_fft_seq_output_count = 0;
    mass_inst->stats->queries_calc_fft_rand_input_count = 0;
    mass_inst->stats->queries_calc_fft_rand_output_count = 0;    
    
    mass_inst->stats->queries_dot_product_fft_input_time = 0;  
    mass_inst->stats->queries_dot_product_fft_output_time = 0;
    mass_inst->stats->queries_dot_product_fft_cpu_time = 0;
    mass_inst->stats->queries_dot_product_fft_total_time = 0;

    mass_inst->stats->queries_dot_product_fft_seq_input_count = 0;
    mass_inst->stats->queries_dot_product_fft_seq_output_count = 0;
    mass_inst->stats->queries_dot_product_fft_rand_input_count = 0;
    mass_inst->stats->queries_dot_product_fft_rand_output_count = 0;    

    mass_inst->stats->queries_calc_dist_input_time = 0;  
    mass_inst->stats->queries_calc_dist_output_time = 0;
    mass_inst->stats->queries_calc_dist_cpu_time = 0;
    mass_inst->stats->queries_calc_dist_total_time = 0;

    mass_inst->stats->queries_calc_dist_seq_input_count = 0;
    mass_inst->stats->queries_calc_dist_seq_output_count = 0;
    mass_inst->stats->queries_calc_dist_rand_input_count = 0;
    mass_inst->stats->queries_calc_dist_rand_output_count = 0;    

    mass_inst->stats->queries_total_input_time = 0;  
    mass_inst->stats->queries_total_output_time = 0;
    mass_inst->stats->queries_total_cpu_time = 0;
    mass_inst->stats->queries_total_time = 0;

    mass_inst->stats->queries_total_seq_input_count = 0;
    mass_inst->stats->queries_total_seq_output_count = 0;
    mass_inst->stats->queries_total_rand_input_count = 0;
    mass_inst->stats->queries_total_rand_output_count = 0;    

    /*COMBINED PREPROCESSING AND QUERIES STATISTICS*/
    mass_inst->stats->total_input_time = 0;  
    mass_inst->stats->total_output_time = 0;
    mass_inst->stats->total_cpu_time = 0;
    mass_inst->stats->total_time = 0;

    mass_inst->stats->total_seq_input_count = 0;
    mass_inst->stats->total_seq_output_count = 0;
    mass_inst->stats->total_rand_input_count = 0;
    mass_inst->stats->total_rand_output_count = 0;    
    
    mass_inst->stats->total_ts_count = 0;	
    mass_inst->stats->total_queries_count = 0;
    
    return SUCCESS;
}


void mass_print_preprocessing_stats(struct mass_instance * mass_inst, char * dataset)
{
        int id = -1;
	printf("Total_ts_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->total_ts_count,	       
	       dataset,
	       id);				
}


void mass_print_query_stats(struct mass_instance * mass_inst, unsigned int query_num, char * queries)
{
  
        printf("Query_calc_fft_input_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_input_time/1000000,
	       queries,
	       query_num); 
        printf("Query_calc_fft_output_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_output_time/1000000,
	       queries,
	       query_num);
        printf("Query_calc_fft_cpu_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_cpu_time/1000000,
	       queries,
	       query_num);	
        printf("Query_calc_fft_total_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_total_time/1000000,
	       queries,
	       query_num);
	
        printf("Query_calc_fft_seq_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_seq_input_count,
	       queries,
	       query_num);
	
        printf("Query_calc_fft_seq_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_seq_output_count,
	       queries,
	       query_num);
	
        printf("Query_calc_fft_rand_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_rand_input_count,
	       queries,
	       query_num);
	
        printf("Query_calc_fft_rand_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_fft_rand_output_count,
	       queries,
	       query_num);

	
        printf("Query_dot_product_fft_input_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_input_time/1000000,
	       queries,
	       query_num); 
        printf("Query_dot_product_fft_output_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_output_time/1000000,
	       queries,
	       query_num);
        printf("Query_dot_product_fft_cpu_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_cpu_time/1000000,
	       queries,
	       query_num);	
        printf("Query_dot_product_fft_total_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_total_time/1000000,
	       queries,
	       query_num);
	
        printf("Query_dot_product_fft_seq_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_seq_input_count,
	       queries,
	       query_num);
	
        printf("Query_dot_product_fft_seq_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_seq_output_count,
	       queries,
	       query_num);
	
        printf("Query_dot_product_fft_rand_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_rand_input_count,
	       queries,
	       query_num);
	
        printf("Query_dot_product_fft_rand_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_dot_product_fft_rand_output_count,
	       queries,
	       query_num);
	
        printf("Query_calc_dist_input_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_input_time/1000000,
	       queries,
	       query_num); 
        printf("Query_calc_dist_output_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_output_time/1000000,
	       queries,
	       query_num);
        printf("Query_calc_dist_cpu_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_cpu_time/1000000,
	       queries,
	       query_num);	
        printf("Query_calc_dist_total_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_total_time/1000000,
	       queries,
	       query_num);
	
        printf("Query_calc_dist_seq_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_seq_input_count,
	       queries,
	       query_num);
	
        printf("Query_calc_dist_seq_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_seq_output_count,
	       queries,
	       query_num);
	
        printf("Query_calc_dist_rand_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_rand_input_count,
	       queries,
	       query_num);
	
        printf("Query_calc_dist_rand_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_calc_dist_rand_output_count,
	       queries,
	       query_num); 

        printf("Query_total_input_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_total_input_time/1000000,
	       queries,
	       query_num); 
        printf("Query_total_output_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_total_output_time/1000000,
	       queries,
	       query_num);
        printf("Query_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_total_cpu_time/1000000,
	       queries,
	       query_num);	
        printf("Query_total_time_secs\t%lf\t%s\t%d\n",
	       mass_inst->stats->query_total_time/1000000,
	       queries,
	       query_num);
	
        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_total_seq_input_count,
	       queries,
	       query_num);
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_total_seq_output_count,
	       queries,
	       query_num);
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_total_rand_input_count,
	       queries,
	       query_num);
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_total_rand_output_count,
	       queries,
	       query_num); 

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       mass_inst->stats->query_exact_distance,
	       queries,
	       query_num
	       );	
        printf("Query_exact_offset\t%llu\t%s\t%d\n",
	       mass_inst->stats->query_exact_offset,
	       queries,
	       query_num
	       );
}

void mass_print_queries_stats(struct mass_instance * mass_inst)
{

        printf("Queries_calc_fft_input_time_secs \t  %f \n", mass_inst->stats->queries_calc_fft_input_time/1000000); 
        printf("Queries_calc_fft_output_time_secs \t  %f \n", mass_inst->stats->queries_calc_fft_output_time/1000000); 
        printf("Queries_calc_fft_cpu_time_secs \t  %f \n", mass_inst->stats->queries_calc_fft_cpu_time/1000000);
        printf("Queries_calc_fft_total_time_secs \t  %f \n", mass_inst->stats->queries_calc_fft_total_time/1000000);

        printf("Queries_calc_fft_seq_input_count \t  %llu  \n", mass_inst->stats->queries_calc_fft_seq_input_count); 
        printf("Queries_calc_fft_seq_output_count \t  %llu \n", mass_inst->stats->queries_calc_fft_seq_output_count);
        printf("Queries_calc_fft_rand_input_count \t  %llu  \n", mass_inst->stats->queries_calc_fft_rand_input_count); 
        printf("Queries_calc_fft_rand_onput_count \t  %llu \n", mass_inst->stats->queries_calc_fft_rand_output_count);

        printf("Queries_dot_product_fft_input_time_secs \t  %f \n", mass_inst->stats->queries_dot_product_fft_input_time/1000000); 
        printf("Queries_dot_product_fft_output_time_secs \t  %f \n", mass_inst->stats->queries_dot_product_fft_output_time/1000000); 
        printf("Queries_dot_product_fft_cpu_time_secs \t  %f \n", mass_inst->stats->queries_dot_product_fft_cpu_time/1000000);
        printf("Queries_dot_product_fft_total_time_secs \t  %f \n", mass_inst->stats->queries_dot_product_fft_total_time/1000000);

        printf("Queries_dot_product_fft_seq_input_count \t  %llu  \n", mass_inst->stats->queries_dot_product_fft_seq_input_count); 
        printf("Queries_dot_product_fft_seq_output_count \t  %llu \n", mass_inst->stats->queries_dot_product_fft_seq_output_count);
        printf("Queries_dot_product_fft_rand_input_count \t  %llu  \n", mass_inst->stats->queries_dot_product_fft_rand_input_count); 
        printf("Queries_dot_product_fft_rand_onput_count \t  %llu \n", mass_inst->stats->queries_dot_product_fft_rand_output_count);
	
        printf("Queries_calc_dist_input_time_secs \t  %f \n", mass_inst->stats->queries_calc_dist_input_time/1000000); 
        printf("Queries_calc_dist_output_time_secs \t  %f \n", mass_inst->stats->queries_calc_dist_output_time/1000000); 
        printf("Queries_calc_dist_cpu_time_secs \t  %f \n", mass_inst->stats->queries_calc_dist_cpu_time/1000000);
        printf("Queries_calc_dist_total_time_secs \t  %f \n", mass_inst->stats->queries_calc_dist_total_time/1000000);

        printf("Queries_calc_dist_seq_input_count \t  %llu  \n", mass_inst->stats->queries_calc_dist_seq_input_count); 
        printf("Queries_calc_dist_seq_output_count \t  %llu \n", mass_inst->stats->queries_calc_dist_seq_output_count);
        printf("Queries_calc_dist_rand_input_count \t  %llu  \n", mass_inst->stats->queries_calc_dist_rand_input_count); 
        printf("Queries_calc_dist_rand_onput_count \t  %llu \n", mass_inst->stats->queries_calc_dist_rand_output_count);

        printf("Queries_total_input_time_secs \t  %f \n", mass_inst->stats->queries_total_input_time/1000000); 
        printf("Queries_total_output_time_secs \t  %f \n", mass_inst->stats->queries_total_output_time/1000000); 
        printf("Queries_total_cpu_time_secs \t  %f \n", mass_inst->stats->queries_total_cpu_time/1000000);
        printf("Queries_total_time_secs \t  %f \n", mass_inst->stats->queries_total_time/1000000);	

        printf("Queries_total_seq_input_count \t  %llu  \n", mass_inst->stats->queries_total_seq_input_count); 
        printf("Queries_total_seq_output_count \t  %llu \n", mass_inst->stats->queries_total_seq_output_count);
        printf("Queries_total_rand_input_count \t  %llu  \n", mass_inst->stats->queries_total_rand_input_count); 
        printf("Queries_total_rand_onput_count \t  %llu \n", mass_inst->stats->queries_total_rand_output_count);

}

void mass_inst_destroy(struct mass_instance *mass_inst, boolean is_db_new)
{

   destroy_buffer_manager(mass_inst);
   
   free(mass_inst->stats);
   free(mass_inst->settings);
   free(mass_inst);

}

void destroy_buffer_manager(struct mass_instance *mass_inst)
{

  if(mass_inst->buffer_manager != NULL)
  {
    free(mass_inst->buffer_manager->mem_array);     
    free(mass_inst->buffer_manager);
  }
 
}

