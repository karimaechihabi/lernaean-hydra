#ifndef sfa_sfa_file_buffer_manager_h
#define sfa_sfa_file_buffer_manager_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfa_trie.h"
#include "sfa_file_buffer.h"

struct sfa_file_buffer_manager {
  struct sfa_file_map * file_map;
  struct sfa_file_map * file_map_tail;  
  
  unsigned long max_buffered_size;
  long current_count;
  long batch_remove_size;

  unsigned long chunk_size;
  
  int current_record_index;
  int max_record_index;

  char * ts_mem_array;
  char * current_ts_record;

  char * dft_mem_array;
  char * current_dft_record;
  
  char * sfa_mem_array;
  char * current_sfa_record;
  
  int file_map_size;
};


struct sfa_file_map{
  struct sfa_file_buffer * file_buffer;
  struct sfa_file_map * next;
  struct sfa_file_map * prev;  
};

enum response init_file_buffer_manager(struct sfa_trie *trie);
enum response set_buffered_memory_size(struct sfa_trie * trie);
enum response get_file_buffer(struct sfa_trie *trie, struct sfa_node *node);
enum response save_all_buffers_to_disk(struct sfa_trie *trie);
enum response add_file_buffer_to_map(struct sfa_trie * trie, struct sfa_node *node);  

#endif
