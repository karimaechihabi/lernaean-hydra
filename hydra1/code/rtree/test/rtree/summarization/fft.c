/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#include "../globals.h"
#include "../ts.h"
#include "fft.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>

int fft_from_ts(ts_type *ts,  ts_type *fft_data, int size, char is_normalized) {
	int i,j;
	///////////////////////// CONVERT TO FFT ///////////////////////////////////
	// input: TS   (ts_type*)
	// input: size int
	////////////////////////////////////////////////////////////////////////////

	// CONVERT TO DOUBLE
	double *input = malloc(sizeof(double) * size);
	// CONVERT TO AN ARRAY OF SIZE: ORIGINAL + 2
	for(i=0; i<size; i++)
		input[i] = (double) ts[i];
	// INIT FFT STUFF
	fftw_complex* out_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));
	fftw_plan fft = fftw_plan_dft_r2c_1d(size, input, out_cpx, FFTW_ESTIMATE);
	fftw_execute(fft);

	j = 0;
	for(i=0;i<size/2+1;i++)
	{
		// If it is normalized do not use the first complex co-efficient as it's
		// always 0 for normalized data.
		if(i==0 && is_normalized) {
			continue;
		}
		fft_data[j]   = (ts_type) out_cpx[i][0];	//Extract real component
		fft_data[j+1] = (ts_type) out_cpx[i][1];   //Extract imaginary component
		j += 2;
	}
	// FREE MEMORY
	free(input);
	fftw_destroy_plan(fft);
	fftw_free(out_cpx);
	////////////////////////////////////////////////////////////////////////////
	
	return EXIT_SUCCESS;
}

int ts_from_fft(ts_type *fft_data,  ts_type *ts, int size, char is_normalized) {
	int i,j;
	int coefficients = (size/2) + 1;

	// CONVERT TO DOUBLE
	double *output = malloc(sizeof(double) * size);
	for(i=0; i<size; i++)
		output[i] = 0;


	// INIT FFT STUFF
	fftw_complex* out_cpx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*(size/2+1));

	j = 0;
	for(i=0;i<coefficients;i++)
	{
		// If it is normalized use 0.0 for the first co-efficient, as it is
		// always 0 for normalized data.
		if(i==0 && is_normalized) {
			out_cpx[0][0] = 0.0;
			out_cpx[0][1] = 0.0;
		}
		else {
			out_cpx[i][0] = (double) fft_data[j];
			out_cpx[i][1] = (double) fft_data[j+1];
			j += 2;
		}
	}

	fftw_plan ifft = fftw_plan_dft_c2r_1d(size, out_cpx, output, FFTW_ESTIMATE);
	fftw_execute(ifft);

	for(i=0; i<size; i++)
		ts[i] = output[i] / size;

	// FREE MEMORY
	free(output);
	fftw_destroy_plan(ifft);
	fftw_free(out_cpx);
	////////////////////////////////////////////////////////////////////////////

	return EXIT_SUCCESS;
}


double minidist_fft_to_fft(ts_type *fft1, ts_type *fft2, int original_points, int reduced_points) {

	double distance = 0.0;
	int i;

	for(i=0; i<reduced_points; i+=2) {
		double real = (double) fft1[i] - fft2[i];
	    double imag = (double) fft1[i+1] - fft2[i+1];
	    double abs = (real * real) + (imag * imag);
        if(i<reduced_points-2)
        {
        	distance += 2 * abs;
        }
        else
        {
        	distance += abs;
        }
	}

    distance = sqrtf(distance/original_points);
    return distance;
}

double test_fft_distance(ts_type *ts1, ts_type *ts2, int size) {
	int reduced_points = size;

	ts_type * fft1 = malloc(sizeof(ts_type) * size);
	fft_from_ts(ts1, fft1, size, 1);

	ts_type * fft2 = malloc(sizeof(ts_type) * size);
	fft_from_ts(ts2, fft2, size, 1);


	//ts_print(ts1, size);
	//ts_print(ts2, size);


	float real_distance = ts_euclidean_distance(ts1, ts2, size);
	float transformation_distance = minidist_fft_to_fft(fft1, fft2, size, reduced_points);

	//printf("RDIST: %lf, MDIST: %lf\n", real_distance, transformation_distance);
	if(transformation_distance == 0)
		return 0;

	free(fft1);
	free(fft2);

	return real_distance / transformation_distance;
}
