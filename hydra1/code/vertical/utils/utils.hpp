#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include "LruCache/LRUCache.hpp"

void z_normalize(double* ts, int size);
int dirExists(const char* path);

using namespace std;

template<typename T>
void read_from_binary_file (ifstream& infile, T* data, int size){

  infile.read(reinterpret_cast<char*> (data), size * sizeof(T));

}

template<typename T>
void read_from_txt_file (ifstream& infile, T* data, int size){

  int index = 0;
  T x;
  
  while(infile >> x){
  
    data[index] = x;         
    index++;    
    
    if(index == size)
      break;
  }

}

template <typename T>
void read_from_file (ifstream& infile, bool isBinary, T* data, int size){

  if (isBinary)
    read_from_binary_file(infile, data, size);
  else 
    read_from_txt_file(infile, data, size);
}

template<typename T>
vector<T> fetch_level(ifstream& infile, lru::Cache<std::vector<int>,std::vector<T> >* cache, int cache_size, int data_size, int index, int level){
  
  std::vector<int> x (2);
  T* data;

  x[0] = index;
  x[1] = level;
  

  if (cache->contains(x)){
    std::vector<T> tmp;
    tmp = cache->get(x);
    //data = &tmp[0];
    return tmp;
  }

  else{
    data = new T [data_size];
    infile.seekg((streamoff)index*data_size*sizeof(T), infile.beg);
    infile.read(reinterpret_cast<char*> (data), data_size * (sizeof(T)));
    if(infile.bad()){
      cerr << "Error reading input file"<<endl;
      exit(EXIT_FAILURE);
    }
    
    std::vector<T> tmp1 (data, data + data_size);

    if(level == 0){
      if(index < cache_size)  
	cache->insert(x, tmp1);
    }
    else
      cache->insert(x, tmp1);

    delete [] data; 
    return tmp1;
  }
    
}

template<typename T>
void fetch_level_(ifstream& infile, lru::Cache<std::vector<int>,std::vector<T> >* cache, int cache_size, int data_size, int index, int level, T*\
 data){


  infile.seekg((streamoff)index*data_size*sizeof(T), infile.beg);
  infile.read(reinterpret_cast<char*> (data), data_size * (sizeof(T)));

  if(infile.bad()){
    cerr << "Error reading input file"<<endl;
    exit(EXIT_FAILURE);
  }
}


#endif
