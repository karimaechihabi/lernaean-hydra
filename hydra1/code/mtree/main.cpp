/**
 ****************************************************************************
 ** Copyright (C) 2012 Michele Dallachiesa
 ** All rights reserved.
 **
 ** Warning: This program is protected by copyright law and international
 ** treaties. Unauthorized reproduction and distribution of this program,
 ** or any portion of it, may result in severe civil and criminal penalties,
 ** and will be persecuted to the maximum extent possible under the law.
 ****************************************************************************
 ** Author: Michele Dallachiesa <dallachiesa@disi.unitn.it>
 ****************************************************************************
 **/

#include "main.h"
#include "common.h"
#include "dataset.h"
#include "ts.h"
#include "baseline.h"
#include "mspknn.h"
#include "pknn.h"
#include "euclidean.h"
#include "mtree.h"
#include "stats.h"
#include <getopt.h>

//#include "dust.h"

using namespace std;
using namespace boost;

/* globals */
OPT o;
int lineOutput = 1;
int random_seed = 1;
struct stats_info stats;


/* prototypes */
int main(int argc, char **argv);
void initOpt(int argc, char **argv);
void help();
void cleanUp();
void experiment_query();


/*******************************************/


int default_K = 1;
int default_N = 1000000; //dataset size
int default_m = 500;
//int default_m = 1;
int default_n = 256; //data series length
int num_samples = 0;


const char * dataset = "data_100M_s1184_256.bin";
const char * queries = "queries_1K_s14784_256.bin";
const char * index_path = "myexperiment/";


static int dataset_size = 1000000;
static int queries_size = 100;
static int time_series_size = 256;
static int leaf_size = 1000;
static char use_index = 0;
static float minimum_distance = FLT_MAX;
static float min_util = 0.2;

   
double default_perturbation_mean = 0.0;
double default_perturbation_stddev = 0.5;
double default_outlier_probab= 0.0;
double default_outlier_stddev = default_perturbation_stddev * 5;
int default_pruning = PRUNING_METRIC_LSCAN;

const char *dataset_names[] = { "50words","Beef","CBF","Coffee","ECG200","FaceAll","FaceFour","Gun_Point","Lighting2",
                                "Lighting7","OSULeaf","OliveOil","SwedishLeaf","Trace","Two_Patterns","fish","synthetic_control",
                                "wafer","yoga","ChlorineConcentration",
                                "CinC_ECG_torso","Cricket_X","Cricket_Y","Cricket_Z","DiatomSizeReduction","ECGFiveDays","FacesUCR",
                                "Haptics","InlineSkate","ItalyPowerDemand","MALLAT","MedicalImages","MoteStrain","SonyAIBORobotSurface",
                                "SonyAIBORobotSurfaceII","Symbols","TwoLeadECG","WordsSynonyms","uWaveGestureLibrary_X",
                                "Adiac","StarLightCurves",
                                "uWaveGestureLibrary_Y","uWaveGestureLibrary_Z",
                                "NonInvasiveFatalECG_Thorax1","NonInvasiveFatalECG_Thorax2", NULL};


/*******************************************/

#define TEST_DATASET_NAME "Coffee"


/*******************************************/


void experiment_query(void) {

    //default_m = 100;
    //default_K = 2;


    fprintf(stderr, "Generating dataset ....\n");

    Dataset data;
    /*  
    dataset.synthetic2(default_N, default_m, default_n,
                       default_perturbation_mean, default_perturbation_stddev,
                       default_outlier_probab, default_outlier_stddev);
    TS query = dataset.walk();
    */
    stats_reset_partial_counters();
    stats_count_partial_time_start();

    data.loadFromFileBin(dataset, default_N,default_n);

    //TS query = dataset.loadQueryFromFileBin("/home/karima/sdb1_mount/data/synthetic/data_size1M_seed1184_len256_double_znorm.bin",defult_N,default_n);
    fprintf(stderr, "Constructing the M-tree ....\n");
    //LOG << "Constructing M-tree ....\n";
    ElapsedTime time_indexing;
    time_indexing.begin();
    MTREE mtree = MTREE();	
    //mtree.load(dataset, 0.001);
    mtree.load(data, num_samples);
    stats_count_partial_time_end();
    time_indexing.end();   
    stats_count_partial_time_end();
    stats_update_index_stats();    
    print_index_stats(stats,dataset);        
	
    stats_reset_partial_counters();
    stats_count_partial_time_start();

    ifstream queries_file(queries);
    
    assert(queries_file.is_open());

    if (!queries_file.is_open()) {
        FATAL("unable to open file " << queries);
    }

    TS query;
    for (int q_loaded = 1 ; q_loaded <= queries_size;++q_loaded)
    {// top-k

        mtree.kdist_candidates.clear();
        //LOG << "Query " << i << ":\n";
	stats_reset_query_counters();      
	query.loadFromFile(queries_file, default_n); 
	vector<int> candidates = mtree.find_topk(query, default_K).first;	

	print_query_stats(stats, q_loaded,queries);
	stats_reset_partial_counters();
	stats_count_partial_time_start();


	/*
        ElapsedTime time_querying;
        time_querying.begin();
        vector<int> candidates = mtree.find_topk(query, default_K).first;
        time_querying.end();
        LOG << "Indexing time = " << time_indexing.t/1000.0 << "s -- Querying time =  " << time_querying.t/1000.0 << "s \n";
	*/
	
    }
    
    queries_file.close();
        
  
}



int main (int argc, char **argv)
{

  random_seed = 12345;

    
  while (1)
    {    
      static struct option long_options[] =  {
	{"dataset", required_argument, 0, 'd'},
	{"queries", required_argument, 0, 'q'},
	{"index-path", required_argument, 0, 'p'},
	{"dataset-size", required_argument, 0, 'z'},
	{"queries-size", required_argument, 0, 'k'},
	{"minimum-distance", required_argument, 0, 's'},
	{"min-util", required_argument, 0, 'u'},
	{"timeseries-size", required_argument, 0, 't'},
	{"leaf-size", required_argument, 0, 'l'},
	{"use-index", no_argument, 0, 'e'},	    
	{"help", no_argument, 0, '?'},
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
               
 	case 'l':
	  leaf_size = atoi(optarg);
	  break;
	case 'u':
	  min_util = atof(optarg);
	  break;	  
	case '?':
#ifdef BENCHMARK
	  printf(PRODUCT);
#endif
	  printf("Usage:\n\
                       \t--dataset XX \t\t\tThe path to the dataset file\n\
                       \t--queries XX \t\t\tThe path to the queries file\n\
                       \t--dataset-size XX \t\tThe number of time series to load\n\
                       \t--queries-size XX \t\tThe number of queries to do\n\
                       \t--minimum-util XX\t\tThe minimum utilization of a node\n\
                       \t--minimum-distance XX\t\tThe minimum distance we search (MAX if not set)\n\
                       \t--index-path XX \t\tThe path of the output folder\n\
                       \t--timeseries-size XX\t\tThe size of each time series\n\
                       \t--leaf-size XX\t\t\tThe maximum size of each leaf\n\
                       \t--use-index  \t\t\tBuild the index and run queries\n\
                       \t--help\n\n");
	  return 0;
	  break;
	case 'e':
	  use_index = 1;
	  break;		
	default:
	  exit(-1);
	  break;
        }
    }

  default_n = time_series_size;
  default_N = dataset_size;
  
  //num_samples = (int)MAX(MIN(leaf_size, ceil(((double)dataset_size)/leaf_size)), leaf_size*min_util;
  num_samples = leaf_size;
  stats_init();
  stats.total_ts_count = dataset_size; 
  stats_count_total_time_start(); 
  
  //LOG << "INT limits: " << std::numeric_limits<int>::min() << " " << std::numeric_limits<int>::max() << "\n";
    
  experiment_query();

  
  stats_count_total_time_end();
  fprintf(stderr,"Sanity check: combined times should be less than: %f secs \n",
	 stats.counters.total_time/1000000);

  
  return 0;
}


void  initOpt(int argc, char **argv) {
    int            c;

    cout << "argv: ";
    for(int i = 0; i < argc; i++) {
        cout << argv[i] << " ";
    }
    cout << "\n\n";

    /* default params */

    o.nRuns = 1;
    o.experimentIds.clear();

    o.mspknn_func_C = 1;
    o.mspknn_func_R = 1;

    o.generate_perturbed_datasets = false;

    o.datasetname = NULL;

    o.generate_perturbed_datasets = false;

    /**/

    opterr = 0;

    while ((c = getopt(argc, argv, "R:e:c:r:Gd:D")) != EOF)
        switch (c)
        {

        case 'd':
            o.datasetname = strdup(optarg);
            break;

        case 'R':
            o.nRuns = atoi(optarg);
            break;

        case 'e':
            o.experimentIds.insert(atoi(optarg));
            break;

        case 'c':
            o.mspknn_func_C = atoi(optarg);
            break;

        case 'r':
            o.mspknn_func_R = atoi(optarg);
            break;

        case 'G':
            o.generate_perturbed_datasets = true;
            break;

        case 'D':
            o.dust_gen_lookup_tables = true;

            break;

        default:
            FATAL("option '" << (char) optopt << "' invalid");
        }


}



void help() {
    exit(0);
}


void cleanUp() {
}

