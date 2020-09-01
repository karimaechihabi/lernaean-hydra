/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

extern struct stats_info_bis full_stats_bis;
/*
namespace SpatialIndex
{
	namespace RTree
	{
		class RTree;
		class Node;
		class Leaf;
		class Index;

		class Statistics : public SpatialIndex::IStatistics
		{
		public:
			Statistics();
			Statistics(const Statistics&);
			virtual ~Statistics();
			Statistics& operator=(const Statistics&);

			//
			// IStatistics interface
			//
			virtual uint64_t getReads() const;
			virtual uint64_t getWrites() const;
			virtual uint32_t getNumberOfNodes() const;
			virtual uint64_t getNumberOfData() const;

			virtual uint64_t getSplits() const;
			virtual uint64_t getHits() const;
			virtual uint64_t getMisses() const;
			virtual uint64_t getAdjustments() const;
			virtual uint64_t getQueryResults() const;
			virtual uint32_t getTreeHeight() const;
			virtual uint32_t getNumberOfNodesInLevel(uint32_t l) const;

		private:
			void reset();

			uint64_t m_u64Reads;

			uint64_t m_u64Writes;

			uint64_t m_u64Splits;

			uint64_t m_u64Hits;

			uint64_t m_u64Misses;

			uint32_t m_u32Nodes;

			uint64_t m_u64Adjustments;

			uint64_t m_u64QueryResults;

			uint64_t m_u64Data;

			uint32_t m_u32TreeHeight;

			std::vector<uint32_t> m_nodesInLevel;

			friend class RTree;
			friend class Node;
			friend class Index;
			friend class Leaf;
			friend class BulkLoader;

			 struct stats_counters_bis {
			   double tS;
			   double tE;
			 };			


			 struct stats_info_bis {

			   struct stats_counters_bis counters;
			 };


		friend std::ostream& operator<<(std::ostream& os, const Statistics& s);
		}; // Statistics

		std::ostream& operator<<(std::ostream& os, const Statistics& s);

		
	}
}
*/
namespace SpatialIndex
{
	namespace RTree
	{
		class RTree;
		class Node;
		class Leaf;
		class Index;

		class Statistics : public SpatialIndex::IStatistics
		{
		public:
			Statistics();
			Statistics(const Statistics&);
			virtual ~Statistics();
			Statistics& operator=(const Statistics&);

			//
			// IStatistics interface
			//
			virtual uint64_t getReads() const;
			virtual uint64_t getWrites() const;
			virtual uint32_t getNumberOfNodes() const;
			virtual uint64_t getNumberOfData() const;

			virtual uint64_t getSplits() const;
			virtual uint64_t getHits() const;
			virtual uint64_t getMisses() const;
			virtual uint64_t getAdjustments() const;
			virtual uint64_t getQueryResults() const;
			virtual uint32_t getTreeHeight() const;
			virtual uint32_t getNumberOfNodesInLevel(uint32_t l) const;


			


        double tS;
        double tE;

        struct timeval total_time_start;
        struct timeval parse_time_start;
        struct timeval input_time_start;
        struct timeval output_time_start;
        struct timeval load_node_start;
        struct timeval current_time;
        struct timeval fetch_start;
        struct timeval fetch_check_start;
        double total_input_time;
        double total_output_time;
        double total_parse_time;
        double load_node_time;
        double total_time;


        struct timeval partial_time_start;
        struct timeval partial_input_time_start;
        struct timeval partial_output_time_start;
        struct timeval partial_load_node_time_start;         

        double partial_time;
        double partial_input_time;
        double partial_output_time;
        double partial_load_node_time;

        unsigned long long partial_seq_input_count;
        unsigned long long partial_seq_output_count;
        unsigned long long partial_rand_input_count;
        unsigned long long partial_rand_output_count;

        unsigned long total_nodes_count;
        unsigned long leaf_nodes_count;
        unsigned long empty_leaf_nodes_count;
        unsigned long loaded_nodes_count;
        unsigned long checked_nodes_count;
        unsigned long loaded_ts_count;
        unsigned long checked_ts_count;
        unsigned long total_ts_count;


	unsigned long leaves_counter;
	int * leaves_sizes;
	int * leaves_heights;
	
	
        double idx_total_input_time;
        double idx_total_output_time;
        double idx_total_cpu_time;
        double idx_total_time;

        unsigned long long idx_total_seq_input_count;
        unsigned long long idx_total_seq_output_count;
        unsigned long long idx_total_rand_input_count;
        unsigned long long idx_total_rand_output_count;

        double query_total_input_time;
        double query_total_output_time;
        double query_total_cpu_time;
        double query_total_time;    

	unsigned long long query_total_seq_input_count;
        unsigned long long query_total_seq_output_count;
        unsigned long long query_total_rand_input_count;
        unsigned long long query_total_rand_output_count;

        unsigned int query_total_loaded_nodes_count;
        unsigned int query_total_checked_nodes_count;
	unsigned long long query_total_loaded_ts_count;
        unsigned long long query_total_checked_ts_count;
	
        double query_approx_distance;
        char * query_approx_node_filename;  
        unsigned int query_approx_node_size;
        unsigned int query_approx_node_level;  

        double query_exact_distance;
        char * query_exact_node_filename;
        unsigned int query_exact_node_size;
        unsigned int query_exact_node_level;
  
        double query_eff_epsilon;
        double query_pruning_ratio;

        long long idx_size_bytes;
        long long idx_size_blocks; 
			
		private:
			void reset();

			uint64_t m_u64Reads;

			uint64_t m_u64Writes;

			uint64_t m_u64Splits;

			uint64_t m_u64Hits;

			uint64_t m_u64Misses;

			uint32_t m_u32Nodes;

			uint64_t m_u64Adjustments;

			uint64_t m_u64QueryResults;

			uint64_t m_u64Data;

			uint32_t m_u32TreeHeight;

			std::vector<uint32_t> m_nodesInLevel;

			friend class RTree;
			friend class Node;
			friend class Index;
			friend class Leaf;
			friend class BulkLoader;


		friend std::ostream& operator<<(std::ostream& os, const Statistics& s);
		}; // Statistics

		std::ostream& operator<<(std::ostream& os, const Statistics& s);

		
	}
}
