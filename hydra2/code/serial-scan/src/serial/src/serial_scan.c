//
//  main.c
//  ds-tree C version
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#include "../config.h"
#include "../globals.h"



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include "../include/ts.h"
#include "../include/serial_scan_utils.h"

#ifdef VALUES
#include <values.h>
#endif
		
int main (int argc, char **argv)
{
    INIT_STATS()
    COUNT_TOTAL_TIME_START

    static char * dataset = "/home/karima/myDisk/data/Cgenerator/data_current.txt";
    static char * queries = "/home/karima/myDisk/data/Cgenerator/query_current.txt";
    
    static char * index_path = "out/";
    static int dataset_size = 2000;
    static int queries_size = 2000;
    static int timeseries_size = 256;
    static int paa_segments = 16;
    static int sax_cardinality = 8;
    static float minimum_distance = FLT_MAX;
    static int calc_tlb = 0;
    static int subsequence_matching = 0; //1 if subsequence and 0 if whole matching
    static unsigned int k = 1;    

    /* SHOULD USE DIST1 IF DATA IS NOT NORMALIZED. SHOULD ADD Z-NORMALIZATION EARLY ABDANDONNING FOR SUBSEQUENCE MATCHING*/    
    struct stats_info * stats = NULL;
    
    static int  use_ascii_input = 0;


    while (1)
    {
        static struct option long_options[] =  {
            {"dataset", required_argument, 0, 'd'},
            {"queries", required_argument, 0, 'q'},
            {"minimum-distance", required_argument, 0, 's'},
            {"dataset-size", required_argument, 0, 'z'},
            {"queries-size", required_argument, 0, 'a'},
            {"timeseries-size", required_argument, 0, 't'},
            {"subsequence-matching", required_argument, 0, 'n'},
            {"sax-cardinality", required_argument, 0, 'x'},
            {"calc-tlb", no_argument, 0, 'o'},
            {"help", no_argument, 0, 'h'},
            {"k", required_argument, 0, 'k'},	    
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
            case 's':
                 minimum_distance = atof(optarg);
                 break;

            case 'q':
                queries = optarg;
                break;

            case 'a':
                queries_size = atoi(optarg);
                break;

            case 'k':
                k = atoi(optarg);
                break;		
                if (k < 1)
		{
		  fprintf(stderr,"Please change the k to be greater than 0.\n");
		  exit(-1);
		}
                break;		

            case 'd':
                dataset = optarg;
                break;

            case 'z':
                dataset_size = atoi(optarg);
                break;

            case 't':
                timeseries_size = atoi(optarg);
                break;

            case 'x':
                sax_cardinality = atoi(optarg);
                break;

            case 'o':
                calc_tlb = 1;
                break;

            case 'n':
                subsequence_matching = 1;
                break;

            case 'h':
                printf("Usage:\n\
                       \t--dataset XX \t\t\tThe path to the dataset file\n\
                       \t--queries XX \t\t\tThe path to the queries file\n\
                       \t--dataset-size XX \t\tThe number of time series to load\n\
                       \t--queries-size XX \t\tThe number of queries to do\n\
                       \t--minimum-distance XX\t\tThe minimum distance we search (MAX if not set)\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--sax-cardinality XX\t\tThe maximum sax cardinality in number of bits (power of two).\n\
                       \t--calc_tlb\t\t\tSet for calculating the tlb, should not be set when gathering time stats\n\
                       \t--subsequence-matching \t\t\tSet to perform subsequence matching\n\
                       \t--help\n\n");
                return 0;
                break;

            default:
                exit(-1);
                break;
        }
    }

    stats = malloc(sizeof(struct stats_info));
    if(stats == NULL) {
        fprintf(stderr,"Error in serial_scan.c: Could not allocate memory for stats structure.\n");
        return FAILURE;
    }

    init_serial_stats(stats);
    
    if (!calc_tlb)
    {
      serial_scan(dataset, dataset_size,queries, queries_size, timeseries_size,minimum_distance,stats,k);      
    }

    //malloc_stats_print(NULL, NULL, NULL);
    COUNT_TOTAL_TIME_END

      printf("Total time is %f secs\n " , (total_time/1000000));
    
    return 0;


}
