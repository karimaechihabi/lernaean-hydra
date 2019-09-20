#ifndef ARRAY_MATRIX_MANAGEMENT_HPP
#define ARRAY_MATRIX_MANAGEMENT_HPP

template<typename T>
inline T** alloc_1d_or_2d_matrix (T** a, int dim1, int dim2){

  a = (T**) malloc(sizeof(T*) * dim1);
  int i;
  for (i=0; i<dim1; i++){
    a[i]= (T*) malloc(sizeof(T) * dim2);
  }

  return a;
}

template<typename T>
inline T* alloc_1d_or_2d_matrix (T* a, int dim1, int dim2){

  a = (T*) malloc(sizeof(T) * dim1 * dim2);
  return a;
}

template<typename T>
void free_1d_or_2d_matrix(T** a, int dim1, int dim2){

  int i;
  for (i=0; i<dim1; i++){
    free(a[i]);
  }
  free(a);
}

template<typename T>
inline void free_1d_or_2d_matrix(T* a, int dim1, int dim2){
  free(a);
}
#endif
