#ifndef sfa_sfa_file_buffer_h
#define sfa_sfa_file_buffer_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfa_node.h"
#include "sfa_trie.h"


struct sfa_file_buffer {

  struct sfa_node * node; //the buffer points back to its node
  struct sfa_file_map * position_in_map; //the buffer points back to its position in file map

  struct sfa_record * buffered_list;
  
  unsigned int disk_count; //  by default
  int buffered_list_size;   //number of series currently stored in this buffer

  boolean in_disk; //false by default
  boolean do_not_flush;
};

enum response sfa_file_buffer_init(struct sfa_node *node);
enum response flush_buffer_to_disk(struct sfa_trie *trie, struct sfa_node *node, boolean full_flush);
enum response clear_file_buffer(struct sfa_trie *trie, struct sfa_node * node);
enum response delete_file_buffer(struct sfa_trie * trie,struct sfa_node * node);
struct sfa_record * get_all_time_series_in_node(struct sfa_trie * trie, struct sfa_node * node);
struct sfa_record * get_all_records_in_node(struct sfa_trie * trie, struct sfa_node * node);



#endif				   
