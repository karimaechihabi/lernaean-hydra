//
//  sfa.
//  SFA C version
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
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
#include "../include/sfa_file_loaders.h"
#include "../include/sfa_trie.h"
#include "../include/sfa_node.h"
#include "../include/sfa_file_buffer.h"
#include "../include/sfa_file_buffer_manager.h"

#ifdef VALUES
#include <values.h>
#endif
		
int main (int argc, char **argv)
{
    INIT_STATS()
    COUNT_TOTAL_TIME_START

    static char * dataset = "data_current.bin";
    static char *queries = "queries_1K_s14784_256.bin";

    
    static char * trie_path = "out/";
    static unsigned int dataset_size = 1000;
    static unsigned int queries_size = 5;
    static unsigned int time_series_size = 256;
    static unsigned int init_segments = 1;  
    static unsigned int leaf_size = 100;
    static double buffered_memory_size = 6439.2; 
    static int  use_ascii_input = 0;
    static int mode = 0;
    static float minimum_distance = FLT_MAX;
    struct sfa_trie * trie = NULL;
    boolean is_trie_new = 1;
    unsigned int sample_size = 1000;
    boolean is_norm = true;
    unsigned int histogram_type=0; // 0= equi_depth, 1=equi_frequency/width
    unsigned int lb_dist = 1; //0=sfa_lb_dist and 1 = dft_lb_dist

    unsigned int word_length = 16; //num of fourier coeff.
    unsigned int num_symbols = 8; //alphabet size of sfa word
    
    while (1)
    {    
        static struct option long_options[] =  {
            {"dataset", required_argument, 0, 'd'},
            {"ascii-input", required_argument, 0, 'a'},
            {"queries", required_argument, 0, 'q'},
            {"trie-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},
            {"sample-size", required_argument, 0, 'c'},	    
            {"queries-size", required_argument, 0, 'k'},
            {"mode", required_argument, 0, 'x'},
            {"buffer-size", required_argument, 0, 'b'},	    
	    {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
            {"alphabet-size", required_argument, 0, 'f'},
            {"histogram-type", required_argument, 0, 'e'},
            {"lb-dist", required_argument, 0, 'g'},	    
            {"leaf-size", required_argument, 0, 'l'},
            {"help", no_argument, 0, '?'}
        };
        
        /* getopt_long stores the option trie here. */
        int option_trie = 0;
        
        int c = getopt_long (argc, argv, "",
                             long_options, &option_trie);
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
                trie_path = optarg;
                break;
            case 'f':
                num_symbols = atoi(optarg);
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
                       \t--floating points. They can be binary of ascii.\n\
                       \t--However performance is faster with binary files\n\
                       \t--dataset XX \t\t\tThe path to the dataset file\n\
                       \t--queries XX \t\t\tThe path to the queries file\n\
                       \t--dataset-size XX \t\tThe number of time series to load\n\
                       \t--queries-size XX \t\tThe number of queries to run\n\
                       \t--mode: 0=trie, 1=query, 2=trie & query  3=calc_tlb\t\t\n\
                       \t--trie-path XX \t\tThe path of the output folder\n\
                       \t--buffer-size XX \t\tThe size of the buffer memory in MB\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--ascii-input X \t\t\0 for ascii files and 1 for binary files\n\
                       \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                       \t--histogram_type XX\t\t\t0 for equi-depth and 1 for equi-width\n\
                       \t--lb-dist XX\t\t\t0 for sfa_lb_list and 1 for dft_lb_dist\n\
                       \t--help\n\n\
                       \t--**********************EXAMPLES**********************\n\n\
                       \t--*********************TRIE MODE*********************\n\n\
                       \t--bin/sfa --dataset XX --dataset-size XX             \n\n\
                       \t--          --trie-path XX --timeseries-size XX --mode 0\n\n\
                       \t--*********************QUERY MODE*********************\n\n\
                       \t--bin/sfa --queries XX --queries-size XX             \n\n\
                       \t--           --trie-path XX --mode 1                 \n\n\
                       \t--*****************TRIE AND QUERY MODE***************\n\n\
                       \t--bin/sfa --dataset XX --dataset-size XX             \n\n\
                       \t--          --timeseries-size XX --trie-path XX      \n\n\
                       \t--           --queries XX --queries-size XX --mode 2  \n\n\
                       \t--****************************************************\n\n");
  
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
    
    if (mode == 0)  //only build and store the trie
    {

        RESET_PARTIAL_COUNTERS()
        COUNT_PARTIAL_TIME_START

        struct sfa_trie_settings * trie_settings = sfa_trie_settings_init(trie_path,
                                                                    time_series_size,   
                                                                    word_length,
								    num_symbols,
                                                                    leaf_size,          
								    buffered_memory_size,
								    is_norm,
								    histogram_type,
								    lb_dist,
								    sample_size,
								    is_trie_new);  

       if (trie_settings == NULL)
       { 
         fprintf(stderr, "Error main.c:  Could not initialize the trie settings.\n");
         return -1;              
       }
    
       trie = sfa_trie_init(trie_settings);
       trie->first_node = sfa_root_node_init(trie->settings);    
    

       if (trie == NULL)
       { 
          fprintf(stderr, "Error main.c:  Could not initialize the trie.\n");
          return -1;              
        }
        if (!use_ascii_input) {
 
  	    sfa_trie_set_bins(trie, dataset);
	  
	    if (!sfa_trie_binary_file(dataset, dataset_size, trie))
            { 
               fprintf(stderr, "Error main.c:  Could not build the trie.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	    trie->stats->idx_building_total_time  = partial_time;	
	    trie->stats->idx_building_input_time  = partial_input_time;
	    trie->stats->idx_building_output_time = partial_output_time;
	    trie->stats->idx_building_cpu_time    = partial_time
	                                        - partial_input_time
	                                        - partial_output_time;
	    trie->stats->idx_building_seq_input_count   = partial_seq_input_count;
	    trie->stats->idx_building_seq_output_count  = partial_seq_output_count;
	    trie->stats->idx_building_rand_input_count  = partial_rand_input_count;
	    trie->stats->idx_building_rand_output_count = partial_rand_output_count;	    

	    save_all_buffers_to_disk(trie);

            sfa_get_trie_stats (trie);
            sfa_print_trie_stats(trie, dataset);
	    
            //sfa_query_binary_file(queries, 100, trie, minimum_distance);
	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START
	      	    
  	    if (!sfa_trie_write(trie)) 
            { 
               fprintf(stderr, "Error main.c:  Could not save the trie to disk.\n");
               return -1;              
            }
	      
            COUNT_PARTIAL_TIME_END
	    trie->stats->idx_writing_total_time  = partial_time;	
	    trie->stats->idx_writing_input_time  = partial_input_time;
	    trie->stats->idx_writing_output_time = partial_output_time;
	    trie->stats->idx_writing_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    trie->stats->idx_writing_seq_input_count   = partial_seq_input_count;
	    trie->stats->idx_writing_seq_output_count  = partial_seq_output_count;
	    trie->stats->idx_writing_rand_input_count  = partial_rand_input_count;
	    trie->stats->idx_writing_rand_output_count = partial_rand_output_count;	    
        }
      
    }
    
    else if (mode == 1)  //read an existing trie and execute queries
    {
            RESET_PARTIAL_COUNTERS()
            COUNT_PARTIAL_TIME_START
	    is_trie_new = 0;
	    trie = sfa_trie_read(trie_path);
  	    if (trie == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the trie from disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	    trie->stats->idx_reading_total_time  = partial_time;	
	    trie->stats->idx_reading_input_time  = partial_input_time;
	    trie->stats->idx_reading_output_time = partial_output_time;
	    trie->stats->idx_reading_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    trie->stats->idx_reading_seq_input_count   = partial_seq_input_count;
	    trie->stats->idx_reading_seq_output_count  = partial_seq_output_count;
	    trie->stats->idx_reading_rand_input_count  = partial_rand_input_count;
	    trie->stats->idx_reading_rand_output_count = partial_rand_output_count;


            sfa_get_trie_stats (trie);
            sfa_print_trie_stats(trie,dataset);
	      
	    fprintf(stderr, ">>> Trie loaded successfully from: %s\n", trie_path);
	    if (use_ascii_input)
	    {
	      
	      // if (!sfa_query_ascii_file(queries, queries_size, DELIMITER, trie, minimum_distance))
              // { 
              //    fprintf(stderr, "Error main.c:  Could not execute the query.\n");
              //    return -1;              		  
      	      // }
              	      
	    }
	    else
	    {
	      sfa_query_binary_file(queries, queries_size, trie, minimum_distance);
	    }	    
    }
    else if (mode == 3)  //read an existing index and execute queries
    {
	    is_trie_new = 0;
	    trie = sfa_trie_read(trie_path);
  	    if (trie == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the trie from disk.\n");
               return -1;              
            }
	      
	    fprintf(stderr, ">>> Index loaded successfully from: %s\n", trie_path);
	    if (!use_ascii_input)
	    {
	      sfa_tlb_binary_file(queries, queries_size, trie, minimum_distance);
	    }	    
    }
    
    
    else
    {
      fprintf(stderr, "Please use a valid mode. run sfa --help for more information. \n");
      return -1;              
    }
      
    COUNT_TOTAL_TIME_END
    fprintf(stderr,"Sanity check: combined trieing and querying times should be less than: %f secs \n",
      	   total_time/1000000);
    
    sfa_trie_destroy(trie, trie->first_node, is_trie_new);
    free(trie->first_node);
    free(trie->stats);
    free(trie->settings);
    free(trie);
     
    //printf("\n");

    malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}

