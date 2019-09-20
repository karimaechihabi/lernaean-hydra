//
//  isax_file_loaders.h
//  isax
//
//  Created by Kostas Zoumpatianos on 4/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isax_isax_file_loaders_h
#define isax_isax_file_loaders_h
#include "config.h"
#include "globals.h"
#include "sax/ts.h"
#include "sax/sax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isax_index.h"

void isax_index_ascii_file(const char *ifilename, file_position_type ts_num, 
                           const char delimiter, isax_index *index);
void isax_query_ascii_file(const char *ifilename, int q_num, 
                           const char delimiter, isax_index *index,
						   float minimum_distance);
void isax_index_binary_file(const char *ifilename, file_position_type ts_num, isax_index *index);
void isax_query_binary_file(const char *ifilename, int q_num,  isax_index *index,
						   float minimum_distance);
enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length);
int znorm_comp(const void *a, const void* b);
void isax_tlb_binary_file(const char *ifilename, int q_num, isax_index *index);


#endif
