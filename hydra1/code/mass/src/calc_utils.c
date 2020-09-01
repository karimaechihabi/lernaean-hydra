//
//  calc_utils.c
//
//  Created by Karima Echihabi on 03/11/2017
//  Copyright 2017 Paris Descartes University. All rights reserved.
//
//  Based on code by Michele Linardi on 01/01/2016
//  Copyright 2016 Paris Descartes University. All rights reserved.
//


#include "../config.h"
#include "../globals.h"
#include "../include/calc_utils.h"
#include "math.h"
#include "fftw3.h"

enum response  mass_compute_mean_std(ts_type * series,
				     unsigned long long series_size,
				     unsigned long long query_size,
				     ts_type * mean_std_complete)
{
	int j;
	int cont=0;
	ts_type sum=0;
	ts_type sum_sq=0;
	for(j=0;j<series_size;j++)
	{
		sum=sum+series[j];
		sum_sq=sum_sq+series[j]*series[j];
		if((j+1)>=query_size)
		{ 
			ts_type mean = sum/query_size;
			mean_std_complete[cont] = mean;
			ts_type sigmaSq = ((sum_sq/query_size)-mean*mean);
			if (sigmaSq>0)
			{				
				mean_std_complete[cont+(series_size-query_size+1)] = sqrtf(sigmaSq);
			}
			else
			{
			  mean_std_complete[cont+(series_size-query_size+1)] = 0;
			}
			sum=sum-series[j-(query_size-1)];
			sum_sq=sum_sq-series[j-(query_size-1)]*series[j-(query_size-1)];
			cont++;
		}
	}

	return SUCCESS;

}

enum response sliding_dot_product(ts_type * series_ts, ts_type * query_ts, unsigned long long series_size,
			      unsigned long long query_size, fftwf_complex * out, fftwf_complex * out2, fftwf_complex * out3,
			      ts_type * final_dot_product_sum,
				  ts_type * output_dp,
				  fftwf_plan plan_backward)
{

	// compute the dot product in frequency domain
        int i;
	unsigned long long nc = series_size + 1 ;
	for (i = 0; i < nc; i++ )
	{
		out3[i][0] = ((out[i][0] * out2[i][0]) - (out[i][1] * out2[i][1]));
		out3[i][1] = ((out[i][0] * out2[i][1]) + (out[i][1] * out2[i][0])); 
	}
	
	fftwf_execute ( plan_backward );
		
	int pos_i=0;
	for (i=(series_size-query_size)+1;i>0;i--)
	{
	        ts_type dp = (final_dot_product_sum[series_size-i] / (ts_type) (series_size*2)) ;
		//printf ( "%12f \n", dp);
		output_dp[pos_i] = dp;
		pos_i++;
	}


	return SUCCESS;
}


enum response reverse_signal(ts_type * query_ts, unsigned long long n)
{  
	int switchL = ceil(n/2);
	unsigned long long i;
	ts_type sw;
	
	for (i = 0; i < switchL; i++)
	{
  	   sw = query_ts[i];
	   query_ts[i] = query_ts[(n-1)-i];
	   query_ts[(n-1)-i] = sw;
	}

	return SUCCESS;
}

