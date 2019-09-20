#ifndef masslib_mass_instance_h
#define masslib_mass_instance_h
#include "../config.h"
#include "../globals.h"
#include "calc_utils.h"
#include "ts.h"


struct mass_settings {
    const char* root_directory;
    char* data_filename;
    char* queries_filename;  
    double buffered_memory_size;
    unsigned long long timeseries_size;
    unsigned long long timeseries_byte_size;
    unsigned long long timeseries_count;
    unsigned long long  subsequence_size;
    unsigned long long subsequence_byte_size;
};

struct stats_info {

        double query_calc_fft_input_time;  
        double query_calc_fft_output_time;
        double query_calc_fft_cpu_time;
        double query_calc_fft_total_time;

        unsigned long long query_calc_fft_seq_input_count;
        unsigned long long query_calc_fft_seq_output_count;
        unsigned long long query_calc_fft_rand_input_count;
        unsigned long long query_calc_fft_rand_output_count;

        double query_dot_product_fft_input_time;  
        double query_dot_product_fft_output_time;
        double query_dot_product_fft_cpu_time;
        double query_dot_product_fft_total_time;

        unsigned long long query_dot_product_fft_seq_input_count;
        unsigned long long query_dot_product_fft_seq_output_count;
        unsigned long long query_dot_product_fft_rand_input_count;
        unsigned long long query_dot_product_fft_rand_output_count;

        double query_calc_dist_input_time;
        double query_calc_dist_output_time;
        double query_calc_dist_cpu_time;
        double query_calc_dist_total_time;    

        unsigned long long query_calc_dist_seq_input_count;
        unsigned long long query_calc_dist_seq_output_count;
        unsigned long long query_calc_dist_rand_input_count;
        unsigned long long query_calc_dist_rand_output_count;

        double query_total_input_time;
        double query_total_output_time;
        double query_total_cpu_time;
        double query_total_time;    

        unsigned long long query_total_seq_input_count;
        unsigned long long query_total_seq_output_count;
        unsigned long long query_total_rand_input_count;
        unsigned long long query_total_rand_output_count;

        double query_exact_distance;
        double query_exact_distance_sanity_check;
        unsigned  long long  query_exact_offset;

        double queries_calc_fft_input_time;  
        double queries_calc_fft_output_time;
        double queries_calc_fft_cpu_time;
        double queries_calc_fft_total_time;

        unsigned long long queries_calc_fft_seq_input_count;
        unsigned long long queries_calc_fft_seq_output_count;
        unsigned long long queries_calc_fft_rand_input_count;
        unsigned long long queries_calc_fft_rand_output_count;
  
        double queries_dot_product_fft_input_time;  
        double queries_dot_product_fft_output_time;
        double queries_dot_product_fft_cpu_time;
        double queries_dot_product_fft_total_time;

        unsigned long long queries_dot_product_fft_seq_input_count;
        unsigned long long queries_dot_product_fft_seq_output_count;
        unsigned long long queries_dot_product_fft_rand_input_count;
        unsigned long long queries_dot_product_fft_rand_output_count;

        double queries_calc_dist_input_time;
        double queries_calc_dist_output_time;
        double queries_calc_dist_cpu_time;
        double queries_calc_dist_total_time;    

        unsigned long long queries_calc_dist_seq_input_count;
        unsigned long long queries_calc_dist_seq_output_count;
        unsigned long long queries_calc_dist_rand_input_count;
        unsigned long long queries_calc_dist_rand_output_count;

        double queries_total_input_time;
        double queries_total_output_time;
        double queries_total_cpu_time;
        double queries_total_time;    

        unsigned long long queries_total_seq_input_count;
        unsigned long long queries_total_seq_output_count;
        unsigned long long queries_total_rand_input_count;
        unsigned long long queries_total_rand_output_count;
    
        double total_input_time;
        double total_output_time;
        double total_cpu_time;
        double total_time;    
        double total_time_sanity_check;
  
        unsigned long long total_seq_input_count;
        unsigned long long total_seq_output_count;
        unsigned long long total_rand_input_count;
        unsigned long long total_rand_output_count;
  
        unsigned long long total_ts_count;  
        unsigned int total_queries_count; //this could be different from the input queries to eliminate cases d(Q,S) = 0
    
};

struct  mass_instance {
    unsigned long long total_processed_ts;
    struct mass_settings *settings;
    struct mass_buffer_manager * buffer_manager;
    struct stats_info * stats;
};

struct mass_instance * mass_inst_init(struct mass_settings *settings);
struct mass_settings * mass_settings_init(
					  char * dataset,
					  char * queries,
					  unsigned long long timeseries_size, 
					  unsigned long long timeseries_count,
					  unsigned long long subsequence_size, 					  
					  double buffered_memory_size,
					  boolean is_db_new);
  
enum response mass_init_stats(struct mass_instance * mass_inst);

void mass_print_preprocessing_stats (struct mass_instance *mass_inst, char * dataset);
void mass_print_query_stats(struct mass_instance * mass_inst, unsigned int query_num, char * queries);
void mass_print_queries_stats(struct mass_instance * mass_inst);

void mass_inst_destroy(struct mass_instance * mass_inst, boolean is_db_new);
void destroy_buffer_manager(struct mass_instance *mass_inst);




#endif
