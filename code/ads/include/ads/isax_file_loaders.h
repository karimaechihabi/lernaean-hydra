//
//  isax_file_loaders.h
//  isax
//
//  Created by Kostas Zoumpatianos on 4/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isax_isax_file_loaders_h
#define isax_isax_file_loaders_h
#include "../../config.h"
#include "../../globals.h"
#include "sax/ts.h"
#include "sax/sax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isax_index.h"
#include "isax_query_engine.h"

void isax_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_sorted_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_merge_sorted_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_query_binary_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type *, ts_type *,  ts_type *, int *, unsigned int, isax_index *, float, int));

void ads_tlb_binary_file(const char *ifilename, int q_num, isax_index *index);

enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length);
int znorm_comp(const void *a, const void* b);

#endif
