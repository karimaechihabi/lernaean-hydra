//
//  cals_utils.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef seriallib_serial_scan_utils_h
#define seriallib_serial_scan_utils_h

#include "../config.h"
#include "../globals.h"
#include "math.h"
#include "ts.h"



/// Data structure for sorting the query.
typedef struct q_index
    {   double value;
        int    index;
    } q_index;

typedef struct query_result {
    ts_type distance;
    size_t file_position;
};

struct stats_info {


        double query_total_input_time;
        double query_total_output_time;
        double query_total_load_node_time;
        double query_total_cpu_time;
        double query_total_dist1_time;
        double query_total_dist2_time;
        double query_total_time;    

        double queries_total_input_time;
        double queries_total_output_time;
        double queries_total_load_node_time;
        double queries_total_cpu_time;

        double queries_total_time;

        unsigned long long query_total_seq_input_count;
        unsigned long long query_total_seq_output_count;
        unsigned long long query_total_rand_input_count;
        unsigned long long query_total_rand_output_count;

        unsigned long long queries_total_seq_input_count;
        unsigned long long queries_total_seq_output_count;
        unsigned long long queries_total_rand_input_count;
        unsigned long long queries_total_rand_output_count;
        unsigned long long total_ts_count;

        double query_exact_distance;
        ts_type query_exact_file_position;
  
        double query_lb_distance;  
  
        double query_tlb;
        double query_sum_tlb;
  
        double queries_min_tlb;
        double queries_max_tlb;
        double queries_sum_tlb;
        double queries_sum_squares_tlb;
        double queries_avg_tlb;
        double queries_sd_tlb;
  
  
        unsigned long long tlb_ts_count; //this could be different from the total_ts_count. It includes only the ts for which d(Q,S) != 0
    
}; 


void update_query_stats(struct stats_info *stats,unsigned int query_id,
			unsigned int found_knn, struct query_result bsf_result);
void print_query_stats(struct stats_info *stats, unsigned int query_num,unsigned int found_knn, char * queries);
enum response serial_scan(const char * dataset, int dataset_size,const char * queries, int queries_size, int ts_length, float minimum_distance, struct stats_info *stats, unsigned int k);
int znorm_comp(const void *a, const void* b);
enum response reorder_query(ts_type * query_ts, int * query_order, int ts_length);

#endif

