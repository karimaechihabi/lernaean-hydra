#ifndef SIGNS_HPP
#define SIGNS_HPP

#include <fstream>
#include <string>

#include "bitArray.hpp"


#if defined(BIT_SIGNS_MATRIX)
typedef int signs_type;
#define N_OF_BITS_IN_INT (sizeof(int)*8)
#define N_OF_SIGNS_BLOCK 1
#else
typedef bool signs_type;
#endif

template<typename T> 
inline void set_sign (T s, int i, int j, int dim2, bool b);

template<>
inline void set_sign<bool**>(bool** s, int i, int j, int dim2, bool b){
  s[i][j] = b;
}

template<>
inline void set_sign<bool*>(bool* s, int i, int j, int dim2, bool b){
  s[i*dim2 + j] = b;
}

template<>
inline void set_sign<int**>(int** s, int i, int j, int dim2, bool b){
  if(b)
    SetBit(s[i], j);
  else 
    ClearBit(s[i], j);
}

template<>
inline void set_sign<int*>(int* s, int i, int j, int dim2, bool b){

  if(b)
    SetBit(&s[i*dim2], j);
  else 
    ClearBit(&s[i*dim2], j);
}

template<typename T>
void read_signs (std::ifstream& in, T s, int dim1, int dim2, int length){
  
 int i=0;
 while (i < dim1){
   
   std::string sk_line;
   getline(in, sk_line);
   for (int j=0; j < length; j++){
     char cx = sk_line[j];
     if(cx == '1')
       set_sign(s, i, j, dim2, true);
     else 
       set_sign(s, i, j, dim2, false);
   }
   i++;
 }
}

template<>
inline void read_signs<int*>(std::ifstream& in, int* signs, 
			     int dim1, int dim2, int length) {

  in.read(reinterpret_cast<char*> (signs), dim1*dim2*sizeof(signs_type));
}

template<>
inline void read_signs<int**>(std::ifstream& in, int** signs, 
			     int dim1, int dim2, int length) {

  in.read(reinterpret_cast<char*> (signs), dim1*dim2*sizeof(signs_type));
}

template<typename T>
inline void write_signs (std::ofstream& out, T buffer, int dim);

template<>
inline void write_signs<bool*>(std::ofstream& out, bool* buffer, int dim){
  for (int i=0; i<dim; i++){
    out << buffer[i];
  }
}

template<>
inline void write_signs (std::ofstream& out, int* buffer, int dim){
   out.write (reinterpret_cast<char*> (buffer), dim * sizeof(signs_type));
}

#endif /*SIGNS_HPP*/
