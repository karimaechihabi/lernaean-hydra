//
//  isax_node_buffer.h
//  aisax
//
//  Created by Kostas Zoumpatianos on 4/6/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef aisax_isax_node_buffer_h
#define aisax_isax_node_buffer_h
#include "config.h"
#include "globals.h"
#include "isax_node.h"
#include "isax_node_record.h"
#include "isax_index.h"

enum response add_to_node_buffer(isax_node *node, isax_node_record *record, 
                                 int sax_size, int ts_size, int initial_buffer_size);
enum response flush_node_buffer(isax_node *node, isax_index_settings *settings);
enum response clear_node_buffer(isax_node *node, const char full_clean);
#endif
