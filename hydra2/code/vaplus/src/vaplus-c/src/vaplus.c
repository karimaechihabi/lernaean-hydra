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
#include "../include/stats.h"

#ifdef VALUES
#include <values.h>
#endif

struct stats_info  stats;

int main (int argc, char **argv)
{
    stats_init();
    INIT_STATS()
    COUNT_TOTAL_TIME_START

    static char * dataset = "/home/karima/myDisk/data/Cgenerator/data_current.txt";
    //static char * queries = "/home/karima/myDisk/data/Cgenerator/query_current.txt";
    static char *queries = "/home/karima/sdb1_mount/data/synthetic/queries_1K_s14784_256.bin";
    //static char *queries = "/home/karima/sdb1_mount/data/synthetic/data_100M_s1184_256.bin";
    static char * dataset_hists = "/home/karima/myDisk/data/Cgenerator/data_current_hists.txt";
    
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
    //printf("new code\n");
    unsigned int histogram_type=0; // 0= equi_depth, 1=equi_frequency/width
    unsigned int lb_dist = 1; //0=vaplus_lb_dist and 1 = dft_lb_dist
    static float epsilon = 0;  //by default perform exact search 
    static float delta = 1;  //by default perform exact search 
    ts_type r_delta = FLT_MAX;
    static unsigned int nprobes = 0;       
    static unsigned int k = 1;  //by default search for 1-NN
    
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
            {"queries-size", required_argument, 0, 'w'},
            {"mode", required_argument, 0, 'x'},
            {"buffer-size", required_argument, 0, 'b'},	    
	        {"minimum-distance", required_argument, 0, 's'},
            {"timeseries-size", required_argument, 0, 't'},
            {"num-bits", required_argument, 0, 'f'},
            {"histogram-type", required_argument, 0, 'e'},
            {"lb-dist", required_argument, 0, 'g'},	    
            {"leaf-size", required_argument, 0, 'l'},
   	        {"epsilon", required_argument, 0, 'i'},
   	        {"delta", required_argument, 0, 'h'},
            {"k", required_argument, 0, 'k'},	    
            {"nprobes", required_argument, 0, 'o'},
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
		 
	    case 'i':
                 epsilon = atof(optarg);
                 break;

	    case 'h':
                 delta = atof(optarg);
                 break;

            case 'w':
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
            case 'k':
                k = atoi(optarg);
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
		
            case 'o':
                nprobes = atoi(optarg);
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
    
    const char *index_filename = malloc(sizeof(char) * (strlen(index_path) + 15));
    index_filename = strcpy(index_filename, index_path);
    index_filename = strcat(index_filename, "root.idx\0");
	
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
       //index->first_node = vaplus_root_node_init(index->settings);    
    

       if (index == NULL)
       { 
          fprintf(stderr, "Error main.c:  Could not initialize the index.\n");
          return -1;              
        }
        if (!use_ascii_input) {
 
  	    vaplus_index_set_bins(index, dataset);
            
	    vaplus_index_build(index);

            /*
	    if (!vaplus_index_binary_file(dataset, dataset_size, index))
            { 
               fprintf(stderr, "Error main.c:  Could not build the index.\n");
               return -1;              
            }
             */

	    //save_all_buffers_to_disk(index);

	    
            //vaplus_query_binary_file(queries, 100, index, minimum_distance);
	      	    
  	    if (!vaplus_index_write(index)) 
            { 
               fprintf(stderr, "Error main.c:  Could not save the index to disk.\n");
               return -1;              
            }
            COUNT_PARTIAL_TIME_END
	    stats_update_idx_building_stats();
	    
            get_index_stats (index);
	    print_index_stats(dataset, 0);	    
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

            get_index_stats (index);
            print_index_stats(dataset,0);
	      
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
	      //calculate r_delta
	      if (delta > 0 && delta < 1)
		{
		  FILE *fp;
		  int bins;
		  double *x, *y;
		  int i,j;
		  if((fp=fopen(dataset_hists, "r"))!=NULL)
		    {
		      fscanf(fp, "%d", &bins);
		
		      x = (double *) calloc(bins, sizeof(double));
		      y = (double *) calloc(bins, sizeof(double));

		      for(int j=0; j<bins; j++)
			fscanf(fp, "%lf%lf", &x[j], &y[j]);
		      fclose(fp);		
		    }
		  else printf("Error opening data distribution file.\n");

		  for (j=0; (j<bins)&&(y[j]<delta); j++);
		  j--;
		  fprintf(stderr,"Using histogram bin %lf.\n", y[j]);
			
		  r_delta=x[j]+(((delta-y[j])*(x[j+1]-x[j]))/(y[j+1]-y[j]));
		  fprintf(stderr,"Using r_delta = %lf.\n", r_delta);
		}	    
	      //answer delta-epsilon kNN queries, including k=1
	      vaplus_query_binary_file(queries, queries_size, index,
				       minimum_distance, k, epsilon,r_delta,nprobes);	      
	    }	    
    }
   
    /*
    else if (mode == 3)  //read an existing index and execute queries
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
	      vaplus_tlb_binary_file(queries, queries_size, index, minimum_distance);
	    }	    
    }
    
    */
    else
    {
      fprintf(stderr, "Please use a valid mode. run vaplus --help for more information. \n");
      return -1;              
    }
    free(index_filename);      
    COUNT_TOTAL_TIME_END
    fprintf(stderr,"Sanity check: combined indexing and querying times should be less than: %f secs \n",
      	   total_time/1000000);
    
    vaplus_index_destroy(index, index->first_node, is_index_new);
    //free(index->first_node);
    free(index->settings);
    free(index);
     
    //printf("\n");

    malloc_stats_print(NULL, NULL, NULL);    
    return 0;
}

