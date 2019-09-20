#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <fstream>
#include <vector>

#include "type_definition.hpp"
#include "nodes.hpp"
#include "heap.hpp"
#include "utils/LruCache/LRUCache.hpp"

using namespace std;

void transform_query(double** query, double** query2, sums_type** sumquery, 
		     bool** sign_query, ifstream& sk_sr, 
		     int program_NUM_QUERIES, int program_size, 
		     sums_type* sumQlevels_array, int count_ts);

void findKNN(signs_type** signs, int signs_dim1, int signs_dim2, sums_type* sums, double* query, double* query2,
	     bool* signquery, sums_type* sumquery, int offset, int prev_offset,
	     double total_coeff_count, int & program_MAX_ENTRIES,
	     int & program_size, 
	     int & program_kKNN, int & program_count_ts,
	     int & program_levelCT, int & program_levelPow, 
	     int & program_levelPow2,int & program_maxlevel, 
	     int & program_file_ind, int* & program_levelsizes, 
	     int & program_prev_ind, string program_BASE_DIR,
	     string program_coeff_file, sums_type & program_sumQlevels,
	     lru::Cache<vector<int>, vector<double> >* cache, int cache_size,
	     ofstream& sw_log, ofstream& out, unsigned long & total_bytes_read, time_t & total_io_time);

//void findKNN(bool** signs, double** sums, double* query, double* query2, bool* signquery, double* sumquery);

void quicksort_prune2(candidateList* cList, MaxHeap* theHeap, int & size_temp,int k);
//void quicksort_prune2(candidateList* cList, MaxHeap* theHeap, int & size_temp,int k);

#endif /*SEARCH_HPP*/
