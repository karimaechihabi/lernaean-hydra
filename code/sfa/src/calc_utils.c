//
//  calc_utils.c
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.




#include "../config.h"
#include "../globals.h"
#include "../include/calc_utils.h"
#include "math.h"


ts_type calc_mean (ts_type * series, int start, int end)
{

  ts_type mean = 0;

  if (start >= end)
  {
    int j = 0;
    j++;
    printf("error start > end \n");
  }
  else
  {
   for (int i=start; i < end; i++) 
   {
     mean += series[i];
   }
  
   mean /= (end-start); 
  }

  return mean;
  
}

/*
  Using the stdev computational formula.

*/

ts_type calc_stdev (ts_type * series, int start, int end)
{

  ts_type sum_x_squares=0, sum_x=0, stdev = 0; //sum of x's and sum of x squares
  int i, count_x;

  if (start >= end)
  {
    printf ("error in stdev start >= end\n");
  }
  else
  {
    count_x = end-start; //size of the series
  
    for (int i=start; i<end; i++) 
    {
     sum_x += series[i];
     sum_x_squares += pow(series[i],2);
    }
  
    sum_x_squares -= (pow(sum_x,2) / count_x);

    stdev = sqrt(sum_x_squares/count_x);
  }
  
  return stdev;
  
}

/* 
 This is the compare function used in the binary search code
 */

short compare_ts_type (const void * a, const void * b)
{
  ts_type ts_a = *((ts_type*) a);
  ts_type ts_b = *((ts_type*) b);

  if (ts_a < ts_b )
    return -1;
  else if  (ts_a == ts_b)
    return 0;
  else
    return 1;
}


  

short compare_short (const void * a, const void * b)
{
  if (*(short*)a < *(short*)b )
    return -1;
  else if (*(short*)a == *(short*)b )
    return 0;
  else
    return 1;

}
/*
  returns the current time in string format.
*/

void get_current_time(char * time_buf)
{
    time_t timer;
    
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(time_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

