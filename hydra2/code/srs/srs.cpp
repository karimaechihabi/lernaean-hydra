//
//  main.c
//  ds-tree C version
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

//#include "../include/systemutils.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
//#include "ts.h"
//#include "srs_file_loaders.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <getopt.h>
#include <cstdlib>
#include <dirent.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <boost/math/distributions/chi_squared.hpp>
#include "SRSInMemory.h"
#include "stats.hpp"



struct stats_info  stats;

double cal_thres(double c, double p_thres, int m);
template<class T>
void query_workload(SRS_In_Memory<T> * searcher,
		    int k,
		    int t,
		    double thres,
                    char *query_file_path,
		    unsigned int ts_length,
		    unsigned int queries_size);
		
int main (int argc, char **argv)
{


    stats_init();
    //INIT_STATS()

    stats_count_total_time_start();


    static char * dataset = "/home/karima/myDisk/data/Cgenerator/data_current.txt";
    static char * queries = "/home/karima/myDisk/data/Cgenerator/query_current.txt";
    
    static char * index_path = "out/";
    static unsigned int dataset_size = 1000;
    static unsigned int queries_size = 5;
    static unsigned int ts_length = 256;
    static unsigned int projected_dim = 16;    
    static unsigned int init_segments = 1;  
    static unsigned int leaf_size = 100;
    static double buffered_memory_size = 6439.2; 
    static int  use_ascii_input = 0;
    static int mode = 0;
    static float minimum_distance = FLT_MAX;
    static float epsilon = 0; //perform exact search by default
    static float delta = 1;    //perform exact search by default
    struct srs_index * index = NULL;
    boolean is_index_new = 1;
    int k = 1; //k neighbors in k-NN search
    
    //printf("new code\n");
    
    while (1)
    {    
        static struct option long_options[] =  {
            {"ascii-input", required_argument, 0, 'a'},
            {"buffer-size", required_argument, 0, 'b'},
            {"projected-dim", required_argument, 0, 'c'},	    
            {"dataset", required_argument, 0, 'd'},
            {"epsilon", required_argument, 0, 'e'},
            {"delta", required_argument, 0, 'f'},	    
            {"k", required_argument, 0, 'k'},
            {"queries", required_argument, 0, 'q'},
            {"index-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},
            {"queries-size", required_argument, 0, 'g'},
            {"mode", required_argument, 0, 'x'},
	    {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
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

	    case 'e':
                 epsilon = atof(optarg);
                 break;

	    case 'f':
                 delta = atof(optarg);
                 break;
		 
            case 'k':
                k = atoi(optarg);
                if (k < 1)
		{
		  fprintf(stderr,"Please change k to be greater than 1.\n");
		  exit(-1);
		}
                break;		 
            case 'g':
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
		
            case 'p':
                index_path = optarg;
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
		
            case 't':
                ts_length = atoi(optarg);
                break;

            case 'c':
                projected_dim = atoi(optarg);
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
                       \t--k XX \t\tThe number of nearest neighbors to return\n\
                       \t--queries-size XX \t\tThe number of queries to run\n\
                       \t--mode: 0=index, 1=query, 2=index & query  3=calc_tlb\t\t\n\
                       \t--index-path XX \t\tThe path of the output folder\n\
                       \t--buffer-size XX \t\tThe size of the buffer memory in MB\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--ascii-input X \t\t\0 for ascii files and 1 for binary files\n\
                       \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                       \t--help\n\n\
                       \t--**********************EXAMPLES**********************\n\n\
                       \t--*********************INDEX MODE*********************\n\n\
                       \t--bin/srs --dataset XX --dataset-size XX             \n\n\
                       \t--          --index-path XX --timeseries-size XX --mode 0\n\n\
                       \t--*********************QUERY MODE*********************\n\n\
                       \t--bin/srs --queries XX --queries-size XX             \n\n\
                       \t--           --index-path XX --mode 1                 \n\n\
                       \t--*****************INDEX AND QUERY MODE***************\n\n\
                       \t--bin/srs --dataset XX --dataset-size XX             \n\n\
                       \t--          --timeseries-size XX --index-path XX      \n\n\
                       \t--           --queries XX --queries-size XX --mode 2  \n\n\
                       \t--****************************************************\n\n");
		    printf("\n");
  
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

    char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
    index_full_filename = strcpy(index_full_filename, index_path);
    index_full_filename = strcat(index_full_filename, "index.bin");	
    
    if (mode == 0)  //only build and store the index
    {

        stats_reset_partial_counters(); 
        stats_count_partial_time_start();
	  /* 
       struct srs_index_settings * index_settings = srs_index_settings_init(index_path,
                                                                    time_series_size,   
                                                                    init_segments,       
                                                                    leaf_size,          
								    buffered_memory_size,
								    is_index_new);  

       if (index_settings == NULL)
       { 
         fprintf(stderr, "Error main.c:  Could not initialize the index settings.\n");
         return -1;              
       }
    
       index = srs_index_init(index_settings);
       index->first_node = srs_root_node_init(index->settings);

       if (index == NULL)
       { 
          fprintf(stderr, "Error main.c:  Could not initialize the index.\n");
          return -1;              
        }
	  
	   //n = dataset_size, d = original dim, m=projected dim
 	    indexer->build_index(n, d, m, data_file_path);
	   */
            SRS_In_Memory<float> * indexer = new SRS_In_Memory<float>(index_path);
    	    //n = dataset_size, d = original dim, m=projected dim
            indexer->build_index(dataset_size, ts_length, projected_dim, dataset);
            delete indexer;


            stats_count_partial_time_end();
            stats_update_idx_building_stats();

	    /*

	    RESET_PARTIAL_COUNTERS()
	    COUNT_PARTIAL_TIME_START
  	    if (!srs_index_write(index)) 
            { 
               fprintf(stderr, "Error main.c:  Could not save the index to disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	      //COUNT_TOTAL_TIME_END	      
	    index->stats.idx_writing_total_time  = partial_time;	
	    index->stats.idx_writing_input_time  = partial_input_time;
	    index->stats.idx_writing_output_time = partial_output_time;
	    index->stats.idx_writing_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    index->stats.idx_writing_seq_input_count   = partial_seq_input_count;
	    index->stats.idx_writing_seq_output_count  = partial_seq_output_count;
	    index->stats.idx_writing_rand_input_count  = partial_rand_input_count;
	    index->stats.idx_writing_rand_output_count = partial_rand_output_count;	    
	    */
  	     get_index_stats (&stats, index_full_filename);
             print_index_stats(stats, (char *)dataset, (unsigned int)1);
	    //COUNT_TOTAL_TIME_START	    
    }
    else if (mode == 1)  //read an existing index and execute queries
    {
	    /*
	    is_index_new = 0;
	    index = srs_index_read(index_path);
  	    if (index == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the index from disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	      //COUNT_TOTAL_TIME_END	      
	    index->stats.idx_reading_total_time  = partial_time;	
	    index->stats.idx_reading_input_time  = partial_input_time;
	    index->stats.idx_reading_output_time = partial_output_time;
	    index->stats.idx_reading_cpu_time    = partial_time
	                                          - partial_input_time
	                                          - partial_output_time;
	    index->stats.idx_reading_seq_input_count   = partial_seq_input_count;
	    index->stats.idx_reading_seq_output_count  = partial_seq_output_count;
	    index->stats.idx_reading_rand_input_count  = partial_rand_input_count;
	    index->stats.idx_reading_rand_output_count = partial_rand_output_count;

            
            srs_get_index_stats (index);
            srs_print_index_stats(index,dataset);
	    //COUNT_TOTAL_TIME_START
	    */    
	    SRS_In_Memory<float> * searcher = new SRS_In_Memory<float>(index_path);
            searcher->restore_index ();
	    //delta = p_thres, c= epsilon + 1
	    //t = max data points to look at, set it to dataset_size
            query_workload(searcher,
			   k,
			   dataset_size,
			   cal_thres((epsilon+1),delta, projected_dim),  
                           queries,
			   ts_length,
			   queries_size);

	   stats_count_partial_time_end();
           stats_reset_partial_counters();

            delete searcher;
    }
    /*
    else if (mode == 3)  //read an existing index and execute queries
    {
	    is_index_new = 0;
	    index = srs_index_read(index_path);
  	    if (index == NULL) 
            { 
               fprintf(stderr, "Error main.c:  Could not read the index from disk.\n");
               return -1;              
            }
	      
	    fprintf(stderr, ">>> Index loaded successfully from: %s\n", index_path);
	    if (!use_ascii_input)
	    {
	       if (!srs_tlb_binary_file(queries, queries_size, index, minimum_distance))
               { 
                  fprintf(stderr, "Error main.c:  Could not execute the query.\n");
                  return -1;              
               }
	    }	    
    }
    */    
    else
    {
      fprintf(stderr, "Please use a valid mode. run srs --help for more information. \n");
      return -1;              
    }

    
    stats_count_total_time_end();
    fprintf(stderr,"Sanity check: combined indexing and querying times should be less than: %f secs \n",
      	   stats.counters.total_time/1000000);
    

    free(index_full_filename);
    printf("\n");

    //malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}


template<class T>
void query_workload(SRS_In_Memory<T> * searcher,
		    int k,
		    int t,
		    double thres,
                    char *query_file_path,
		    unsigned int ts_length,
		    unsigned int queries_size)
{

  stats_reset_partial_counters();
  stats_count_partial_time_start();


  typedef typename Accumulator<T>::Type ResultType;
  int qn, d, gn, gk, tmp;

  stats_count_partial_rand_input(1);
  stats_count_partial_input_time_start();
  FILE *ifile = fopen(query_file_path, "rb");
  stats_count_partial_input_time_end();

  if (ifile == NULL) {
    fprintf(stderr, "File %s not found!\n", query_file_path);
    exit(-1);
  }

  stats_count_partial_rand_input(2);
  stats_count_partial_input_time_start();
  fseek(ifile, 0L, SEEK_END);
  file_position_type sz = (file_position_type) ftell(ifile);
  fseek(ifile, 0L, SEEK_SET);
  stats_count_partial_input_time_end();
  file_position_type total_records = sz/ts_length * sizeof(ts_type);

  if (total_records < queries_size) {
    fprintf(stderr, "File %s has only %llu records!\n", query_file_path, total_records);
    exit(-1);
  }
  
  unsigned int q_loaded = 0;
  T * query = new T[ts_length];
  std::vector<res_pair_raw<ResultType> > res;

  	
  while (q_loaded < queries_size)
  {      
      q_loaded++;
      stats_reset_query_counters();
	  
      stats_count_partial_seq_input(1);
      stats_count_partial_input_time_start();
      fread(query, sizeof(ts_type), ts_length, ifile);
      stats_count_partial_input_time_end();
	      
      searcher->knn_search(query, k, t + k - 1, thres, res);
      std::sort(res.begin(), res.end());
      for (unsigned int pos = 1; pos <= k; ++pos)
      {
	stats_count_partial_time_end();	
	stats_update_query_stats(sqrtf(res[pos-1].dist), -1);
	get_query_stats(&stats);
	print_query_stats(stats, query_file_path, q_loaded,pos);	
	stats_reset_query_counters();
	stats_reset_partial_counters();	
	stats_count_partial_time_start();
      }      	  
  }
  stats_count_partial_time_end();
  stats_reset_partial_counters();
      
     
  if(fclose(ifile))
    {   
      fprintf(stderr, "Error in srs_file_loaders.c: Could not close the query filename %s", query_file_path);
      exit (0); 
    }

  delete[] query;

}


double cal_thres(double c, double p_thres, int m) {
  if (p_thres >= 1) {
    return -1;
  }
  boost::math::chi_squared chi(m);
  return boost::math::quantile(chi, p_thres) / c / c;
}

