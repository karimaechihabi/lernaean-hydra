/******************************************************************************
 * Copyright (C) 2013-2014, Kostas Zoumpatianos <zoumpatianos@disi.unitn.eu>  *
 * All rights reserved.                                                       *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 ******************************************************************************/

#ifndef isaxlib_sax_h
#define isaxlib_sax_h
#include "../globals.h"
#include "../ts.h"

int sax_from_ts(ts_type *ts_in, sax_type *sax_out, int ts_values_per_segment,
            int segments, int cardinality, int bit_cardinality);
int compare(const void *a, const void *b);
float minidist_paa_to_isax(float *paa, sax_type *sax, sax_type *sax_cardinalities, 
                           sax_type max_bit_cardinality,
                           sax_type max_cardinality, 
                           int number_of_segments, 
                           int min_val, int max_val, float ratio_sqrt);
float minidist_paa_to_paa(float *paa1, float *paa2,
						  int number_of_segments,
						  float ratio_sqrt);
int paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment);
int sax_from_paa (ts_type *paa, sax_type *sax, int segments,
                            int cardinality, int bit_cardinality);
#endif
