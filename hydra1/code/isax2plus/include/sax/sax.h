//
//  sax.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/10/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isaxlib_sax_h
#define isaxlib_sax_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"

enum response sax_from_ts(ts_type *ts_in, sax_type *sax_out, int ts_values_per_segment, 
            int segments, int cardinality, int bit_cardinality);
void sax_print(sax_type *sax, int segments, int cardinality);
void printbin(unsigned long long n, int size);
void serial_printbin (unsigned long long n, int size);
int compare(const void *a, const void *b);
float minidist_paa_to_isax(ts_type *paa, sax_type *sax, sax_type *sax_cardinalities, 
                           sax_type max_bit_cardinality,
                           sax_type max_cardinality, 
                           int number_of_segments, 
                           int min_val, int max_val, float ratio_sqrt);
ts_type minidist_paa_to_isax_raw(ts_type *paa, sax_type *sax, 
			       sax_type *sax_cardinalities,
			       sax_type max_bit_cardinality,
			       int max_cardinality,
			       int number_of_segments,
			       int min_val,
			       int max_val,
			       ts_type ratio_sqrt) ;

enum response paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment);
enum response sax_from_paa (ts_type *paa, sax_type *sax, int segments, 
                            int cardinality, int bit_cardinality);
#endif
