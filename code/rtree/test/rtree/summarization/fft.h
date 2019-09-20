/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#ifndef FFT_H_
#define FFT_H_
#include "../globals.h"
#include "../ts.h"

int fft_from_ts(ts_type *ts,  ts_type *fft_data, int size, char is_normalized);
int ts_from_fft(ts_type *fft_data,  ts_type *ts, int size, char is_normalized);
double minidist_fft_to_fft(ts_type *fft1, ts_type *fft2, int original_points, int reduced_points);
double test_fft_distance(ts_type *ts1, ts_type *ts2, int size);

#endif /* FFT_H_ */

