//
//  visualize_index.c
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>

#include "ads/isax_index.h"
#include "ads/isax_node.h"
#include "ads/isax_visualize_index.h"
#include "ads/sax/sax.h"

void bst_print_dot_null(void * key, int nullcount, FILE* stream)
{
    fprintf(stream, "    null%d [shape=point];\n", nullcount);
    fprintf(stream, "    %lu -> null%d;\n", (size_t)key, nullcount);
}

void bst_print_dot_aux(isax_node* node, FILE* stream)
{
    static int nullcount = 0;
    
    if (node->left_child)
    {
        fprintf(stream, "    %lu -> %lu;\n", (size_t)node, (size_t)node->left_child);
        bst_print_dot_aux(node->left_child, stream);
    }
    else
        bst_print_dot_null(node, nullcount++, stream);
    
    if (node->right_child)
    {
        fprintf(stream, "    %lu -> %lu;\n", (size_t)node, (size_t)node->right_child);
        bst_print_dot_aux(node->right_child, stream);
    }
    else
        bst_print_dot_null(node, nullcount++, stream);
}

void bst_print_dot(isax_node* tree, FILE* stream)
{
    //fprintf(stream, "digraph BST {\n");
    fprintf(stream, "    node [fontname=\"Arial\"];\n");
    
    if (!tree)
        fprintf(stream, "\n");
    else if (!tree->right_child && !tree->left_child)
        fprintf(stream, "    %lu;\n", (size_t)tree);
    else
        bst_print_dot_aux(tree, stream);
    
    //fprintf(stream, "}\n");
}

void isax_print_dot(isax_index* index, FILE *stream) 
{
    isax_node *node = index->first_node;
    fprintf(stream, "digraph BST {\n");
    while (node != NULL) {
        if(!node->is_leaf) {
            fprintf(stream, "    %lu -> %lu;\n", (size_t)index, (size_t)node);
            bst_print_dot(node, stream);
            fprintf(stream, "\n");
            
        }
        node = node->next;
    }
    fprintf(stream, "\n}");
    
}
