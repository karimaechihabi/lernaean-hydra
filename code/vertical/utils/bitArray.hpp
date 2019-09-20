#ifndef BITARRAY_HPP
#define BITARRAY_HPP

#define n_bits_in_int sizeof(int)*8

void  SetBit( int* A,  int k );
void  ClearBit( int* A,  int k );

inline int TestBit( int* A,  int k )
{
  return ( (A[k/32] & (1 << (k%32) )) != 0 ) ;     
}



#endif



