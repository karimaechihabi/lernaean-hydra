//
//  dstree_file_loaders.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#ifndef dstree_dstree_file_loaders_h
#define dstree_dstree_file_loaders_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dstree_index.h"
#include "calc_utils.h"

enum response dstree_query_ascii_file(const char *ifilename, int q_num, const char delimiter, struct dstree_index *index, float minimum_distance, ts_type epsilon, ts_type delta);
enum response dstree_query_binary_file(const char *ifilename, int q_num,
				       struct dstree_index *index, float minimum_distance,
				       ts_type epsilon, ts_type delta);  
enum response dstree_knn_query_binary_file(const char *ifilename, int q_num, struct dstree_index *index,
					   float minimum_distance, ts_type epsilon, ts_type r_delta,
					   unsigned int k, boolean track_bsf, boolean track_pruning,
					   boolean dump_mindists, boolean max_policy,
					   unsigned int nprobes, unsigned char incremental);
enum response dstree_index_binary_file(const char *ifilename, file_position_type ts_num, struct dstree_index *index);
enum response dstree_index_ascii_file(const char *ifilename, file_position_type ts_num, const char delimiter, struct dstree_index *index);
enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length);
enum response dstree_tlb_binary_file(const char *ifilename, int q_num, struct dstree_index *index,float minimum_distance);

#endif
