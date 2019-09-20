/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#include "sax.h"
#include "sax_breakpoints.h"

#include "../globals.h"
#include "../ts.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/** 
 This is used for converting to sax
 */
int compare(const void *a, const void *b)
{
    float * c = (float *) b - 1;
    if (*(float*)a>*(float*)c && *(float*)a<=*(float*)b) {
        return 0;
    }
    else if (*(float*)a<=*(float*)c) {
        return -1;
    }
    else
    {
        return 1;
    }
}

/** 
 Calculate paa.
 */
int paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment) {
    int s, i;
    for (s=0; s<segments; s++) {
        paa_out[s] = 0;
        for (i=0; i<ts_values_per_segment; i++) {
            paa_out[s] += ts_in[(s * ts_values_per_segment)+i];
        }
        paa_out[s] /= ts_values_per_segment;
    }
    return EXIT_SUCCESS;
}


int sax_from_paa (ts_type *paa, sax_type *sax, int segments,
                            int cardinality, int bit_cardinality) {
    int offset = ((cardinality - 1) * (cardinality - 2)) / 2;
    
    int si;
    for (si=0; si<segments; si++) {
        sax[si] = 0;
        float *res = (float *) bsearch(&paa[si], &sax_breakpoints[offset], cardinality - 1,
                                       sizeof(ts_type), compare);
        if(res != NULL)
            sax[si] = (int) (res -  &sax_breakpoints[offset]);
        else if (paa[si] > 0)
            sax[si] = cardinality-1;
    }

    return EXIT_SUCCESS;
}

/**
 This function converts a ts record into its sax representation.
 */
int sax_from_ts(ts_type *ts_in, sax_type *sax_out, int ts_values_per_segment,
                 int segments, int cardinality, int bit_cardinality)
{
    // Create PAA representation
    float * paa = malloc(sizeof(float) * segments);
    if(paa == NULL) {
        fprintf(stderr,"error: could not allocate memory for PAA representation.\n");
        return EXIT_FAILURE;
    }
    
    int s, i;
    for (s=0; s<segments; s++) {
        paa[s] = 0;
        for (i=0; i<ts_values_per_segment; i++) {
            paa[s] += ts_in[(s * ts_values_per_segment)+i];
        }
        paa[s] /= ts_values_per_segment;
    }
    
    // Convert PAA to SAX
    // Note: Each cardinality has cardinality - 1 break points if c is cardinality
    //       the breakpoints can be found in the following array positions: 
    //       FROM (c - 1) * (c - 2) / 2 
    //       TO   (c - 1) * (c - 2) / 2 + c - 1
    int offset = ((cardinality - 1) * (cardinality - 2)) / 2;
    
    int si;
    for (si=0; si<segments; si++) {
        sax_out[si] = 0;

        float *res = (float *) bsearch(&paa[si], &sax_breakpoints[offset], cardinality - 1,
                                       sizeof(ts_type), compare);
        if(res != NULL)
            sax_out[si] = (int) (res -  &sax_breakpoints[offset]);
        else if (paa[si] > 0)
            sax_out[si] = cardinality-1;
    }
    
    free(paa);
    return EXIT_SUCCESS;
}

float minidist_paa_to_paa(float *paa1, float *paa2,
						  int number_of_segments,
						  float ratio_sqrt) {
	float dist = ratio_sqrt * ts_euclidean_distance(paa1, paa2, number_of_segments);
	return dist;
}

float minidist_paa_to_isax(float *paa, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           sax_type max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt) 
{
   
    float distance = 0;
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;
    int i;
    for (i=0; i<number_of_segments; i++) {
        
        sax_type c_c = sax_cardinalities[i];
        sax_type c_m = max_bit_cardinality;
        sax_type v = sax[i];
        
        sax_type region_lower = v << (c_m - c_c);
        sax_type region_upper = (~((int)MAXFLOAT << (c_m - c_c)) | region_lower);
        
        float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        float breakpoint_upper = 0; // <-- - || -
        
        
        if (region_lower == 0) {
            breakpoint_lower = min_val;
        }
        else
        {
            breakpoint_lower = sax_breakpoints[offset + region_lower - 1];
        }
        if (region_upper == max_cardinality - 1) {
            breakpoint_upper = max_val;
        }
        else
        {
            breakpoint_upper = sax_breakpoints[offset + region_upper];
        }
        if (breakpoint_lower > paa[i]) {
            distance += pow(breakpoint_lower - paa[i], 2);
        }
        else if(breakpoint_upper < paa[i]) {
            distance += pow(breakpoint_upper - paa[i], 2);
        }
    }
    
    distance = ratio_sqrt * sqrtf(distance);
    return distance;
}


