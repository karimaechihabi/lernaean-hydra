#include <iostream>
#include <cstdlib>
#include <cmath>

#include "nodes.hpp"
#include "./utils/signs.hpp"

using namespace std;

void init_candidateNode (candidateNode* n, int size, sums_type* sums, signs_type* signs, int signs_dim1_block, int signs_dim2, int index){

  n->sums = new sums_type [2];
  copy(sums+(index*2), sums+(index*2)+2, n->sums);
  //n->sums[0] = sums[index][0];
  //n->sums[1] = sums[index][1];
  n->signs = signs+((index%signs_dim1_block)*signs_dim2);
  n->next = NULL;

  n->t_ub = 0;
  n->t_lb = 0; 
  n->t_d = 0;
  n->qsum_odd = 0;
  n->qsum_even = 0;

}

void distance_all(candidateNode* n, double* data, int col,
		  int level, double* query, double* query2, 
		  sums_type* sumquery, bool* signquery, 
		  int program_levelCT, int program_levelPow,
		  int program_levelPow2, int program_size, 
		  sums_type program_sumQlevels){

  // first calculating actual distance as p^2 + q^2 - 2*p*q
  double distance = 0, temp = 0, product_temp = 0;
  sums_type sub_sum = 0; //holds the sum of coefficients of the candidate of a particular level
  
  double temp1 = 0; // holds square of coefficients in the loop below
  
  double commonBound, thirdterm;

  int counter, pos, endcounter, temp4;
  double temp_qsum_odd, temp_qsum_even;
  double thirdterm_u;
  double thirdterm_l;
  bool* signL;

  // cerr << "................................" << endl;

  signL = new bool[program_size];

  for (int i = 0; i < program_size; i++){
#if defined(BIT_SIGNS_MATRIX)
    signL[i] = (( (n->signs[i/32] & (1 << (i%32) )) != 0 ) == signquery[i]);
#else 
    signL[i] = (n->signs[i] == signquery[i]);
#endif
  }
  for (int i=0, j = (int) program_levelCT; i<col; i++){
    temp = data[i];
    // cerr << " The number read in was: " << temp << endl;
    temp1 = temp * temp;

    distance += temp1;
    if(signL[j])
      product_temp += temp * abs(query[j]);
    else 
      product_temp -= temp * abs(query[j]);
    j++;
  }
  //cerr << ".................................." << endl;
  sub_sum = distance;
  
  distance += sumquery[level] -2 * product_temp;
  //cout << "\nprogram_levelPow: " << program_levelPow << endl;
  n->t_d += program_levelPow * distance; //t_d holds the prev_distace value, so updation required

  //cout <<"n->sums[1]: " <<n->sums[1]<<endl;
  //cout <<"n->sums[0]: " <<n->sums[0]<<endl;


  n->sums[1] -= sub_sum;
  sub_sum *= (program_levelPow);
  n->sums[0] -= sub_sum;

  //cout <<"n->sums[1]: " <<n->sums[1]<<endl;
  //cout <<"n->sums[0]: " <<n->sums[0]<<endl;

  //  DateTime ttst = DateTime.Now;
  commonBound = n->t_d;
  //cout << "\ncommonBound: " << commonBound << endl;
  thirdterm = n->sums[1];
  //cout << "\nthirdterm: " << thirdterm << endl;

  commonBound += n->sums[0]; //adding the pow(2,l)pow(coeff,2)
  commonBound += program_sumQlevels; //adding the query2 terms

  //qsum_odd = 0; qsum_even = 0;//remove when comment removed from level 1
  if (level == 0){
    
    counter = 2; 
    pos = counter;

    temp4 = program_levelPow2 / 4;

    temp_qsum_odd = 0; temp_qsum_even = 0;
                
    while (counter < program_size){
      
      endcounter = pos + counter;
      temp_qsum_odd = 0; temp_qsum_even = 0;
      
      for (int j = (int)pos; j < endcounter; j++){
	if (signL[j] == false)
	  temp_qsum_odd += query2[j]; // do it with 1D array OR scrap 1D arrays and use original query2
	else
	  temp_qsum_even += query2[j];
      }

      n->qsum_odd += temp_qsum_odd * temp4;
      n->qsum_even += temp_qsum_even * temp4;

      temp4 = temp4 / 4;

      pos = pos + counter;
      counter = counter * 2;
                    
    }
  }
  else {
    counter = program_levelCT;
    pos = counter;
    temp4 = program_levelPow2;
    
    temp_qsum_odd = 0; temp_qsum_even = 0;

    endcounter = counter *2;

    while (pos < endcounter){
	
      if (signL[pos] == false)
	temp_qsum_odd += query2[pos];
      else
	temp_qsum_even += query2[pos];
      pos++;
    }
      
    n->qsum_odd -= temp_qsum_odd * temp4;
    n->qsum_even -= temp_qsum_even * temp4;
  }
  /* Console.WriteLine("The temp_qsum_odd is {0}:", temp_qsum_odd);
     Console.WriteLine("The temp_qsum_even is {0}:", temp_qsum_even);*/
  //cout << "Current level is: " << level << endl;
           
  //if (level == 3)
  //Console.WriteLine();

  if(thirdterm < 0)
    thirdterm = 0;

  thirdterm = 2 * (double) sqrt(thirdterm);

  if(n->qsum_odd < 0)
    n->qsum_odd = 0;

  thirdterm_u = thirdterm * (double)sqrt(n->qsum_odd);
  n->t_ub = commonBound + thirdterm_u;

  if(n->qsum_even < 0)
    n->qsum_even = 0;

  thirdterm_l = thirdterm * (double)sqrt(n->qsum_even);
  n->t_lb = commonBound - thirdterm_l;

  // DateTime ttet = DateTime.Now;
  // TimeSpan ts = ttet - ttst;
  // Program.thirdterm_time_count += (double)ts.TotalMilliseconds;
  /* Console.WriteLine("The lower bound is:{0}", t_lb);
     Console.WriteLine("The upper bound is:{0}", t_ub);*/
  delete [] signL;
}

void init_candidateList (candidateList* l){

  l->head = NULL;
  l->last = NULL;
  l->totalcNode = 0;
}

void insert (candidateList* list, double u, double l, double d, int ind, 
	     int size, sums_type* sums, signs_type* signs, 
	     int signs_dim1_block, int signs_dim2){
  
  if (list->head == NULL){

    list->head = new candidateNode [1];
    list->head->sums = NULL;
    list->head->signs = NULL;
    init_candidateNode(list->head, size, sums, signs, 
		       signs_dim1_block, signs_dim2, ind);
    
    list->head->t_ub = u;
    list->head->t_lb = l;
    list->head->t_d = d;
    list->head->t_li = ind;
    list->head->qsum_even = 0;
    list->head->qsum_odd = 0;

    // for (int i = 0; i < 2; i++){
    //   list->head->sums[i] = sums[ind][i];
    //   //cout << "sums[" << ind << "][" << i <<"]:" << sums[ind][i] << endl;
    // }
    // for (int i = 0; i < size; i++)
    //   list->head->signs[i] = signs[ind][i];
    list->last = list->head;
  }
  else {
    candidateNode* temp = new candidateNode [1];
    init_candidateNode(temp, size, sums, signs, 
		       signs_dim1_block, signs_dim2, ind);
    temp->t_ub = u;
    temp->t_lb = l;
    temp->t_d = d;
    temp->t_li = ind;
    
    temp->qsum_even = 0;
    temp->qsum_odd = 0;

    // for (int i = 0; i < 2; i++)
    //   temp->sums[i] = sums[ind][i];
    // for (int i = 0; i < size; i++)
    //   temp->signs[i] = signs[ind][i];

    list->last->next = temp;
    list->last = temp;
  }
  list->totalcNode++;         
}
