#include "../../config.h"
#include "../../globals.h"

#include <stdio.h>
#include <stdlib.h>

#include <ads.h>

int leaf_id = 0;

isax_node *node_read(FILE *file, int paa_segments, int max_leaf_size) {
	isax_node *node;

	char is_leaf = 0;
	fread(&is_leaf, sizeof(unsigned char), 1, file);

	root_mask_type mask = 0;
	fread(&(mask), sizeof(root_mask_type), 1, file);

	if(is_leaf) {
		node = malloc(sizeof(isax_node));
		node->is_leaf = 1;
		node->has_partia_data_file = 0;
		node->has_full_data_file = 0;
		node->right_child = NULL;
		node->left_child = NULL;
		node->parent = NULL;
		node->next = NULL;
		node->leaf_size = 0;
		node->filename = NULL;
		node->isax_values = NULL;
		node->isax_cardinalities = NULL;
		node->previous = NULL;
		node->split_data = NULL;
		node->mask = 0;

		int filename_size = 0;
		fread(&filename_size, sizeof(int), 1, file);
		if(filename_size > 0) {
			node->filename = malloc(sizeof(char) * (filename_size + 1));
			fread(node->filename, sizeof(char), filename_size, file);
			node->filename[filename_size] = '\0';
			fread(&(node->leaf_size), sizeof(int), 1, file);
			printf("%d\t%2.2lf\t(%d/%d)\n", (leaf_id++), (float)node->leaf_size*100 / (float)max_leaf_size, node->leaf_size, max_leaf_size);
			fread(&(node->has_full_data_file), sizeof(char), 1, file);
			fread(&(node->has_partial_data_file), sizeof(char), 1, file);
			COUNT_NEW_NODE();
		}
		else {
			node->filename = NULL;
			node->leaf_size = 0;
			node->has_full_data_file = 0;
			node->has_partial_data_file = 0;
		}
	}
	else {
		node = malloc(sizeof(isax_node));
		node->buffer = NULL;
		node->is_leaf = 0;
		node->filename = NULL;
		node->has_full_data_file = 0;
		node->has_partial_data_file = 0;
		node->leaf_size = 0;
		node->split_data = malloc(sizeof(isax_node_split_data));
		node->split_data->split_mask = malloc(sizeof(sax_type) * paa_segments);

		fread(&(node->split_data->splitpoint), sizeof(int), 1, file);
		fread(node->split_data->split_mask, sizeof(sax_type), paa_segments, file);
	}
	node->mask = mask;

	char has_isax_data = 0;
	fread(&has_isax_data, sizeof(char), 1, file);

	if(has_isax_data) {
		node->isax_cardinalities = malloc(sizeof(sax_type) * paa_segments);
		node->isax_values = malloc(sizeof(sax_type) * paa_segments);
		fread(node->isax_cardinalities, sizeof(sax_type), paa_segments, file);
		fread(node->isax_values, sizeof(sax_type), paa_segments, file);
	}
	else {
		node->isax_cardinalities = NULL;
		node->isax_values = NULL;
	}

	if(!is_leaf) {
		node->left_child = node_read(file, paa_segments, max_leaf_size);
		node->right_child = node_read(file, paa_segments, max_leaf_size);
	}

	return node;
}


void index_read(const char* root_directory) {
	fprintf(stderr, ">>> Loading index: %s\n", root_directory);
	const char *filename = malloc(sizeof(char) * (strlen(root_directory) + 15));
	filename = strcpy(filename, root_directory);
	filename = strcat(filename, "/index.idx");
	FILE *file = fopen(filename, "rb");
	free(filename);

	int raw_filename_size = 0;
	char *raw_filename = NULL;
	int timeseries_size = 0;
	int paa_segments = 0;
	int sax_bit_cardinality = 0;
	int max_leaf_size = 0;
	int min_leaf_size = 0;
	int initial_leaf_buffer_size = 0;
	int max_total_buffer_size = 0;
	int initial_fbl_buffer_size = 0;
	int total_loaded_leaves = 0;
	int tight_bound = 0;
	int aggressive_check = 0;
	int new_index = 0;

	fread(&raw_filename_size, sizeof(int), 1, file);
	raw_filename = malloc(sizeof(char) * raw_filename_size);
	fread(raw_filename, sizeof(char), raw_filename_size, file);
	fread(&timeseries_size, sizeof(int), 1, file);
	fread(&paa_segments, sizeof(int), 1, file);
	fread(&sax_bit_cardinality, sizeof(int), 1, file);
	fread(&max_leaf_size, sizeof(int), 1, file);
	fread(&min_leaf_size, sizeof(int), 1, file);
	fread(&initial_leaf_buffer_size, sizeof(int), 1, file);
	fread(&max_total_buffer_size, sizeof(int), 1, file);
	fread(&initial_fbl_buffer_size, sizeof(int), 1, file);
	fread(&total_loaded_leaves, sizeof(int), 1, file);
	fread(&tight_bound, sizeof(int), 1, file);
	fread(&aggressive_check, sizeof(int), 1, file);


	while(!feof(file)) {
		int j = 0;
		if(fread(&j, sizeof(int), 1, file)) {
			node_read(file, paa_segments, max_leaf_size);
		}
	}

	fclose(file);
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s [ROOT_DIRECTORY]\n", argv[0]);
		exit(-1);
	}


	index_read(argv[1]);

}
