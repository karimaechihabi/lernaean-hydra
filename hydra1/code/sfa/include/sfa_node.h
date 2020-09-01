#ifndef sfalib_sfa_node_h
#define sfalib_sfa_node_h


#include "../config.h"
#include "../globals.h"    
#include "sfa_file_buffer.h"


struct sfa_node {
    struct sfa_node *children;
    struct sfa_node *next;
  
    struct sfa_node *parent;

    struct sfa_file_buffer * file_buffer;

    char * filename;

    unsigned char * sfa_word;
 
    ts_type * min_values;
    ts_type * max_values;
  
    unsigned int prefix_length;
  
    unsigned int node_size;
    unsigned int level;

    boolean is_leaf;
  
};

struct sfa_node  * sfa_root_node_init();
struct sfa_node  * sfa_leaf_node_init();

enum response sfa_node_append_record(struct sfa_trie * trie, struct sfa_node * node, struct sfa_record* record);

enum response sfa_node_create_filename(struct sfa_trie_settings *settings,
				       struct sfa_node * node,
				       unsigned int index);


enum response sfa_node_update_statistics(struct sfa_trie * trie, struct sfa_node * node, ts_type * dft_transform);
ts_type calculate_node_min_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query, ts_type *query_fft);

ts_type calculate_ts_in_node_distance (struct sfa_trie *trie,
				       struct sfa_node *node,
				       ts_type *query_ts_reordered,
				       int * query_order,
				       unsigned int offset,
				       ts_type bound);

ts_type calculate_node_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query_ts_reordered, int *query_order, unsigned int offset, ts_type bsf);
ts_type calculate_sfa_node_min_distance (struct sfa_trie *trie, struct sfa_node *node, ts_type *query, ts_type *query_fft, unsigned char * query_sfa, ts_type bsf, boolean normed);
ts_type sfa_fft_min_dist (struct sfa_trie  *trie, unsigned char c1_value, unsigned char c2_value, ts_type real_c2, unsigned int dim);

#endif
