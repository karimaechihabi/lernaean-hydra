//
//  dft.h
//  vaplus C version
//
//  Created by Karima Echihabi on 28/04/2017
//  Copyright 2016 Paris Descartes University. All rights reserved.
//  
//
#ifndef vapluslib_dft_h
#define vapluslib_dft_h

#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include "../include/vaplus_file_loaders.h"
#include "../include/calc_utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <math.h>

#include <fftw3.h>
#include <sys/types.h>

struct dft {
  int fft_size;
  //int actual_fft_size;
  fftw_plan plan;
};

enum response init_fft(struct vaplus_index *index, int data_size, int transforms_size);
enum response init_fft2(struct vaplus_index *index, double *in, int data_size, int transforms_size);


enum response destroy_fft(struct vaplus_index * index);
enum response mft_from_ts(struct vaplus_index * index, int start, int end);
double real_ephi (double u, double m);
double complex_ephi (double u, double m);
double complex_mul_real(double r1, double im1, double r2, double im2);
double complex_mul_imag(double r1, double im1, double r2, double im2);
double normalize_ft(double * mft_data, int start, int end, int idx, struct vaplus_index *index);
void fft_from_ts_chunk(struct vaplus_index * index);
void fft_from_ts(struct vaplus_index * index, ts_type *ts,  fftwf_complex *ts_out, ts_type *transform, fftwf_plan plan_forward);
void vaplus_from_fft(struct vaplus_index * index, ts_type * cur_transform, unsigned int * cur_approx);

#endif
