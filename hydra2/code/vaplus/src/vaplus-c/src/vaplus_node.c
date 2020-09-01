//
//  vaplus_node.c
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//



#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#include "../globals.h"
#include "../include/vaplus_node.h"
#include "../include/vaplus_index.h"
#include <math.h>
#include <float.h>
#include <limits.h>

/*

ts_type mindist_fft_to_approx (struct vaplus_index *index, unsigned int * cand_approx, ts_type *query_fft,
					 unsigned int * query_approx)
{
    ts_type distance = 0;

    unsigned int i = 0;
    
    // mean (DC) value
        
    //if (!normed)
    //{
     // distance = vaplus_fft_min_dist(index, node->vaplus_word[0],query_vaplus[0],query_fft[0],0);
      //distance *= distance;
     // i += 2;
    //}
     
 
    //printf(" prefix_length = %u ", node->prefix_length);
    
    for (; i < index->settings->fft_size; i++) {

      ts_type value = mindist_fft_to_cell (index,cand_approx[i],query_approx[i],query_fft[i],i);
      
      distance += 2*value*value;
      //distance += value*value;
    }
   
    return distance;
}
*/


ts_type mindist_fft_to_approx (struct vaplus_index *index, unsigned int * cand_approx, ts_type *query_fft,
					 unsigned int * query_approx)
{
    ts_type distance = 0;

    unsigned int i;
        
    unsigned int transforms_size = index->settings->fft_size;
    unsigned int timeseries_size = index->settings->timeseries_size;

    for (i = 0; i < transforms_size; ++i) {
          ts_type value = (ts_type) mindist_fft_to_cell (index,cand_approx[i],query_approx[i],query_fft[i],i);
          distance += 2*value*value;
    }
    
     distance = distance / (ts_type) timeseries_size;

    //distance = 0;
    return (distance);
}


ts_type mindist_fft_to_cell (struct vaplus_index * index, unsigned int cand_approx, unsigned int q_approx, ts_type q_fft, unsigned int dim) {


  if (cand_approx > q_approx)
  {
    return  (index->bins[dim][cand_approx] - q_fft);
  }
  
  else if (cand_approx < q_approx)
  {
    return  (q_fft - index->bins[dim][cand_approx+1]);
  }    

  else // (q_approx == cand_approx) 
  {
    return 0;
  }

}

ts_type maxdist_fft_to_approx (struct vaplus_index *index, unsigned int * cand_approx, ts_type *query_fft,
					 unsigned int * query_approx)
{
    double distance = 0;

    unsigned int i;
        
    unsigned int transforms_size = index->settings->fft_size;
    unsigned int timeseries_size = index->settings->timeseries_size;

    for (i = 0; i < transforms_size; ++i) {
        double value = (double) maxdist_fft_to_cell (index,cand_approx[i],query_approx[i],query_fft[i],i);
         distance += 2*value*value;
    }
    
    distance = distance / timeseries_size;
    
    return (ts_type)(distance);
}

ts_type maxdist_fft_to_cell (struct vaplus_index * index, unsigned int cand_approx, unsigned int q_approx, ts_type q_fft, unsigned int dim) {

  if (cand_approx > q_approx)
  {
    return  (index->bins[dim][cand_approx+1] - q_fft);
  }  
  else if (cand_approx < q_approx)
  {
    return  (q_fft - index->bins[dim][cand_approx]);
  }    
  else // (q_approx == cand_approx) 
  {
   /*
    return fmax((index->bins[dim][cand_approx+1] - q_fft),
                 (q_fft - index->bins[dim][cand_approx])  
               );
   */
    return FLT_MAX;
  }

}

