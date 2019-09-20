#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>

#include "type_definition.hpp"
#include "search.hpp"
#include "nodes.hpp"
#include "heap.hpp"
#include "utils/computeDHWT.hpp"
#include "utils/LruCache/LRUCache.hpp"
#include "utils/signs.hpp"
#include "utils/array_matrix_management.hpp"

using namespace std;

static void usage(char **argv) {
  std::cout << "Usage: " << argv[0] << " <ts_length>  <number of time series> <input_filename> <input_dir (where the input file is)> <output_dir>" << std::endl;
  return;
}

int main (int argc, char** argv){

  if(argc < 6) {
    usage(argv);
    exit(-1);
  }

  int ts_length = atoi(argv[1]);
  int program_size = atoi(argv[1]);
  int program_count_ts = atoi(argv[2]);
  char* input_filename = argv[3];
  char* input_dir = argv[4];
  char* output_dir = argv[5];


  int program_MAX_ENTRIES = 2;
  int offset = 0;
  int prev_offset = 0;
  //CHECK
  //  SeekOrigin seek = SeekOrigin.Begin;
  int program_file_ind = 0;
  int program_prev_ind = -1;

  string program_BASE_DIR = output_dir; // @"/home/sdecri/Documents/Universita/Master_degree/Thesis/src/KNN Search_Hang/KNN search(Heap Update Version)/Stepwise/";
  string program_coeff_file = "coeff_" + (string) input_filename;
  string program_sign_file = "sign_" + (string) input_filename;
  string program_sum_file = "sum_" + (string) input_filename;
  string program_query_file = "query_" + (string) input_filename;

  int program_kKNN = 1;
  int program_NUM_QUERIES = 5;

  double total_coeff_count = 0;
  double total_time_count = 0;
  double read_time_count = 0;
  double calc_time_count = 0;
  double program_thirdterm_time_count = 0;
  double distancer_time_count = 0;
  double prune_time_count = 0;
  unsigned long total_bytes_read = 0;  
  time_t total_io_time = 0;

  int program_levelPow;
  int program_levelPow2;
  int program_levelCT = 0;
  int32_t program_maxlevel;

  sums_type* program_sumQlevels_array = new sums_type[program_NUM_QUERIES]; // every candidate sees this array for every query
  sums_type program_sumQlevels; // watching the value of the current Query from the array 

  int* program_levelsizes;

  signs_type** signs = NULL;
  
  int signs_dim1 = program_count_ts, signs_dim2; 
#if defined(BIT_SIGNS_MATRIX)
  signs_dim2 = program_size / 32;
#else
  signs_dim2 = program_size;
#endif

  sums_type* sums;
  int i;

  double** query;
  double** query2 ;
  sums_type** sum_query;
  bool** sign_query;
  double* tempquery;
  double* tempquery2;
  sums_type* tempsumquery;
  bool* tempsignquery;

  ifstream sk_sr;
  ofstream sw_log;

  struct timespec start, end, start_total, end_total;

  string tmp_filename;
  int cache_size = 32;
  lru::Cache<vector<int>,vector<double> >* cache = new lru::Cache<vector<int>,vector<double> > (cache_size, 0);
 
  if(program_size % 2 != 0){
    cerr << "lenght of time series is not a multiple of 2, this can cause a memory error in case you are using a bitmatrix!";    
  }

  ofstream out ("search.log");
  sw_log.open("log.txt", ios::ate);
#if defined(BIT_SIGNS_MATRIX)
  sw_log << "Using bit matrix" << endl;
#endif

  out << "Creating wavelets...";
  out.flush();
  clock_gettime(CLOCK_MONOTONIC, &start);
  
  computeDHWT(input_filename, input_dir, program_count_ts, program_size, output_dir);
  
  clock_gettime(CLOCK_MONOTONIC, &end);
  out << "done " << (end.tv_sec - start.tv_sec) / 60 << " minutes" << endl;
  

  tmp_filename = program_BASE_DIR + program_sum_file;
  
  //  sk_sr.open(program_sum_file);
  sk_sr.open(&tmp_filename[0], ios::binary);
  
  if(!sk_sr){
    sw_log << "Could not open SUM file: " << tmp_filename <<endl;
    exit(1);
  }
  
  
  program_maxlevel = (int32_t) log2(program_size);

  program_levelsizes =(int*) calloc((program_maxlevel+1), sizeof(int));
  //memset(program_levelsizes, 0, sizeof(int)*(program_maxlevel+1));

  out <<"allocating memory for sums...";
  out.flush();
  clock_gettime(CLOCK_MONOTONIC, &start);
  sums = new sums_type [program_count_ts * 2];

  clock_gettime(CLOCK_MONOTONIC, &end);
  out << "done " << (end.tv_sec - start.tv_sec)/60 << " minutes and " << (end.tv_sec - start.tv_sec)%60 << " seconds" << endl;
  sw_log.flush();


  out << "reading sums...";
  out.flush();
  clock_gettime(CLOCK_MONOTONIC, &start);

  sk_sr.seekg(0, ios::beg);
  sk_sr.read(reinterpret_cast<char*> (sums), program_count_ts*2*sizeof(sums_type)); 


  clock_gettime(CLOCK_MONOTONIC, &end);
  sw_log << "done " << (end.tv_sec - start.tv_sec)/60 << " minutes" << (end.tv_sec - start.tv_sec)%60 << " seconds" << endl;
  sw_log.flush();
 
  sk_sr.close();
  
  //reading signs file

  tmp_filename = program_BASE_DIR + program_sign_file;
  sk_sr.open(&tmp_filename[0]);
  if(!sk_sr){
    sw_log << "Could not open SIGN file" <<endl;
    sw_log.close();
    exit(1);
  }

  out << "allocating memory for signs matrix...";
  out.flush();

  clock_gettime(CLOCK_MONOTONIC, &start); 

  signs =  alloc_1d_or_2d_matrix(signs, N_OF_SIGNS_BLOCK, 
				 (signs_dim1/N_OF_SIGNS_BLOCK)*signs_dim2);
  clock_gettime(CLOCK_MONOTONIC, &end);
  out << "done " << (end.tv_sec - start.tv_sec)/60 << " minutes" << (end.tv_sec - start.tv_sec)%60 << " seconds" << endl;

  out << "reading signs...";
  out.flush();

  clock_gettime(CLOCK_MONOTONIC, &start);
  for(int i=0; i<N_OF_SIGNS_BLOCK; i++){
    read_signs(sk_sr, signs[i], signs_dim1/N_OF_SIGNS_BLOCK, signs_dim2, program_size);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  out << "done " << (end.tv_sec - start.tv_sec)/60 << " minutes" << (end.tv_sec - start.tv_sec)%60 << " seconds" << endl;
  out.flush();

  sk_sr.close();

  tmp_filename = program_BASE_DIR + program_query_file;
  tmp_filename = "../Data/queries.txt.bin";
  sk_sr.open(&tmp_filename[0]);

  if(!sk_sr){
    sw_log << "Could not open QUERY file" <<endl;
    sw_log.close();
    exit(1);
  }
  
  query = new double* [program_NUM_QUERIES];
  query2 = new double* [program_NUM_QUERIES];
  sum_query = new sums_type* [program_NUM_QUERIES];
  sign_query = new bool* [program_NUM_QUERIES];

  for(int j=0; j < program_NUM_QUERIES; j++){
    query[j] = new double [program_size];
    query2[j] = new double [program_size];
    sum_query[j] = new sums_type [program_maxlevel + 1];
    sign_query[j] = new bool [program_size];
  }

  transform_query(query, query2, sum_query, 
		  sign_query, sk_sr, 
		  program_NUM_QUERIES, program_size, 
		  program_sumQlevels_array, program_count_ts);

  out << "Searching.... ";
  out.flush();
  
  tempquery = new double[program_size];
  tempquery2 = new double[program_size];
  tempsumquery = new sums_type[program_maxlevel + 1];
  tempsignquery = new bool[program_size];

  clock_gettime(CLOCK_MONOTONIC, &start_total);
  for (int i1 = 0; i1 < program_NUM_QUERIES; i1++) {
    out << "------------------------------\n";
    copy(query[i1], query[i1]+program_size, tempquery);
    copy(query2[i1], query2[i1]+program_size, tempquery2);
    copy(sign_query[i1], sign_query[i1]+program_size, tempsignquery);
    copy(sum_query[i1], sum_query[i1]+program_maxlevel+1, tempsumquery);


    program_sumQlevels = program_sumQlevels_array[i1];  //i1 - refers to the query

    clock_gettime(CLOCK_MONOTONIC, &start);
    
    findKNN(signs, signs_dim1, signs_dim2, sums, tempquery, tempquery2, tempsignquery, 
	    tempsumquery, offset, prev_offset, total_coeff_count,
	    program_MAX_ENTRIES, program_size, program_kKNN,
	    program_count_ts, program_levelCT, program_levelPow,
	    program_levelPow2, program_maxlevel, program_file_ind,
	    program_levelsizes, program_prev_ind, program_BASE_DIR,
	    program_coeff_file, program_sumQlevels,
	    cache, cache_size, sw_log, out, total_bytes_read, total_io_time);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    out << "done " << (end.tv_sec - start.tv_sec)/60 << " minutes " << (end.tv_sec - start.tv_sec)%60 << " seconds\n";
    out << "\\------------------------------\n";
    
  }



  for (int i1 = 0; i1 < program_maxlevel; i1++){
    program_levelsizes[i1] /= program_NUM_QUERIES;
    sw_log << " Level: " << i1 << " Count: " << program_levelsizes[i1] << endl;
  }
  
  sw_log << endl;
  
  clock_gettime(CLOCK_MONOTONIC, &end_total);
  out << "total search time: " << (end_total.tv_sec - start_total.tv_sec) / 60 << " minutes and " <<((end_total.tv_sec - start_total.tv_sec)%60) << " seconds" << endl;

  out << "total io_time: " << total_io_time / 60 << " minutes and " << total_io_time % 60 << " seconds" << endl;
  
  out << "total bytes read: " << total_bytes_read << endl;
  
  
  sw_log.close();
  out.close();

  delete [] tempquery;
  delete [] tempquery2;
  delete [] tempsumquery;
  delete [] tempsignquery;


  for(int j=0; j < program_NUM_QUERIES; j++){
    delete [] query[j];
    delete [] query2[j];
    delete [] sum_query[j];
    delete [] sign_query[j];
  }

  delete [] query;
  delete [] query2;
  delete [] sum_query;
  delete [] sign_query;
    
  
  delete [] sums;
  free_1d_or_2d_matrix(signs, N_OF_SIGNS_BLOCK, signs_dim2);

  delete cache;  
  
  free(program_levelsizes);
  delete [] program_sumQlevels_array;
    
  return 0;
}
