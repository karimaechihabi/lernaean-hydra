//
//  calc_utils.c
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//



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

ts_type calc_mean_per_segment (ts_type * series, short * segments, ts_type *means, int size)
{

  int start=0, end;
  for (int i=0; i< size; i++) 
  {
    end = segments[i];
    means[i] = calc_mean (series, start, end);
    start = end;
  }
  
}

ts_type calc_stdev_per_segment (ts_type * series, short * segments, ts_type *stdevs, int size)
{

  int start=0, end;
  for (int i=0; i< size; i++) 
  {
    end = segments[i];
    stdevs[i] = calc_stdev (series, start, end);
    start = end;
  }
  
}



/* 
 This is the compare function used in the binary search code
 */

  

ts_type compare_ts_type (const void * a, const void * b)
{
  return ( *(ts_type*)a - *(ts_type*)b );
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
short compare_file_map_entries (const void * a, const void * b)
{
  char * entry_a = (char *) a;
  struct dstree_file_map *entry_b = (struct dstree_file_map*) b;

  return ( strcmp(entry_a, entry_b->filename));

}
*/
short compare_file_buffer (const void * a, const void * b)
{
  struct dstree_file_buffer * entry_a = *((struct dstree_file_buffer**) a);
  struct dstree_file_buffer * entry_b = *((struct dstree_file_buffer**) b);

  if (entry_a->buffered_list_size < entry_b->buffered_list_size )
    return 1;
  else if  (entry_a->buffered_list_size == entry_b->buffered_list_size )
    return 0;
  else
    return -1;
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

ts_type calculate_mean_std_dev_range(struct segment_sketch sketch, int len){

  ts_type mean_width = sketch.indicators[0]-sketch.indicators[1];

  ts_type stdev_upper = sketch.indicators[2];
  ts_type stdev_lower = sketch.indicators[3];

  return (len * (mean_width * mean_width + stdev_upper * stdev_upper));
  
}


int get_segment_start(short * points, int idx)
{
  if (idx == 0 )
    return 0;
  else
    return points[idx-1];
}

int get_segment_end(short * points, int idx)
{
  return points[idx];
}

int get_segment_length(short * points, int i)
{

  if (i == 0)
    return points[i];
  else
    return points[i] - points[i-1];

}


