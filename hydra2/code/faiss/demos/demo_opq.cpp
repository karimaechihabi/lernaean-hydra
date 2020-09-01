//
//  main.c
//  ds-tree C version
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.

/*
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>

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
*/

#include <cmath>
#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/time.h>
#include "../AutoTune.h"
#include "../index_io.h"

#include <getopt.h>
#include "../stats.hpp"

#include "../IndexPQ.h"
#include "../IndexIVFPQ.h"
#include "../IndexFlat.h"
#include "../index_io.h"


void query_workload(faiss::Index * index,
		    char * queries,
		    unsigned int queries_size,
		    unsigned int ts_length,
		    unsigned int k);

//ts_type *
void read_data(char * dataset,
	       ts_type ** pdata,
	       unsigned int ts_length,
	       unsigned int data_size,
	       unsigned int offset);
  
struct stats_info  stats;

int main (int argc, char **argv)
{


    stats_init();
    //INIT_STATS()

    stats_count_total_time_start();

    static char * dataset = "/home/karima/myDisk/data/Cgenerator/data_current.txt";
    static char * queries = "/home/karima/myDisk/data/Cgenerator/query_current.txt";
    static char * training = "/home/karima/myDisk/data/Cgenerator/training_current.txt";
    
    static char * index_path = "out/";
    static unsigned int dataset_size = 1000;
    static unsigned int queries_size = 5;
    static unsigned int ts_length = 256;
    static unsigned int projected_dim = 16;    
    static unsigned int init_segments = 1;  
    static unsigned int leaf_size = 100;
    unsigned int buffer_size = 1000; 
    static int  use_ascii_input = 0;
    static int mode = 0;
    static float minimum_distance = FLT_MAX;
    static float epsilon = 0; //perform exact search by default
    //static float delta = 1;    //perform exact search by default
    boolean is_index_new = 1;

    static unsigned int nprobes = 4;
    static unsigned int max_codes = 262144;
    static unsigned int ht = 118;
    
    int k = 1; //k neighbors in k-NN search

    //char * index_key = "OPQ16_64,IMI2x12,PQ8+16";
    char * index_key = "OPQ16_64,IMI2x8,PQ8+16";
    // const char *index_key = "Flat";
    //char * index_key = "OPQ16_64,IVFx,PQ8+16";
    //char * index_key = "OPQ16_64,IVF2x8,PQ8+16";
    
    //static unsigned int training_size =  64 * pow(2,10);
    static unsigned int training_size = 10000;
    
    faiss::Index * index;    
    index = faiss::index_factory(ts_length, index_key);
	
    while (1)
    {    
        static struct option long_options[] =  {
            {"ascii-input", required_argument, 0, 'a'},
            {"buffer-size", required_argument, 0, 'b'},
             {"projected-dim", required_argument, 0, 'c'},	    
            {"dataset", required_argument, 0, 'd'},
            {"epsilon", required_argument, 0, 'e'},
            {"nprobes", required_argument, 0, 'f'},	    
            {"queries-size", required_argument, 0, 'g'},
            {"training", required_argument, 0, 'h'},
            {"training-size", required_argument, 0, 'i'},
            {"index-key", required_argument, 0, 'j'},	    
	    {"k", required_argument, 0, 'k'},
            {"queries", required_argument, 0, 'q'},
            {"index-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},
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
                 buffer_size = atoi(optarg);
                 break;
		 
	    case 'i':
                 training_size = atoi(optarg);
                 break;
	    case 'e':
                 epsilon = atof(optarg);
                 break;

	    case 'f':
                 nprobes = atoi(optarg);
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

            case 'h':
                training = optarg;
                break;		

            case 'j':
                index_key = optarg;
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

    char *index_trained_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 17));
    index_trained_filename = strcpy(index_trained_filename, index_path);
    index_trained_filename = strcat(index_trained_filename, "index_trained.bin");

    char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 19));
    index_full_filename = strcpy(index_full_filename, index_path);
    index_full_filename = strcat(index_full_filename, "index_populated.bin");    
   
    
    //std::vector <ts_type> data (training_size * ts_length);

	
    if (mode == 0)  //only build and store the index
    {
          //training the index
      if (training_size != 0)
	{
	    ts_type *data = new ts_type [training_size * ts_length];
	    
	    stats_reset_partial_counters(); 
	    stats_count_partial_time_start();
	
	    read_data(training,&data,ts_length,training_size,0);

	    index = faiss::index_factory(ts_length, index_key);

	    printf ("Training on %ld vectors\n", training_size);
	    index->train(training_size, data);
	
	    stats_count_partial_time_end();
	    stats_update_idx_training_stats();
	    
	    write_index(index, index_trained_filename);	  

	    delete [] data;	    
	}
	  
	  //Loading the index
	  {
	    stats_count_partial_time_start();

	    unsigned int chunk_count = (unsigned  int) (dataset_size / buffer_size);
	    unsigned int last_chunk_size = dataset_size % buffer_size;	
	    unsigned int chunk_size = buffer_size;
		  
	    ts_type *data = new ts_type [chunk_size * ts_length];
	    
	    //add the data, one chunk at a time
	    unsigned int i;
	    for (i = 0; i < chunk_count; ++i)
	      {
		printf ("Loading %ld vectors of chunk %ld\n", chunk_size,i+1);
		read_data(dataset,&data,ts_length,chunk_size,i*chunk_size);
		index->add(chunk_size, data);	  
	      }
	
	    //add the last chunk if needed
	    if (last_chunk_size != 0)
	      {
		read_data(dataset,&data,ts_length,last_chunk_size,i*chunk_size);
		printf ("Loading %ld vectors of the last chunk %ld\n", last_chunk_size,i+1);
		index->add(last_chunk_size, data);	  
	      }
	    
	    write_index(index, index_full_filename);  
       
	    stats_count_partial_time_end();
	    stats_update_idx_building_stats(); //including training time
	    delete [] data;	    
	  }
	  
	get_index_stats (&stats, index_full_filename);
	print_index_stats(stats, (char *)dataset, (unsigned int)1);

	delete index;	
    }
    else if (mode == 1)  //read an existing index and execute queries
    {
      index = faiss::read_index(index_full_filename,faiss::IO_FLAG_READ_ONLY);

      faiss::ParameterSpace ps;
      ps.set_index_parameter(index, "nprobe", nprobes);
      //ps.set_index_parameter(index, "max_codes", max_codes);
      //ps.set_index_parameter(index, "ht", ht);
      //ps.display();

      query_workload(index,queries, queries_size, ts_length,k);      

      stats_count_partial_time_end();
      stats_reset_partial_counters();
      delete index;      
    }
    else
    {
      fprintf(stderr, "Please use a valid mode. run opq --help for more information. \n");
      return -1;              
    }

    
    stats_count_total_time_end();
    fprintf(stderr,"Sanity check: combined indexing and querying times should be less than: %f secs \n",
      	   stats.counters.total_time/1000000);
    

    free(index_trained_filename);
    free(index_full_filename);
    
    printf("\n");

    //malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}


//ts_type *
void read_data(char * dataset,
		   ts_type ** pdata,
		   unsigned int ts_length,
		   unsigned int data_size,
		   unsigned int offset)
{
  
  stats_count_partial_rand_output(1);
  stats_count_partial_input_time_start();
  FILE *dfp = fopen(dataset, "rb");
  fseek(dfp, 0, SEEK_SET);
  fseek(dfp, (unsigned long) offset * ts_length * sizeof(ts_type), SEEK_SET);
  fread(*pdata, sizeof(ts_type), data_size * ts_length, dfp);
  stats_count_partial_input_time_end();    
  
  stats_count_partial_seq_input(data_size);
  stats_count_total_ts_count(data_size);

  fclose(dfp);

  //return data;
  
}


void query_workload(faiss::Index *index,
		    char * queries,
		    unsigned int queries_size,
		    unsigned int ts_length,
		    unsigned int k)
{
  stats_reset_partial_counters();
  stats_count_partial_time_start();
  
  std::vector<ts_type> query(ts_length);
  
  faiss::Index::idx_t *I = new  faiss::Index::idx_t[k];
  float *D = new float[k];

  stats_count_partial_rand_output(1);
  stats_count_partial_input_time_start();
  FILE *dfp = fopen(queries, "rb");
  stats_count_partial_input_time_end();
  
  unsigned int q_loaded = 0;
  
  while (q_loaded < queries_size)
  {
    q_loaded++;
    stats_reset_query_counters();
	  
    stats_count_partial_seq_input(1);
    stats_count_partial_input_time_start();
    fread(query.data(), sizeof(ts_type), ts_length, dfp);
    stats_count_partial_input_time_end();
	      
    index->search(1,query.data(), k, D, I);
    
    for (unsigned int pos = 1; pos <= k; ++pos)
      {
	stats_count_partial_time_end();	
	stats_update_query_stats(sqrt(D[pos-1]), -1,I[pos-1]);
	get_query_stats(&stats);
	print_query_stats(stats, queries, q_loaded,pos);	
	stats_reset_query_counters();
	stats_reset_partial_counters();	
	stats_count_partial_time_start();
      }      	  
  }
  
  stats_count_partial_time_end();
  stats_reset_partial_counters();
  
  fclose(dfp);
  
}
