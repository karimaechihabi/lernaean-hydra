//
//  main.c
//  Created by Karima Echihabi on 18/12/2018
//  Copyright 2018 Paris Descartes University. All rights reserved.



#include <ctype.h>
#include <float.h>
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
#include <iostream>
#include <fstream>
#include "stats.hpp"
#include <queue>
#include <chrono>
#include "hnswlib/hnswlib.h"


#include <unordered_set>

using namespace std;
using namespace hnswlib;

struct stats_info  stats;

//void
//query_workload(ts_type *massQ, size_t vecsize, size_t qsize, HierarchicalNSW<ts_type> &appr_alg, size_t vecdim,
//	       vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers, size_t k, char * queries, size_t efs);

void
query_workload(size_t vecsize,
	       size_t qsize, HierarchicalNSW<ts_type> &appr_alg,
	       size_t vecdim, vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
	       size_t k, char * queries, size_t efs);

/*static void
query_workload_per_efs(ts_type *massQ,
		       size_t vecsize,
		       size_t qsize,
		       HierarchicalNSW<ts_type> &appr_alg,
		       size_t vecdim,
		       vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
		       size_t k,
		       char * queries);
*/
void read_data(char * dataset,
	       ts_type ** pdata,
	       unsigned int ts_length,
	       unsigned int data_size,
	       unsigned int offset);

void add_data(HierarchicalNSW<ts_type> &appr_alg,
	      ts_type * data,
	      unsigned int ts_length,
	      unsigned int data_size);

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
    unsigned int buffer_size = 5000000; 
    static int  use_ascii_input = 0;
    static int mode = 0;
    static float minimum_distance = FLT_MAX;
    static int efs = 0; //perform exact search by default
    static float delta = 1;    //perform exact search by default
    struct srs_index * index = NULL;
    boolean is_index_new = 1;
    int k = 1; //k neighbors in k-NN search
    //int efConstruction = 40;    
    //int M = 16;
    //per ANN-benchmarks setting    
    int efConstruction = 500;        
    int M = 4;   

    
    //printf("new code\n");
    
    while (1)
    {    
        static struct option long_options[] =  {
            {"ascii-input", required_argument, 0, 'a'},
	    {"efConstruction", required_argument, 0, 'b'},
            {"projected-dim", required_argument, 0, 'c'},	    
            {"dataset", required_argument, 0, 'd'},
            {"efs", required_argument, 0, 'e'},
            {"delta", required_argument, 0, 'f'},	    
            {"k", required_argument, 0, 'k'},
            {"queries", required_argument, 0, 'q'},
            {"index-path", required_argument, 0, 'p'},
            {"dataset-size", required_argument, 0, 'z'},	    
            {"queries-size", required_argument, 0, 'g'},
            {"buffer-size", required_argument, 0, 'i'},	    
            {"mode", required_argument, 0, 'x'},
	    {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
            {"M", required_argument, 0, 'm'},
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
                 efConstruction = atoi(optarg);
                 break;

	    case 'e':
                 efs = atoi(optarg);
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

            case 'i':
	      buffer_size = atoi(optarg);
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
		
            case 'm':
                M = atoi(optarg);
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

    //HierarchicalNSW<ts_type> *appr_alg;
    minimum_distance = FLT_MAX;
    L2Space l2space(ts_length);

    char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
    index_full_filename = strcpy(index_full_filename, index_path);
    index_full_filename = strcat(index_full_filename, "index.bin");	

    
    if (mode == 0)  //only build and store the index
    {      
      //ts_type *mass = new ts_type[dataset_size * ts_length];

      
      stats_reset_partial_counters(); 
      stats_count_partial_time_start();

      unsigned int chunk_count = (unsigned  int) (dataset_size / buffer_size);
      unsigned int last_chunk_size = dataset_size % buffer_size;	
      unsigned int chunk_size = buffer_size;
      
      ts_type *data = new ts_type [chunk_size * ts_length];
      
      HierarchicalNSW<ts_type> appr_alg (&l2space, dataset_size, M, efConstruction);

      //ifstream input(dataset, ios::binary);
      //ifstream input("../../1M_d=4.bin", ios::binary);
      //add the data, one chunk at a time

      int i;
      for (i = 0; i < chunk_count; ++i)
	{
	  printf ("Loading %ld vectors of chunk %ld\n", chunk_size,i+1);
	  read_data(dataset,&data,ts_length,chunk_size,i*chunk_size);	  
	  add_data(appr_alg,data, ts_length,chunk_size);
	}
      //add the last chunk if needed
      if (last_chunk_size != 0)
	{
	  printf ("Loading %ld vectors of the last chunk %ld\n", last_chunk_size,i+1);
	  read_data(dataset,&data,ts_length,last_chunk_size,i*chunk_size);
	  add_data(appr_alg,data, ts_length,last_chunk_size);

      //for (int k = 0; k < last_chunk_size; ++k)
      //{	
 	  //   printf ("Timeseries[k] = %g \n", data[k]);
      //}
	}      

      appr_alg.saveIndex(index_full_filename);
      stats_count_partial_time_end();
      stats_update_idx_building_stats();
      get_index_stats (&stats, index_full_filename);
      print_index_stats(stats, (char *)dataset, (unsigned int)1);
    }
    else if (mode == 1)  //read an existing index and execute queries
    {
      //appr_alg = new HierarchicalNSW<ts_type>(&l2space,index_full_filename, false);
      HierarchicalNSW<ts_type> appr_alg (&l2space,index_full_filename, false);      
      vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
      //size_t k = 10;
      //cout << "Loading gt\n";
      //get_gt(mass, massQ, vecsize, qsize, l2space, vecdim, answers,k);
      //get_gt(massQA, massQ, mass, vecsize, qsize, l2space, vecdim, answers, (size_t)k);
      //cout << "Loaded gt\n";

      //ts_type *massQ = new ts_type[queries_size * ts_length];
      //ifstream input(queries, ios::binary);
      //ifstream input("../../1M_d=4.bin", ios::binary);
      //input.read((char *) massQ, queries_size * ts_length * sizeof(ts_type));
      //input.close();
      
      //for (int i = 0; i < 1; i++)
      query_workload(//massQ,
		     (size_t)dataset_size,
		     (size_t)queries_size,
		     appr_alg,
		     (size_t)ts_length,
		     answers,
		     (size_t)k,
		     queries,
		     (size_t)efs);

      
      stats_count_partial_time_end();
      stats_reset_partial_counters();
      

    }
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


void add_data(HierarchicalNSW<ts_type> &appr_alg,
	      ts_type * data,
	      unsigned int ts_length,
	      unsigned int data_size)

{
  for (int i = 0; i < 1; i++) {
    appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i);
  }
  
#pragma omp parallel for
  for (int i = 1; i < data_size; i++) {
#pragma omp critical
    appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i);
  }      
}

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
  
}
/*
void
query_workload(ts_type *massQ, size_t vecsize, size_t qsize, HierarchicalNSW<ts_type> &appr_alg, size_t vecdim,
		   vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers, size_t k, char * queries)
{

    //per ANN-Benchmarks settings
    vector<size_t> efs = { 10,20,40,80,120,200,24,400,600,800};

    for (size_t ef : efs) {
        appr_alg.setEf(ef);
        query_workload_per_efs(massQ, vecsize, qsize, appr_alg, vecdim, answers, k, queries);
    }
}

static void
query_workload_per_efs(ts_type *massQ,
		       size_t vecsize,
		       size_t qsize,
		       HierarchicalNSW<ts_type> &appr_alg,
		       size_t vecdim,
		       vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
		       size_t k,
		       char * queries)
{
    size_t correct = 0;
    size_t total = 0;
    stats_reset_partial_counters();
    stats_count_partial_time_start();
    //#pragma omp parallel for
    for (int i = 0; i < qsize; i++) {
      stats_reset_query_counters();
      //why they have 10?? Should it be k? Maybe ef gets the other k if (k>10)?
      std::priority_queue<std::pair<ts_type, labeltype >> result = appr_alg.searchKnn(massQ + vecdim * i, 10);
      size_t size = result.size();
      ts_type result_sorted[size];
      int idx = size-1;
      while (result.size())
      {
	result_sorted[idx] = result.top().first;
	result.pop();
	--idx;
      }
	
      for (unsigned int pos = 1; pos <= k; ++pos)
      {
	stats_count_partial_time_end();	
	stats_update_query_stats(sqrtf(result_sorted[pos-1]), -1);
	get_query_stats(&stats);
	print_query_stats(stats, queries, i+1,pos);	
	result.pop();
	stats_reset_query_counters();
	stats_reset_partial_counters();	
	stats_count_partial_time_start();				 
      }      	  
    }
      stats_count_partial_time_end();
      stats_reset_partial_counters();      
}

*/

void
query_workload(//ts_type *massQ,
		       size_t vecsize,
		       size_t qsize,
		       HierarchicalNSW<ts_type> &appr_alg,
		       size_t vecdim,
		       vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
		       size_t k,
		       char * queries,
		       size_t efs)
{
    size_t correct = 0;
    size_t total = 0;
    
    std::vector<ts_type> query(vecdim);
    
    stats_reset_partial_counters();
    stats_count_partial_time_start();

    stats_count_partial_rand_output(1);
    stats_count_partial_input_time_start();
    FILE *dfp = fopen(queries, "rb");
    stats_count_partial_input_time_end();
  
    //#pragma omp parallel for
    for (int i = 0; i < qsize; i++) {
      stats_reset_query_counters();
      
      stats_count_partial_seq_input(1);
      stats_count_partial_input_time_start();
      fread(query.data(), sizeof(ts_type), vecdim, dfp);
      printf ("query[%d] = %g \n",i,  query.data()[0]);

      stats_count_partial_input_time_end();
      
      //why they have 10?? Should it be k? Maybe ef gets the other k if (k>10)?
      appr_alg.setEf(efs);
      //std::priority_queue<std::pair<ts_type, labeltype >> result = appr_alg.searchKnn(massQ + vecdim * i, k);
      std::priority_queue<std::pair<ts_type, labeltype >> result = appr_alg.searchKnn(query.data(), k);
      size_t size = result.size();
      ts_type result_sorted[size];
      int idx = size-1;
      while (result.size())
      {
		result_sorted[idx] = result.top().first;
		result.pop();
		--idx;
      }
	
      for (unsigned int pos = 1; pos <= k; ++pos)
      {
	stats_count_partial_time_end();	
	stats_update_query_stats(sqrtf(result_sorted[pos-1]), -1);
	get_query_stats(&stats);
	print_query_stats(stats, queries, i+1,pos);	
	result.pop();
	stats_reset_query_counters();
	stats_reset_partial_counters();	
	stats_count_partial_time_start();				 
      }      	  
    }
      stats_count_partial_time_end();
      stats_reset_partial_counters();      
}
