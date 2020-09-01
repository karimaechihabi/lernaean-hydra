//
//  vaplus_index.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#ifndef vapluslib_vaplus_index_h
#define vapluslib_vaplus_index_h

#include "../config.h"
#include "../globals.h"
#include "calc_utils.h"
#include "vaplus_node.h"
#include "ts.h"


struct vaplus_index_settings {
    const char* root_directory;  
    unsigned int timeseries_size;
    unsigned int max_leaf_size;    
    unsigned int max_filename_size;
    unsigned int fft_size;  
    unsigned int num_bits;
    double buffered_memory_size;  
    boolean is_norm;
    ts_type norm_factor;  
    unsigned int start_offset;
    unsigned int histogram_type;
    unsigned int lb_dist;  
    unsigned int  dataset_size;  
    unsigned int minimal_depth;
    char * raw_filename;
    int ts_byte_size;
    boolean is_new;

};

struct vaplus_record {
  ts_type * timeseries;
  ts_type * dft_transform;
  unsigned char * vaplus_word;
};


struct  vaplus_index{
    unsigned long long total_records;
    struct vaplus_node *first_node;
    //struct vaplus_node first_node;
    struct vaplus_index_settings *settings;
    struct vaplus_file_buffer_manager * buffer_manager;
    ts_type ** bins;
    unsigned int * cells;    
};

struct vaplus_index * vaplus_index_init(struct vaplus_index_settings *settings);
struct vaplus_index * vaplus_index_read(const char* root_directory); 
enum response vaplus_index_write(struct vaplus_index *index);
struct vaplus_node * vaplus_node_read(struct vaplus_index *index, FILE *file);
enum response vaplus_node_write(struct vaplus_index *index, struct vaplus_node *node, FILE *file);

void vaplus_index_insert(struct vaplus_index * index,
		     //		     struct vaplus_node * node,
		     int i_index,		     
		     struct vaplus_record * record
		     );

struct vaplus_index_settings * vaplus_index_settings_init(const char * root_directory,
						 unsigned int timeseries_size, 
                                                 unsigned int word_length, 
                                                 unsigned int num_bits,
						 double buffered_memory_size,
						 boolean is_norm,
						 unsigned int dataset_size,
						 boolean is_index_new);


void vaplus_index_destroy(struct vaplus_index *index, struct vaplus_node *node, boolean is_index_new);
void destroy_buffer_manager(struct vaplus_index *index);

enum response append_ts_to_buffered_list(struct vaplus_index * index, struct vaplus_node * node, ts_type * timeseries);

enum response vaplus_index_bins_init(struct vaplus_index * index);
void vaplus_index_set_bins(struct vaplus_index *index, char * ifilename);


enum response vaplus_index_bins_calculate_non_uniform(struct vaplus_index * index);

//struct vaplus_ts_buffer * copy_ts_in_mem(struct vaplus_index * index, struct vaplus_node * node);

#endif
