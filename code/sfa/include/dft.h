#ifndef sfalib_dft_h
#define sfalib_dft_h

#include "../config.h"
#include "../globals.h"
#include <stdio.h>
#include "../include/sfa_file_loaders.h"
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

enum response init_fft(struct sfa_trie *trie, int data_size, int transforms_size);
enum response init_fft2(struct sfa_trie *trie, double *in, int data_size, int transforms_size);


enum response destroy_fft(struct sfa_trie * trie);
enum response mft_from_ts(struct sfa_trie * trie, int start, int end);
double real_ephi (double u, double m);
double complex_ephi (double u, double m);
double complex_mul_real(double r1, double im1, double r2, double im2);
double complex_mul_imag(double r1, double im1, double r2, double im2);
double normalize_ft(double * mft_data, int start, int end, int idx, struct sfa_trie *trie);
void fft_from_ts_chunk(struct sfa_trie * trie);
void fft_from_ts(struct sfa_trie * trie, ts_type *ts,  fftwf_complex *ts_out, ts_type *transform, fftwf_plan plan_forward);
void sfa_from_fft(struct sfa_trie * trie, ts_type * cur_transform, unsigned char * cur_sfa_word);

#endif
