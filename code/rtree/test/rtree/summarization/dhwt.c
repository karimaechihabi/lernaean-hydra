/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#include "../globals.h"
#include "../ts.h"
#include "dhwt.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int dhwt_from_ts(ts_type *ts_in, ts_type *dhwt_out, int original_points) {
	int level, s;
	int total_dhwt_levels = log2(original_points);
    for (level=total_dhwt_levels; level>=0; level--) {
        if(level == 0)
        {
        	dhwt_out[0] = 0;
            for (s=0; s<original_points; s++) {
            	dhwt_out[0] += ts_in[s];
            }
            dhwt_out[0] /= original_points;
        }
        else
        {
        	int start = 0;
            int step = pow(2, total_dhwt_levels - level + 1);
            int index = pow(2, level - 1) - 1;
            for (start=0; start<original_points; start+=step) {
                index++;
                float node_value = 0;
                int i=start;
                for(i=start; i<start+step; i++)
                {
                    if(i-start < (step)/2) {
                        node_value += ts_in[i];
                    }
                    else {
                        node_value -= ts_in[i];
                    }
                }
                node_value /= step;
                dhwt_out[index] = node_value;
            }
        }
    }
	return EXIT_SUCCESS;
}


float minidist_dhwt_to_dhwt(ts_type *dhwt1, ts_type *dhwt2, int original_points,
							int reduced_points) {
	int levels = log2(original_points);
    int read_until = log2(reduced_points);
	float d=0;

	int l;
    int read_levels = 0;
	for(l=levels; l>=1; l--) {
        if(read_levels>=read_until)
            break;
		int l_start = pow(2, (levels - l));
		int l_end = pow(2, (levels - l + 1)) - 1;
		if(l_start == 1)
			l_start = 0;
		float sum = 0;
		int i;
		for(i=l_start; i<=l_end; i++)
		{
			sum += pow(fabsf(dhwt1[i] - dhwt2[i]), 2);
		}
        //printf(">>> [Distance at level %d] %.2lf (total: %.3lf)\n", l, pow(2, l) * sum, d+pow(2,l)*sum);
		d += pow(2,l)*sum;
        read_levels++;
	}
	return (float) sqrtf(d);
}

float test_dhwt_distance(ts_type *ts1, ts_type *ts2, int size) {
	ts_type * dhwt1 = malloc(sizeof(ts_type) * size);
	dhwt_from_ts(ts1, dhwt1, size);
//	ts_print(dhwt1, size);

	ts_type * dhwt2 = malloc(sizeof(ts_type) * size);
	dhwt_from_ts(ts2, dhwt2, size);
	//ts_print(dhwt2, size);

	double real_distance = ts_euclidean_distance(ts1, ts2, size);
	double transformation_distance = minidist_dhwt_to_dhwt(dhwt1, dhwt2, size, size);
	//printf("I got a transformation distance: %.5lf\n", transformation_distance);
	//printf("I got a real distance: %.5lf\n", real_distance);

	if(transformation_distance == 0)
		return 0;

	return real_distance / transformation_distance;
}
