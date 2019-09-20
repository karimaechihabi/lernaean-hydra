//
//  main.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

//  Modified by Karima Echihabi on 01/08/2017 (karima.echihabi@gmail.com)
//  Copyright 2017 Paris Descartes University. All rights reserved.
  
#define PRODUCT "----------------------------------------------\
\nThis is the Adaptive Leaf iSAX index.\n\
Copyright (C) 2011-2014 University of Trento.\n\
----------------------------------------------\n\n"
#ifdef VALUES
#include <values.h>
#endif

#include "../../config.h"
#include "../../globals.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <float.h>

#include "ads/sax/sax.h"
#include "ads/sax/ts.h"
#include "ads/isax_visualize_index.h"
#include "ads/isax_file_loaders.h"
#include "ads/isax_visualize_index.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/isax_query_engine.h"


isax_index *idx;
void     INThandler(int);

int main (int argc, char **argv)
{

    INIT_STATS()
    COUNT_TOTAL_TIME_START
      
	signal(SIGINT, INThandler);

#ifndef BENCHMARK
    printf(PRODUCT);

#if VERBOSE_LEVEL == 0
    printf("Executing in silent mode. Please wait.\n");
#endif
#endif
    static char * dataset = "data_current.bin";
    static char * queries = "queries_current.bin";
    static char * index_path = "myexperiment/";

    static int dataset_size = 2000;
    static int queries_size = 2000;
    static int time_series_size = 256;
    static int paa_segments = 16;
    static int sax_cardinality = 8;
    static int leaf_size = 100;
    static int min_leaf_size = 10;
    static int initial_lbl_size = 100;
    static unsigned long long flush_limit = 1000;
    static int initial_fbl_size = 100;
    static char use_index = 0;
    static int complete_type = 0;
    static int total_loaded_leaves = 1;
    static int tight_bound = 0;
    static int aggressive_check = 0;
    static float minimum_distance = FLT_MAX;
    static int serial_scan = 0;
    static int min_checked_leaves = -1;
    static char calc_tlb = 0;

    while (1)
    {
        static struct option long_options[] =  {
            {"dataset", required_argument, 0, 'd'},
            {"complete-type", required_argument, 0, 'c'},
            {"use-index", no_argument, 0, 'a'},
            {"serial", no_argument, 0, 'j'},
            {"min-checked-leaves", required_argument, 0, 'u'},
            {"queries", required_argument, 0, 'q'},
            {"minimum-distance", required_argument, 0, 's'},
            {"index-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},
            {"queries-size", required_argument, 0, 'k'},
            {"timeseries-size", required_argument, 0, 't'},
            {"sax-cardinality", required_argument, 0, 'x'},
            {"leaf-size", required_argument, 0, 'l'},
            {"min-leaf-size", required_argument, 0, 'm'},
            {"initial-lbl-size", required_argument, 0, 'b'},
            {"flush-limit", required_argument, 0, 'f'},
            {"initial-fbl-size", required_argument, 0, 'i'},
            {"total-loaded-leaves", required_argument, 0, 'e'},
            {"calc-tlb", no_argument, 0, 'o'},
            {"tight-bound", no_argument, 0, 'n'},
            {"aggressive-check", no_argument, 0, 'g'},
            {"help", no_argument, 0, 'h'},
            {NULL, 0, NULL, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long (argc, argv, "",
                             long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
        	case 'j':
        		serial_scan = 1;
        		break;
            case 'g':
                 aggressive_check = 1;
                 break;

            case 's':
                 minimum_distance = atof(optarg);
                 break;

            case 'n':
                 tight_bound = 1;
                 break;

            case 'e':
                total_loaded_leaves = atoi(optarg);
                break;

            case 'c':
                complete_type = atoi(optarg);
                break;

            case 'q':
                queries = optarg;
                break;

            case 'k':
                queries_size = atoi(optarg);
                break;

            case 'd':
                dataset = optarg;
                break;

            case 'p':
                index_path = optarg;
                break;

            case 'z':
                dataset_size = atoi(optarg);
                break;

            case 't':
                time_series_size = atoi(optarg);
                break;

            case 'x':
                sax_cardinality = atoi(optarg);
                break;

            case 'l':
                leaf_size = atoi(optarg);
                break;

            case 'm':
                min_leaf_size = atoi(optarg);
                break;

            case 'b':
                initial_lbl_size = atoi(optarg);
                break;

            case 'f':
                flush_limit = atoll(optarg);
                break;

            case 'u':
            	min_checked_leaves = atoi(optarg);
            	break;

            case 'i':
                initial_fbl_size = atoi(optarg);
                break;

            case 'o':
                calc_tlb = 1;
                break;


            case 'h':
#ifdef BENCHMARK
                printf(PRODUCT);
#endif
                printf("Usage:\n\
                       \t--dataset XX \t\t\tThe path to the dataset file\n\
                       \t--queries XX \t\t\tThe path to the queries file\n\
                       \t--dataset-size XX \t\tThe number of time series to load\n\
                       \t--queries-size XX \t\tThe number of queries to do\n\
                       \t--minimum-distance XX\t\tThe minimum distance we search (MAX if not set)\n\
                       \t--use-index  \t\t\tSpecifies that an input index will be used\n\
                       \t--index-path XX \t\tThe path of the output folder\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--sax-cardinality XX\t\tThe maximum sax cardinality in number of bits (power of two).\n\
                       \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                       \t--min-leaf-size XX\t\tThe minimum size of each leaf\n\
                       \t--initial-lbl-size XX\t\tThe initial lbl buffer size for each buffer.\n\
                       \t--flush-limit XX\t\tThe limit of time series in memory at the same time\n\
                       \t--initial-fbl-size XX\t\tThe initial fbl buffer size for each buffer.\n\
                       \t--complete-type XX\t\t0 for no complete, 1 for serial, 2 for leaf\n\
                       \t--total-loaded-leaves XX\tNumber of leaves to load at each fetch\n\
                       \t--min-checked-leaves XX\tNumber of leaves to check at minimum\n\
                	   \t--tight-bound XX\t\tSet for tight bounds.\n\
                       \t--aggressive-check XX\t\tSet for aggressive check\n\
                       \t--serial\t\t\tSet for serial scan\n\
                       \t--calc_tlb\t\t\tSet for calculating the tlb, should not be set when gathering time stats\n\
                       \t--help\n\n");
                return 0;
                break;
            case 'a':
                use_index = 1;
                break;
            default:
                exit(-1);
                break;
        }
    }


    if (use_index) { 
        RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START	  
    	isax_index *idx = index_read(index_path);
        COUNT_PARTIAL_TIME_END
	idx->stats->idx_reading_total_time  = partial_time;	
	idx->stats->idx_reading_input_time  = partial_input_time;
	idx->stats->idx_reading_output_time = partial_output_time;
	idx->stats->idx_reading_cpu_time    = partial_time
	                                      - partial_input_time
	                                      - partial_output_time;
	idx->stats->idx_reading_seq_input_count   = partial_seq_input_count;
	idx->stats->idx_reading_seq_output_count  = partial_seq_output_count;
	idx->stats->idx_reading_rand_input_count  = partial_rand_input_count;
	idx->stats->idx_reading_rand_output_count = partial_rand_output_count;

	
    	idx->settings->tight_bound = tight_bound;
    	idx->settings->aggressive_check = aggressive_check;
    	idx->settings->total_loaded_leaves = total_loaded_leaves;
        idx->settings->min_leaf_size = min_leaf_size;



    	char sanity_test = 0;
	if (calc_tlb)
	{
	  cache_sax_file(idx);
	   ads_tlb_binary_file(queries, queries_size, idx);			   
	}
	else
	{
	  get_index_stats(idx);
	  print_index_stats(idx, dataset);
	  print_settings(idx->settings);
	  
     	  if(sanity_test)
	  {
    		cache_sax_file(idx);
	  }
    	  else
	  {
	    if(serial_scan)
	    {
		cache_sax_file(idx);
		isax_query_binary_file(queries, queries_size, idx, minimum_distance, min_checked_leaves, &exact_search_serial);
	    }
    	 }

	  RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START	  	
	    flush_all_leaf_buffers(idx, TMP_ONLY_CLEAN);
	  index_write(idx);
	  COUNT_PARTIAL_TIME_END
	    idx->stats->idx_writing_total_time  = partial_time;	
	  idx->stats->idx_writing_input_time  = partial_input_time;
	  idx->stats->idx_writing_output_time = partial_output_time;
	  idx->stats->idx_writing_cpu_time    = partial_time
	    - partial_input_time
	    - partial_output_time;
	  idx->stats->idx_writing_seq_input_count   = partial_seq_input_count;
	  idx->stats->idx_writing_seq_output_count  = partial_seq_output_count;
	  idx->stats->idx_writing_rand_input_count  = partial_rand_input_count;
	  idx->stats->idx_writing_rand_output_count = partial_rand_output_count;	    

	    
	  //clear_wedges(idx, NULL);
        }
    	isax_index_destroy(idx, NULL);
    }
    else      
    {

        RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START
	  
        char rm_command[256];
    	sprintf(rm_command, "rm -rf %s", index_path);
    	system(rm_command);

    	isax_index_settings * index_settings = isax_index_settings_init(index_path,         // INDEX DIRECTORY
    	                                                                    time_series_size,   // TIME SERIES SIZE
    	                                                                    paa_segments,       // PAA SEGMENTS
    	                                                                    sax_cardinality,    // SAX CARDINALITY IN BITS
    	                                                                    leaf_size,          // LEAF SIZE
    	                                                                    min_leaf_size,      // MIN LEAF SIZE
    	                                                                    initial_lbl_size,   // INITIAL LEAF BUFFER SIZE
    	                                                                    flush_limit,        // FLUSH LIMIT
    	                                                                    initial_fbl_size,   // INITIAL FBL BUFFER SIZE
    	                                                                    total_loaded_leaves,// Leaves to load at each fetch
    																		tight_bound,		// Tightness of leaf bounds
    																		aggressive_check,	// aggressive check
    																		1);					// new index
    	idx = isax_index_init(index_settings);
    	print_settings(idx->settings);
#ifdef CLUSTERED
        char s[255];
        sprintf(s, "rm -rf %s.*", dataset);
        system(s);
#endif


           isax_index_binary_file(dataset, dataset_size, idx);

	
        float distance = 0;
        FLUSHES++;
        flush_fbl(idx->fbl, idx);
        if(complete_type == 1) {
        	fprintf(stderr,">>> Completing index.\n");
        	complete_index(idx, dataset_size);
        }
        else if(complete_type == 2) {
        	fprintf(stderr,">>> Completing index.\n");
            complete_index_leafs(idx);
        }
        COUNT_PARTIAL_TIME_END
	idx->stats->idx_building_total_time  = partial_time;	
	idx->stats->idx_building_input_time  = partial_input_time;
	idx->stats->idx_building_output_time = partial_output_time;
	idx->stats->idx_building_cpu_time    = partial_time
	                                        - partial_input_time
	                                        - partial_output_time;
	idx->stats->idx_building_seq_input_count   = partial_seq_input_count;
	idx->stats->idx_building_seq_output_count  = partial_seq_output_count;
	idx->stats->idx_building_rand_input_count  = partial_rand_input_count;
	idx->stats->idx_building_rand_output_count = partial_rand_output_count;	    

	RESET_PARTIAL_COUNTERS()
	COUNT_PARTIAL_TIME_START


	index_write(idx);

        COUNT_PARTIAL_TIME_END
        idx->stats->idx_writing_total_time  = partial_time;	
	idx->stats->idx_writing_input_time  = partial_input_time;
	idx->stats->idx_writing_output_time = partial_output_time;
	idx->stats->idx_writing_cpu_time    = partial_time
	                                           - partial_input_time
	                                          - partial_output_time;
        idx->stats->idx_writing_seq_input_count   = partial_seq_input_count;
	idx->stats->idx_writing_seq_output_count  = partial_seq_output_count;
	idx->stats->idx_writing_rand_input_count  = partial_rand_input_count;
	idx->stats->idx_writing_rand_output_count = partial_rand_output_count;	    

        get_index_stats(idx);
	print_index_stats(idx,dataset);
	    isax_index_destroy(idx, NULL);
	    printf("\n");

    }

    //malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     fprintf(stderr, "Do you really want to quit? [y/n] ");
     c = getchar();
     if (c == 'y' || c == 'Y') {
    	 c = getchar();
    	 fprintf(stderr, "Do you want to save the index? [y/n] ");
    	 c = getchar();
    	 if (c == 'y' || c == 'Y') {
    		 flush_fbl(idx->fbl, idx);
    		 index_write(idx);
    	 }
    	 exit(0);
     }
     else
          signal(SIGINT, INThandler);
     getchar(); // Get new line character
}

