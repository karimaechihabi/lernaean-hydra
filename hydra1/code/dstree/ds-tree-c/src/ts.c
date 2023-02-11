//
//  ts.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "../config.h"
#include "../globals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../include/ts.h"

/**
 This function converts a string of floats seperated by a delimeter into a ts 
 record of a size ts_size.
 @param char ts_str[]
 @param int ts_size
 @param const char * delims
 @return *ts
 */
enum response ts_parse_str(char ts_str[], ts_type * ts_out, int ts_size, const char * delims)
{
    int index=0;
    char *result = strtok( ts_str, delims );
	while( result != NULL ) {
   	        ts_out[index] =  atof(result);
		result = strtok( NULL, delims );
#ifdef SANITY_CHECK
        if (index >= ts_size)
        {
            fprintf(stderr, "Error in ts.c: Time series bigger than limit of %d.\n", ts_size);
            return FAILURE; 
        }
#endif
        index++;
	}
    free(result);
    return SUCCESS;
}

float ts_euclidean_distance(ts_type * t, ts_type * s, int size) {
    float distance = 0;
    while (size > 0) {
        size--;
        distance += (t[size] - s[size]) * (t[size] - s[size]);
    }
    //distance = sqrtf(distance);
    
    return distance;
}

ts_type ts_euclidean_distance_reordered(ts_type * q, ts_type * t , int j , int  size ,ts_type bsf, int * order)
{
    int i;
    ts_type sum = 0;
    for ( i = 0 ; i < size && sum < bsf ; i++ )
    {
      ts_type x = t[order[i]];
      sum += (x-q[i])*(x-q[i]);      
    }
    return sum;
}

/** 
 This function prints a ts record of a size.
 @param ts *ts
 @param int size
*/
void ts_print(ts_type *ts, int size) 
{
    int i;
    for (i=0; i < size; i++) {
        printf("%lf", ts[i]);
    }
    printf("\n");
}
