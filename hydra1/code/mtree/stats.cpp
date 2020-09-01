
#include "stats.h"

#include <stdio.h>

void print_query_stats(struct stats_info stats, unsigned int query_num, const char * queries)
{
	
        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       stats.query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       stats.query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       stats.query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       stats.query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       stats.query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       stats.query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       stats.query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       stats.query_total_rand_output_count,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       stats.query_exact_distance,
	       queries,
	       query_num
	       );

	double pruning_ratio =(double)stats.query_pruned_ts/stats.total_ts_count;
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	       pruning_ratio,
	       queries,
	       query_num
	       );
}

void print_index_stats(struct stats_info stats, const char * dataset)
{
        //  id = -1 for index and id = query_id for queries
        int id = -1;
        printf("Index_building_input_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_input_time/1000000,
	       dataset,
	       id); 
        printf("Index_building_output_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_output_time/1000000,
	       dataset,
	       id);
        printf("Index_building_cpu_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_building_total_time_secs\t%lf\t%s\t%d\n",
	       stats.idx_building_total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_building_seq_input_count\t%llu\t%s\t%d\n",
	       stats.idx_building_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_building_seq_output_count\t%llu\t%s\t%d\n",
	       stats.idx_building_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_input_count\t%llu\t%s\t%d\n",
	       stats.idx_building_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_building_rand_output_count\t%llu\t%s\t%d\n",
	       stats.idx_building_rand_output_count,
	       dataset,
	       id); 
	
        printf("Index_total_input_time_secs\t%lf\t%s\t%d\n",
	       stats.total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Index_total_output_time_secs\t%lf\t%s\t%d\n",
	       stats.total_output_time/1000000,
	       dataset,
	       id);
        printf("Index_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       stats.total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Index_total_time_secs\t%lf\t%s\t%d\n",
	       stats.total_time/1000000,
	       dataset,
	       id);
	
        printf("Index_total_seq_input_count\t%llu\t%s\t%d\n",
	       stats.total_seq_input_count,
	       dataset,
	       id);
	
        printf("Index_total_seq_output_count\t%llu\t%s\t%d\n",
	       stats.total_seq_output_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_input_count\t%llu\t%s\t%d\n",
	       stats.total_rand_input_count,
	       dataset,
	       id);
	
        printf("Index_total_rand_output_count\t%llu\t%s\t%d\n",
	       stats.total_rand_output_count,
	       dataset,
	       id); 
	/*
        printf("Internal_nodes_count\t%lu\t%s\t%d\n",
	       (stats.total_nodes_count - stats.leaf_nodes_count),
	       dataset,
	       id);

        printf("Leaf_nodes_count\t%lu\t%s\t%d\n",
	       stats.leaf_nodes_count,
	       dataset,
	       id);

	printf("Empty_leaf_nodes_count\t%lu\t%s\t%d\n",
               stats.empty_leaf_nodes_count,	       
	       dataset,
	       id);

	printf("Total_nodes_count\t%lu\t%s\t%d\n",
	       stats.total_nodes_count,
	       dataset,
	       id);

	double size_MB =  (stats.idx_size_bytes)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);
	*/
	printf("Total_ts_count\t%u\t%s\t%d\n",
	       stats.total_ts_count,	       
	       dataset,
	       id);
}
