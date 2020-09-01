//
//  main.c
//  vaplus C version
//  Created by Karima Echihabi on 18/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#include "../config.h"
#include "../globals.h"
#include "../include/systemutils.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include "../include/ts.h"
#include "../include/vaplus_file_loaders.h"
#include "../include/vaplus_index.h"
#include "../include/vaplus_node.h"
#include "../include/vaplus_file_buffer.h"
#include "../include/vaplus_file_buffer_manager.h"

#ifdef VALUES
#include <values.h>
#endif
		
int main (int argc, char **argv)
{
    INIT_STATS()
    COUNT_TOTAL_TIME_START

    static char * dataset = "data_current.txt";
    static char *queries = "queries_1K_s14784_256.bin";
    
    static char * index_path = "out/";
    static unsigned int dataset_size = 1000;
    static unsigned int queries_size = 5;
    static unsigned int time_series_size = 256;
    static unsigned int init_segments = 1;  
    static unsigned int leaf_size = 100;
    static double buffered_memory_size = 6439.2; 
    static int  use_ascii_input = 0;
    static int mode = 0;
    static float minimum_distance = FLT_MAX;
    struct vaplus_index * index = NULL;
    boolean is_index_new = 1;
    unsigned int sample_size = 1000;
    boolean is_norm = true;
    unsigned int histogram_type=0; // 0= equi_depth, 1=equi_frequency/width
    unsigned int lb_dist = 1; //0=vaplus_lb_dist and 1 = dft_lb_dist

    unsigned int word_length = 16; //num of dimensions: fourier coeff
    unsigned int num_bits = 8; //number of bit per dimension for uniform bit allocation

    
    while (1)
    {    
        static struct option long_options[] =  {
            {"dataset", required_argument, 0, 'd'},
            {"ascii-input", required_argument, 0, 'a'},
            {"queries", required_argument, 0, 'q'},
            {"index-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},
            {"sample-size", required_argument, 0, 'c'},	    
            {"queries-size", required_argument, 0, 'k'},
            {"mode", required_argument, 0, 'x'},
            {"buffer-size", required_argument, 0, 'b'},	    
	    {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
            {"num-bits", required_argument, 0, 'f'},
            {"histogram-type", required_argument, 0, 'e'},
            {"lb-dist", required_argument, 0, 'g'},	    
            {"leaf-size", required_argument, 0, 'l'},
            {"help", no_argument, 0, '?'}
        };
        
        /* getopt_long stores the option index here. */
        int option_index = 0;
        
        int c = getopt_long (argc, argv, "",
                             long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
            case 'q':
                queries = optarg;
                break;
						
            case 's':
                 minimum_distance = atof(optarg);
                 break;

	    case 'b':
                 buffered_memory_size = atof(optarg);
                 break;
		 
            case 'k':
                queries_size = atoi(optarg);
                if (queries_size < 1)
		{
		  fprintf(stderr,"Please change the queries size to be greater than 0.\n");
		  exit(-1);
		}
                break;
		
            case 'd':
                dataset = optarg;
                break;

            case 'e':
                histogram_type = atoi(optarg);
                break;

	   case 'g':
                lb_dist = atoi(optarg);
                break;
		
            case 'p':
                index_path = optarg;
                break;
            case 'f':
                num_bits = atoi(optarg);
                break;
		
            case 'x':
                mode = atoi(optarg);
                break;
		
            case 'z':
                dataset_size = atoi(optarg);
                if (dataset_size < 1)
		{
		  fprintf(stderr,"Please change the dataset size to be greater than 0.\n");
		  exit(-1);
		}
                break;
            case 'c':
                sample_size = atoi(optarg);
                break;
		
            case 't':
                time_series_size = atoi(optarg);
                break;
		
            case 'l':
                leaf_size = atoi(optarg);
                if (leaf_size <= 1)
		{
		  fprintf(stderr,"Please change the leaf size to be greater than 1.\n");
		  exit(-1);
		}
                break;
		
            case '?':
                printf("Usage:\n\
                       \t--Queries and Datasets should be single precision\n\
                       \t--floating points in binary format.\n\
                       \t--However performance is faster with binary files\n\
                       \t--dataset XX \t\t\tThe path to the dataset file\n\
                       \t--queries XX \t\t\tThe path to the queries file\n\
                       \t--dataset-size XX \t\tThe number of time series to load\n\
                       \t--queries-size XX \t\tThe number of queries to run\n\
                       \t--mode: 0=index, 1=query, 2=index & query  3=calc_tlb\t\t\n\
                       \t--index-path XX \t\tThe path of the output folder\n\
                       \t--buffer-size XX \t\tThe size of the buffer memory in MB\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--ascii-input X \t\t\0 for ascii files and 1 for binary files\n\
                       \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                       \t--histogram_type XX\t\t\t0 for equi-depth and 1 for equi-width\n\
                       \t--lb-dist XX\t\t\t0 for vaplus_lb_list and 1 for dft_lb_dist\n\
                       \t--help\n\n\
                       \t--**********************EXAMPLES**********************\n\n\
                       \t--*********************INDEX MODE*********************\n\n\
                       \t--bin/vaplus --dataset XX --dataset-size XX             \n\n\
                       \t--          --index-path XX --timeseries-size XX --mode 0\n\n\
                       \t--*********************QUERY MODE*********************\n\n\
                       \t--bin/vaplus --queries XX --queries-size XX             \n\n\
                       \t--           --index-path XX --mode 1                 \n\n\
                       \t--*****************INDEX AND QUERY MODE***************\n\n\
                       \t--bin/vaplus --dataset XX --dataset-size XX             \n\n\
                       \t--          --timeseries-size XX --index-path XX      \n\n\
                       \t--           --queries XX --queries-size XX --mode 2  \n\n\
                       \t--****************************************************\n\n");
                printf ("\n");
                return 0;
                break;
            case 'a':
  	        use_ascii_input = atoi(optarg);
                break;
            default:
                exit(-1);
                break;
        }
    }

    minimum_distance = FLT_MAX;
    
    if (mode == 0)  //only build and store the index
    {

        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START

        struct vaplus_index_settings * index_settings = vaplus_index_settings_init(index_path,
                                                                    time_series_size,   
                                                                    word_length,
								    num_bits,
								    buffered_memory_size,
								    is_norm,
								    dataset_size,
								    is_index_new);  

       if (index_settings == NULL)
       { 
         fprintf(stderr, "Error main.c:  Could not initialize the index settings.\n");
         return -1;              
       }
    
       index = vaplus_index_init(index_settings);
    
       if (index == NULL)
       { 
          fprintf(stderr, "Error main.c:  Could not initialize the index.\n");
          return -1;              
        }
        if (!use_ascii_input) {
 
  	    vaplus_index_set_bins(index, dataset);
            
	    vaplus_index_build(index);

	      	    
  	    if (!vaplus_index_write(index)) 
            { 
               fprintf(stderr, "Error main.c:  Could not save the index to disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	    index->stats->idx_building_total_time  = partial_time;	
	    index->stats->idx_building_input_time  = partial_input_time;
	    index->stats->idx_building_output_time = partial_output_time;
	    index->stats->idx_building_cpu_time    = partial_time
	                                        - partial_input_time
	                                        - partial_output_time;
	    index->stats->idx_building_seq_input_count   = partial_seq_input_count;
	    index->stats->idx_building_seq_output_count  = partial_seq_output_count;
	    index->stats->idx_building_rand_input_count  = partial_rand_input_count;
	    index->stats->idx_building_rand_output_count = partial_rand_output_count;	    
	      
	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START

            COUNT_PARTIAL_TIME_END
	    index->stats->idx_writing_total_time  = partial_time;	
	    index->stats->idx_writing_input_time  = partial_input_time;
	    index->stats->idx_writing_output_time = partial_output_time;
	    index->stats->idx_writing_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    index->stats->idx_writing_seq_input_count   = partial_seq_input_count;
	    index->stats->idx_writing_seq_output_count  = partial_seq_output_count;
	    index->stats->idx_writing_rand_input_count  = partial_rand_input_count;
	    index->stats->idx_writing_rand_output_count = partial_rand_output_count;	    

            vaplus_get_index_stats (index);
            vaplus_print_index_stats(index, dataset);

        }
      
    }
    else if (mode == 1)  //read an existing index and execute queries
    {
            RESET_PARTIAL_COUNTERS()
            COUNT_PARTIAL_TIME_START
	    is_index_new = 0;
	    index = vaplus_index_read(index_path);
  	    if (index == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the index from disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	    index->stats->idx_reading_total_time  = partial_time;	
	    index->stats->idx_reading_input_time  = partial_input_time;
	    index->stats->idx_reading_output_time = partial_output_time;
	    index->stats->idx_reading_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    index->stats->idx_reading_seq_input_count   = partial_seq_input_count;
	    index->stats->idx_reading_seq_output_count  = partial_seq_output_count;
	    index->stats->idx_reading_rand_input_count  = partial_rand_input_count;
	    index->stats->idx_reading_rand_output_count = partial_rand_output_count;


            vaplus_get_index_stats (index);
            vaplus_print_index_stats(index,dataset);
	      
	    fprintf(stderr, ">>> Index loaded successfully from: %s\n", index_path);
	    if (use_ascii_input)
	    {
	      
	      // if (!vaplus_query_ascii_file(queries, queries_size, DELIMITER, index, minimum_distance))
              // { 
              //    fprintf(stderr, "Error main.c:  Could not execute the query.\n");
              //    return -1;              		  
      	      // }
              	      
	    }
	    else
	    {
	      vaplus_query_binary_file(queries, queries_size, index, minimum_distance);
	    }	    
    }
   
    else if (mode == 3)  //calculate tlb
    {
	    is_index_new = 0;
	    index = vaplus_index_read(index_path);
  	    if (index == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the index from disk.\n");
               return -1;              
            }
	      
	    fprintf(stderr, ">>> Index loaded successfully from: %s\n", index_path);
	    if (!use_ascii_input)
	    {
	      vaplus_tlb_binary_file(queries, queries_size, index);
	    }	    
    }
    
    else
    {
      fprintf(stderr, "Please use a valid mode. run vaplus --help for more information. \n");
      return -1;              
    }
      
    COUNT_TOTAL_TIME_END
    fprintf(stderr,"Sanity check: combined indexing and querying times should be less than: %f secs \n",
      	   total_time/1000000);
    
    vaplus_index_destroy(index, index->first_node, is_index_new);
    //free(index->first_node);
    free(index->stats);
    free(index->settings);
    free(index);
     
    //printf("\n");

    malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}

