#ifndef sfa_sfa_file_loaders_h
#define sfa_sfa_file_loaders_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfa_trie.h"
#include "calc_utils.h"

enum response sfa_query_ascii_file(const char *ifilename, int q_num, const char delimiter, struct sfa_trie *trie, float minimum_distance);
void sfa_query_binary_file(const char *ifilename, int q_num, struct sfa_trie *trie, float minimum_distance);
void sfa_tlb_binary_file(const char *ifilename, int q_num, struct sfa_trie *trie,float minimum_distance);


enum response sfa_trie_binary_file(const char *ifilename, file_position_type ts_num, struct sfa_trie *trie);
enum response sfa_trie_ascii_file(const char *ifilename, file_position_type ts_num, const char delimiter, struct sfa_trie *trie);
enum response reorder_query(ts_type * query_ts, ts_type * query_ts_reordered, int * query_order, int ts_length);

#endif
