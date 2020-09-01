//
//  dot_exporter.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "config.h"
#include "globals.h"
#include "isax_node.h"
#include "isax_index.h"

void bst_print_dot_null(void * key, int nullcount, FILE* stream);
void bst_print_dot_aux(isax_node* node, FILE* stream);
void bst_print_dot(isax_node* tree, FILE* stream);
void isax_print_dot(isax_index* index, FILE *stream);

