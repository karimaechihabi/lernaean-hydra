#include <iostream>
#include <cstdlib>
#include <cmath>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>


#include "heap.hpp"
#include "nodes.hpp"
#include "search.hpp"
#include "utils/utils.hpp"
#include "utils/signs.hpp"
#include "utils/LruCache/LRUCache.hpp"

using namespace std;

void transform_query(double** query, double** query2, sums_type** sumquery, 
		     bool** sign_query, ifstream& sk_sr, 
		     int program_NUM_QUERIES, int program_size, 
		     sums_type* sumQlevels_array, int count_ts){


  int t_size; // temporary variable holding size 
  int c_counter;
  int v_counter;
  int p_counter;
  int endj;
  int32_t m_level;
  double one_coeff;
  double pow2;
  double temp1, temp2;
  int i, counter, pos;

  double* coeff = new double[program_size];
  double* value = new double[program_size - 1];
  
  int data_len = program_size; //CHECK length of data
  double* data = new double [data_len];  
  float * data_ = new float [data_len];

  bool isBinary = true;


  for (int pq = 0; pq < program_NUM_QUERIES; pq++) {
    
    //double tempx; int tempi = 0;
    //CHECK
    // for(int i_=0; i_<data_len; i_++){
    //   sk_sr >> data[i_];
    // }
    if(isBinary){
      read_from_binary_file(sk_sr, data_, data_len);
      copy(data_, data_ + data_len, data);
    }
    else
      read_from_txt_file(sk_sr, data, data_len);

    //    z_normalize(data, data_len);

    //    double* data = Util.StringToArray(sk_line, ' ');
    //CHECK k
    for (int j = program_size - 1, k = 0; j > 0; j -= 2){
      temp1 = data[j]; temp2 = data[j - 1];
      value[k] = (temp1 + temp2) / 2;
      coeff[k] = (temp2 - temp1) / 2;
      k++;
    }
    
    t_size = program_size / 2; // temporary variable holding size 
    c_counter = program_size / 2;
    v_counter = program_size / 2;
    p_counter = 0;

    while (t_size > 1){
      endj = p_counter + t_size;
      for (int j = 0; j < t_size; j += 2){
	coeff[c_counter + j / 2] = (value[p_counter + j + 1] - 
				    value[p_counter + j]) / 2;
	value[v_counter + j / 2] = (value[p_counter + j] + 
				    value[p_counter + j + 1]) / 2;
      }
      p_counter = p_counter + t_size;
      
      t_size = t_size / 2;
      c_counter = c_counter + t_size;
      v_counter = v_counter + t_size;
    }

    coeff[program_size - 1] = value[program_size - 2];

    //ORIGINALLI Convert.Math.Log(size,2))
    m_level = (int32_t) log2(program_size);

    for (int j = program_size - 1, k1 = 0; j >= 0; j--){
      one_coeff = coeff[j];
      query[pq][k1] = one_coeff;
      query2[pq][k1] = one_coeff * one_coeff;
      
      if (one_coeff >= 0)
	sign_query[pq][k1] = true;
      else
	sign_query[pq][k1] = false;
      
      k1++;
    }

    one_coeff = query2[pq][0];
    pow2 = program_size;
    sumquery[pq][0] = one_coeff; //Math.Pow(coeff[size - 1], 2);
    sumquery[pq][0] += query2[pq][1];
    sumQlevels_array[pq] = sumquery[pq][0] * pow2;

    //cerr << "sumQlevels_array: " << sumQlevels_array[pq] << " " << "sumquery[" << 0 << "]: " << sumquery[pq][0] << " sq*pow2: " << sumquery[pq][0]*pow2 << endl;    

    pow2 /= 2;

    i = 0; counter = 2; pos = 2;

    for (i = 1; i < m_level; i++){
      sumquery[pq][i] = 0;
      int j;
      for (j = 0; j < counter; j++){
	sumquery[pq][i] += query2[pq][pos];
	pos++;
      }
      sumQlevels_array[pq] += sumquery[pq][i] * pow2;
      //cerr << "sumQlevels_array: " << sumQlevels_array[pq] << " " << "sumquery[" << i << "]: " << sumquery[pq][i] << " sq*pow2: " << sumquery[pq][i]*pow2 << endl;
      counter *= 2;
      pow2 /= 2;
    }
  }

  delete [] data_;
  delete [] data;
  delete [] value;
  delete [] coeff;
}



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
	     lru::Cache<vector<int>,vector<double> >* cache, int cache_size,
	     ofstream& sw_log, ofstream& out, unsigned long & total_bytes_read, time_t & total_io_time){

  unsigned long bytes_read = 0;
  time_t io_time = 0;
  int size_temp = program_count_ts; //size_temp stores the number of current candidates
  int size_temp_prev = size_temp;
  int level_temp = 0;
  int pow_level_temp = 2; //will be multiplied by 2 with increase at every level - to compensate for pow(2,level_temp-1) in bytecount
  double coeff_count = 0; // coeff_count : to count the total number of coefficients accessed
  int byteCount = -1; // byteCount : to count a specific number of bytes to read from the file of coefficients
  string inp_string; // inp_string : to store the string read from the coefficients files

  int temp_col;
  //CHECK
  string filename;
  string filewithoutext;
  string level_filename;
  candidateNode* cnTemp;
  candidateList* cList;
  MaxHeap* theHeap;
  double* dataread;
  std::vector<double> v_dataread (program_size);
  int line2read;
  double    temp_t_d, temp_t_ub, temp_t_lb;
  int temp_t_li;
  int n_level_coefficients = 0;

  ifstream br;
  if(!out){
    cerr << "Could not open search.log" << endl;
    exit(EXIT_FAILURE);
  }
  double* barray;

  int* heap_pos = (int*) malloc(program_count_ts * sizeof(int));
  memset(heap_pos, -1, program_count_ts * sizeof(int));

  std::stringstream concatenate;

  program_levelCT = 0;
  program_levelPow = program_size;
  program_levelPow2 = program_size * program_size;

  coeff_count = program_count_ts * 2; //(maxlevel + 1) * count_ts; //from sums //Update: 2 as sums has two columns
  coeff_count += (program_count_ts * program_size)/32;       //from signs

  theHeap = new MaxHeap [1];
  init_maxheap(theHeap, program_kKNN);
  
  cList = new candidateList [1];
  cList->head = NULL;
  cList->last = NULL;
  cList->totalcNode = 0;
 
  sw_log << "inserting candidates in list...";
  cout.flush();
  
  for (int i = 0; i < program_count_ts; i++)
    insert(cList, 0, 0, 0, i, program_size, sums, signs[i/(signs_dim1/N_OF_SIGNS_BLOCK)], (signs_dim1/N_OF_SIGNS_BLOCK), signs_dim2);
  
  sw_log << "done"<<endl;
  cout.flush();
  
  out << "Initial Number of candidates: " << cList->totalcNode << "\n";
  out.flush();
  struct timespec end_io, start_io; 
 
  while (size_temp > program_kKNN && level_temp < program_maxlevel){
    //while (level_temp < program_maxlevel){
       
    out << "Level: " << level_temp << "\n";
    if(level_temp == 0 || level_temp == 1)
      n_level_coefficients = 2;
    else 
      n_level_coefficients *= 2;

    program_levelsizes[level_temp] += size_temp;
    
    program_file_ind = 0; program_prev_ind = -1; offset = 0; prev_offset = -1;
    concatenate.str(std::string());
    concatenate << program_BASE_DIR << "level_" << level_temp << "_" << program_coeff_file;
    filename = concatenate.str();//std::string("level_") + level_temp + "_" + coeff_file;
    filewithoutext = filename.erase(filename.length() - 4, 4);
    //concatenate.str("");
    //concatenate.clear();
    //concatenate.str(std::string());
    //concatenate << filewithoutext << ".txt";
    level_filename = concatenate.str();//filewithoutext + "_" + program_file_ind + ".txt";
    br.open (level_filename.c_str(),ios::binary);
    
    if(!br){
      cerr <<"Could not open coefficient file at level " << level_temp << ":" << level_filename.c_str() << endl;
      exit(EXIT_FAILURE);
    }                       
    
    if (level_temp == 0||level_temp==1)
      temp_col = 2;
    else 
      temp_col = pow_level_temp;

    //double[,] distances = new double[size_temp, 3];
    
    // for(int i_=0; i_<size_temp; i_++){
    //   //CHECK
    //   //dataread[i_] = new double [inp_string.length()];
    //   dataread[i_] = new double [n_level_coefficients];
    // }
    
    program_sumQlevels -= program_levelPow * sumquery[level_temp];
    
    // cnTemp = cList->head;
    
    // for (int i = 0; i < size_temp; i++)
    //   {

    // 	v_dataread = fetch_level(br, cache, cache_size, n_level_coefficients, cnTemp->t_li, level_temp);

    // 	dataread=v_dataread[i].data();
    // 	cnTemp = cnTemp->next;
    //   }


    // br.close();

    cnTemp = cList->head;
    // cerr << "computing distance"<< endl;
    // cerr.flush();

    bool use_cache=false;
    for (int i = 0; i < size_temp; i++){
      if (use_cache) {
        v_dataread = fetch_level(br, cache,
                                 cache_size, n_level_coefficients,
                                 cnTemp->t_li, level_temp);
	
        dataread = v_dataread.data();
      }
      else{
	clock_gettime(CLOCK_MONOTONIC, &start_io);
        dataread = new double [n_level_coefficients];
        fetch_level_(br, cache,
                     cache_size, n_level_coefficients,
                     cnTemp->t_li, level_temp, dataread);
	clock_gettime(CLOCK_MONOTONIC, &end_io);
	io_time += end_io.tv_sec - start_io.tv_sec;
      }
      bytes_read += n_level_coefficients * sizeof(float);
      
      // for (int i = 0; i < size_temp; i++){
      
      // v_dataread = fetch_level(br, cache, 
      // 			       cache_size, n_level_coefficients, 
      // 			       cnTemp->t_li, level_temp);
      
      // dataread = v_dataread.data();     
      //cerr << "d:"<< i << " ";
      distance_all(cnTemp, dataread, temp_col, 
		   level_temp, query, query2, sumquery, 
		   signquery, program_levelCT, program_levelPow, 
		   program_levelPow2, program_size, program_sumQlevels);


      temp_t_d = cnTemp->t_d;      
      temp_t_ub = cnTemp->t_ub;
      temp_t_lb = cnTemp->t_lb;
      temp_t_li = cnTemp->t_li;
      
      if (temp_t_lb < 0)
	temp_t_lb = 0;
       //      displayHeap(theHeap);
       if (temp_t_d < 0)
	 cnTemp->t_d = 0;
      
      if (i < program_kKNN && level_temp==0)
	insert(theHeap, heap_pos, temp_t_ub, temp_t_lb,temp_t_li);//First insert k candidates into the heap, k=maxHeapSize
      else                
	update(theHeap, heap_pos, temp_t_ub, temp_t_lb,temp_t_li);//Check the remaining candidates bound values and update the heap(following the heap property)
      
      //displayHeap(theHeap);
      //cout << "--------------------after--------------------------"<<endl;
      cnTemp = cnTemp->next;
                   
    }
    br.close();
    // cerr << "\\computing distance"<< endl;
    // cerr.flush(); 
    //displayHeap(theHeap);
    coeff_count += size_temp * temp_col; //incrementing the coefficient count with the data accessed
   
    size_temp_prev = size_temp;
                
    //if (level_temp == program_maxlevel - 2)
    quicksort_prune2(cList, theHeap, size_temp, program_kKNN);
    

    out << "Current number of candidates: " << cList->totalcNode << "\n";  

    level_temp++;
    program_levelPow /= 2;
    program_levelPow2 /= 4;
    if (program_levelCT == 0){ 
      program_levelCT = 2;     
    }
    else{
      program_levelCT *= 2;
      pow_level_temp *= 2;
    }

    // for(int i_=0; i_<size_temp_prev; i_++){
    //   //CHECK
    //   delete [] dataread[i_];
    // }
   
  }

  // if(level_temp==program_maxlevel-2){
  //   cout << " I'M CALLING PRUNE FUNCTION"<<endl;
  //   quicksort_prune2(cList, theHeap, size_temp,program_kKNN);
  //  }

  candidateNode* tmp = NULL;
  tmp = cList->head;

  out << "Id of remaining nodes: " << "\n"; 
  while(tmp != NULL) {
    out << tmp->t_li << " " << sqrt(tmp->t_d) << "\n";    
    tmp = tmp->next;
  }
  out << "\n";
  total_bytes_read += bytes_read;
  total_io_time += io_time;
  out << "i/o fetch time: " << io_time << "\n";
  out << "bytes read: " << bytes_read << "\n";
  //CHECK time         
  // DateTime endTime = DateTime.Now;
  // TimeSpan duration = endTime - startTime;
  // total_time_count += (double)duration.TotalMilliseconds;
  // Console.WriteLine("Total execution duration is {0}", duration);
  
  //Console.WriteLine("No. of coefficients used : {0}", coeff_count);
  
  //CHECK
  total_coeff_count += coeff_count;


  tmp = NULL;
  while(cList->head != NULL) {
    tmp = cList->head;
    cList->head = cList->head->next;
    delete [] tmp->sums;
    //delete [] tmp->signs;
    delete [] tmp;
  }
  delete [] cList;

  free(heap_pos);

  for(int j=0; j < theHeap->k; j++){
    free(theHeap->boundArray[j]);
  }
  free(theHeap->boundArray);
  delete [] theHeap;

}

void quicksort_prune2(candidateList* cList, MaxHeap* theHeap, int & size_temp,int k){

  candidateNode* cnTemp = cList->head;
  candidateNode* cn;
  candidateNode* tmp;

  double curmaxupper = getUBvalue(theHeap->boundArray[0]);
  //delete from head
  while (cList->head->t_lb > curmaxupper){
    tmp = cList->head;
    cList->head = cList->head->next;
    cList->totalcNode--;
    
    delete [] tmp->sums;
    //delete [] tmp->signs;
    delete [] tmp;
  }

  //delete from elsewhere

  cn = cList->head;
  cnTemp = cn->next;

  while (cnTemp != NULL){

    if (cnTemp->t_lb > curmaxupper){
      tmp = cn->next;
      cn->next = cnTemp->next; //update the previous node
      cList->totalcNode--;
      cnTemp = cn->next; //update the current node and thus delete it
      
      delete [] tmp->sums;
      //delete [] tmp->signs;
      delete [] tmp;
    }
    else {

      cn = cnTemp;
      cnTemp = cn->next;
    }

  }
  size_temp = cList->totalcNode;
  //cout << "Current size of the heap: " << size_temp << endl;
  //cout << "After pruning, the curren maximum upperbound stored in the heap is: " << curmaxupper << endl;
  //cout << "Current number of remaining node is: " << size_temp <<endl;
}
