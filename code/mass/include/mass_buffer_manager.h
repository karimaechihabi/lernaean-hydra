#ifndef masslib_mass_buffer_manager_h
#define masslib_mass_buffer_manager_h
#include "../config.h"
#include "../globals.h"
#include "ts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mass_instance.h"


struct mass_buffer_manager {
  
  unsigned long long chunk_size;
  unsigned long long last_chunk_size;
  unsigned long long num_chunks;
  unsigned int current_chunk_id;
  
  char *mem_array;
  char *current_record;
  unsigned int current_record_index;
  unsigned int max_record_index;

};

enum response init_buffer_manager(struct mass_instance *mass_inst);
enum response set_buffered_memory_size(struct mass_instance * mass_inst);

#endif
