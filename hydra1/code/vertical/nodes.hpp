#ifndef NODE_HPP
#define NODE_HPP

#include "type_definition.hpp"
#include "utils/signs.hpp"

typedef struct candidatenode{

  double t_ub;
  double t_lb;
  double t_d;
  int t_li;

  sums_type* sums;
  signs_type* signs;

  sums_type qsum_odd;
  sums_type qsum_even;
  struct candidatenode* next;

} candidateNode;

typedef struct candidatelist{

  candidateNode* head;
  candidateNode* last;
  int totalcNode;

} candidateList;

//void init_candidateNode (candidateNode* n, int size);
void init_candidateNode (candidateNode* n, int size, sums_type* sums, signs_type* signs, int signs_dim1_block, int signs_dim2, int index);
void distance_all(candidateNode* n, double* data, int col,
		  int level, double* query, double* query2, 
		  sums_type* sumquery, bool* signquery, 
		  int program_levelCT, int program_levelPow, 
		  int program_levelPow2, int program_size,
		  sums_type program_sumQlevels);

  //void distance_all (candidateNode* n, int index, double** data, int col, int level, double* query, double* query2, double* sumquery, bool* signquery);

void init_candidateList (candidateList* l);
void insert (candidateList* list, double u, double l, double d, int ind, 
	     int size, sums_type* sums, signs_type* signs, 
	     int signs_dim1_block, int signs_dim2);

#endif /* NODE_HPP */
