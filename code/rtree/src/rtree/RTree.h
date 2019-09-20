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

#include "Statistics.h"
#include "Node.h"
#include "PointerPoolNode.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>


namespace SpatialIndex
{
	namespace RTree
	{
		class RTree : public ISpatialIndex
		{
                  //class NNEntry;

		public:
			RTree(IStorageManager&, Tools::PropertySet&);
				// String                   Value     Description
				// ----------------------------------------------
				// IndexIndentifier         VT_LONG   If specified an existing index will be openened from the supplied
				//                          storage manager with the given index id. Behaviour is unspecified
				//                          if the index id or the storage manager are incorrect.
				// Dimension                VT_ULONG  Dimensionality of the data that will be inserted.
				// IndexCapacity            VT_ULONG  The index node capacity. Default is 100.
				// LeafCapactiy             VT_ULONG  The leaf node capacity. Default is 100.
				// FillFactor               VT_DOUBLE The fill factor. Default is 70%
				// TreeVariant              VT_LONG   Can be one of Linear, Quadratic or Rstar. Default is Rstar
				// NearMinimumOverlapFactor VT_ULONG  Default is 32.
				// SplitDistributionFactor  VT_DOUBLE Default is 0.4
				// ReinsertFactor           VT_DOUBLE Default is 0.3
				// EnsureTightMBRs          VT_BOOL   Default is true
				// IndexPoolCapacity        VT_LONG   Default is 100
				// LeafPoolCapacity         VT_LONG   Default is 100
				// RegionPoolCapacity       VT_LONG   Default is 1000
				// PointPoolCapacity        VT_LONG   Default is 500

			virtual ~RTree();



			//
			// ISpatialIndex interface
			//
			virtual void insertData(uint32_t len, const byte* pData, const IShape& shape, id_type shapeIdentifier);
			virtual bool deleteData(const IShape& shape, id_type id);
			virtual void containsWhatQuery(const IShape& query, IVisitor& v);
			virtual void intersectsWithQuery(const IShape& query, IVisitor& v);
			virtual void pointLocationQuery(const Point& query, IVisitor& v);
			virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator&);
			virtual void nearestNeighborQueryOptimized(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator&);			
			virtual void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v);
			virtual void selfJoinQuery(const IShape& s, IVisitor& v);
			virtual void queryStrategy(IQueryStrategy& qs);
			virtual void getIndexProperties(Tools::PropertySet& out) const;
			virtual void addCommand(ICommand* pCommand, CommandType ct);
			virtual bool isIndexValid();
			virtual void getStatistics(IStatistics** out) const;
			virtual inline void printIt2();

virtual inline void stats_init(void);
virtual inline void stats_init_counters(void);
virtual inline void stats_init_leaf_counters (void );
virtual inline void stats_get_leaf_counters (void ) ; 
virtual inline void stats_clean_leaf_counters (void );
virtual inline void stats_count_total_nodes (int);
virtual inline void stats_count_leaf_nodes(int ) ;
virtual inline void stats_count_total_ts (int );
virtual inline void stats_count_checked_ts(int i);
virtual inline void stats_count_checked_nodes(int i); 
virtual inline void stats_reset_query_counters(void);
virtual inline void stats_reset_partial_counters(void);
virtual inline void stats_count_partial_seq_input(void);
virtual inline void stats_count_partial_seq_output(void);
virtual inline void stats_count_partial_rand_input(void);
virtual inline void stats_count_partial_rand_output(void);
virtual inline void stats_count_input_time_start(void);
virtual inline void stats_count_output_time_start(void);
virtual inline void stats_count_total_time_start(void);
virtual inline void stats_count_partial_input_time_start(void);
virtual inline void stats_count_partial_output_time_start(void);
virtual inline void stats_count_partial_time_start(void);
virtual inline void stats_count_input_time_end(void);
virtual inline void stats_count_output_time_end(void);
virtual inline void stats_count_total_time_end(void);
virtual inline void stats_count_partial_input_time_end(void);
virtual inline void stats_count_partial_output_time_end(void);
virtual inline void stats_count_partial_time_end(void);
virtual inline void stats_update_idx_stats(void);
virtual void get_index_footprint(const char * filename);
 virtual void print_index_stats(char * dataset,  const char * index_filename, bool print_leaves);

virtual inline void stats_update_query_stats(ts_type dist, int i);
virtual void print_query_stats(char * queries, unsigned int query_num);
 
		private:
			void initNew(Tools::PropertySet&);
			void initOld(Tools::PropertySet& ps);
			void storeHeader();
			void loadHeader();

			void insertData_impl(uint32_t dataLength, byte* pData, Region& mbr, id_type id);
			void insertData_impl(uint32_t dataLength, byte* pData, Region& mbr, id_type id, uint32_t level, byte* overflowTable);
			bool deleteData_impl(const Region& mbr, id_type id);

			id_type writeNode(Node*);
			NodePtr readNode(id_type page);
			void deleteNode(Node*);

			void rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v);
			void selfJoinQuery(id_type id1, id_type id2, const Region& r, IVisitor& vis);
            void visitSubTree(NodePtr subTree, IVisitor& v);
            
			IStorageManager* m_pStorageManager;

			id_type m_rootID, m_headerID;

			RTreeVariant m_treeVariant;

			ts_type m_fillFactor;

			uint32_t m_indexCapacity;

			uint32_t m_leafCapacity;

			uint32_t m_nearMinimumOverlapFactor;
				// The R*-Tree 'p' constant, for calculating nearly minimum overlap cost.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles', Section 4.1]

			ts_type m_splitDistributionFactor;
				// The R*-Tree 'm' constant, for calculating spliting distributions.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles', Section 4.2]

			ts_type m_reinsertFactor;
				// The R*-Tree 'p' constant, for removing entries at reinserts.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				//  for Points and Rectangles', Section 4.3]

			uint32_t m_dimension;

			Region m_infiniteRegion;

			Statistics m_stats;

			bool m_bTightMBRs;

			Tools::PointerPool<Point> m_pointPool;
			Tools::PointerPool<Region> m_regionPool;
			Tools::PointerPool<Node> m_indexPool;
			Tools::PointerPool<Node> m_leafPool;

			std::vector<Tools::SmartPointer<ICommand> > m_writeNodeCommands;
			std::vector<Tools::SmartPointer<ICommand> > m_readNodeCommands;
			std::vector<Tools::SmartPointer<ICommand> > m_deleteNodeCommands;

#ifdef HAVE_PTHREAD_H
			pthread_mutex_t m_lock;
#endif

			class NNEntry
			{
			public:
				id_type m_id;
				IEntry* m_pEntry;
				ts_type m_minDist;

				NNEntry(id_type id, IEntry* e, ts_type f) : m_id(id), m_pEntry(e), m_minDist(f) {}
				~NNEntry() {}

				struct ascending : public std::binary_function<NNEntry*, NNEntry*, bool>
				{
					bool operator()(const NNEntry* __x, const NNEntry* __y) const { return __x->m_minDist > __y->m_minDist; }
				};
			}; // NNEntry

			class NNComparator : public INearestNeighborComparator
			{
			public:
				ts_type getMinimumDistance(const IShape& query, const IShape& entry)
				{
					return query.getMinimumDistance(entry);
				}

				ts_type getMinimumDistance(const IShape& query, const IData& data)
				{
					IShape* pS;
					data.getShape(&pS);
					ts_type ret = query.getMinimumDistance(*pS);
					delete pS;
					return ret;
				}
			}; // NNComparator

			class ValidateEntry
			{
			public:
				ValidateEntry(Region& r, NodePtr& pNode) : m_parentMBR(r), m_pNode(pNode) {}

				Region m_parentMBR;
				NodePtr m_pNode;
			}; // ValidateEntry

			friend class Node;
			friend class Leaf;
			friend class Index;
			friend class BulkLoader;

			friend std::ostream& operator<<(std::ostream& os, const RTree& t);
		}; // RTree

		std::ostream& operator<<(std::ostream& os, const RTree& t);
	}
}

inline void SpatialIndex::RTree::RTree::printIt2()
{
  std::cout	<< "Printing m_stats.m_u32Nodes = " << m_stats.m_u32Nodes << std::endl;
  m_stats.total_nodes_count += 1;
  std::cout	<< "Printing m_stats.total_input_time = " << m_stats.total_nodes_count << std::endl;  
}


inline void SpatialIndex::RTree::RTree::stats_init_counters (void )
{
   m_stats.total_input_time = 0;
   m_stats.total_output_time = 0;
   m_stats.total_time = 0;
   m_stats.total_parse_time = 0;
   m_stats.load_node_time= 0;
   m_stats.partial_time = 0;
   m_stats.partial_input_time = 0;	
   m_stats.partial_output_time = 0;
   m_stats.partial_load_node_time = 0;
   m_stats.partial_seq_input_count = 0;
   m_stats.partial_seq_output_count = 0;
   m_stats.partial_rand_input_count = 0;
   m_stats.partial_rand_output_count = 0;
   m_stats.total_nodes_count = 0;
   m_stats.leaf_nodes_count = 0;
   m_stats.empty_leaf_nodes_count = 0;
   m_stats.loaded_nodes_count = 0;
   m_stats.checked_nodes_count = 0;
   m_stats.loaded_ts_count = 0;			    
   m_stats.checked_ts_count = 0;
   m_stats.total_ts_count = 0;

   //m_stats.leaves_heights = calloc(count_leaves, sizeof(int));
   //m_stats.leaves_sizes = calloc(count_leaves, sizeof(int));
   //m_stats.leaves_counter = 0;
   
}

inline void SpatialIndex::RTree::RTree::stats_init_leaf_counters (void )
{
   //leaf counts not known yet, so use total nodes count   
  m_stats.leaves_heights = (int *) malloc(m_stats.getNumberOfNodes() * sizeof(int));
  m_stats.leaves_sizes = (int *) malloc(m_stats.getNumberOfNodes() * sizeof(int));
   m_stats.leaves_counter = 0;
   
}

inline void SpatialIndex::RTree::RTree::stats_clean_leaf_counters (void )
{
   //leaf counts not known yet, so use total nodes count   
   free(m_stats.leaves_heights);
   free(m_stats.leaves_sizes);
   
}



inline void SpatialIndex::RTree::RTree::stats_init(void)
{
 
    /*IDX_TOTAL STATISTICS*/
    m_stats.idx_total_input_time = 0;  
    m_stats.idx_total_output_time = 0;
    m_stats.idx_total_cpu_time = 0;
    m_stats.idx_total_time = 0;

    m_stats.idx_total_seq_input_count = 0;
    m_stats.idx_total_seq_output_count = 0;
    m_stats.idx_total_rand_input_count = 0;
    m_stats.idx_total_rand_output_count = 0;    
    
    /*PER QUERY STATISTICS*/    
    m_stats.query_total_input_time = 0;
    m_stats.query_total_output_time = 0;
    m_stats.query_total_cpu_time = 0;
    m_stats.query_total_time = 0;    

    m_stats.query_total_seq_input_count = 0;
    m_stats.query_total_seq_output_count = 0;
    m_stats.query_total_rand_input_count = 0;
    m_stats.query_total_rand_output_count = 0;

    m_stats.query_total_loaded_nodes_count = 0;
    m_stats.query_total_checked_nodes_count = 0;
    m_stats.query_total_loaded_ts_count = 0;
    m_stats.query_total_checked_ts_count = 0;
    
    m_stats.query_approx_distance = 0;
    m_stats.query_approx_node_filename=0;  
    m_stats.query_approx_node_size=0;
    m_stats.query_approx_node_level=0;  

    m_stats.query_exact_distance = 0;
    m_stats.query_exact_node_filename=0;  
    m_stats.query_exact_node_size=0;
    m_stats.query_exact_node_level=0;  
    
    m_stats.query_eff_epsilon = 0;
    m_stats.query_pruning_ratio = 0;
    
    m_stats.idx_size_bytes = 0;
    m_stats.idx_size_blocks = 0;
    
    SpatialIndex::RTree::RTree::stats_init_counters();
    
}

inline void SpatialIndex::RTree::RTree::stats_reset_query_counters(void)
{
   m_stats.loaded_nodes_count = 0;
   m_stats.loaded_ts_count = 0;
   m_stats.checked_nodes_count = 0;
   m_stats.checked_ts_count = 0;
}

inline void SpatialIndex::RTree::RTree::stats_reset_partial_counters(void)
{
   m_stats.partial_seq_output_count = 0;
   m_stats.partial_seq_input_count = 0;
   m_stats.partial_rand_output_count = 0;
   m_stats.partial_rand_input_count = 0;
   m_stats.partial_input_time = 0;
   m_stats.partial_output_time = 0;
   m_stats.partial_load_node_time = 0;
   m_stats.partial_time = 0;
  
}
  /*
        #define COUNT_NEW_NODE ++total_nodes_count; 
        #define COUNT_LEAF_NODE ++leaf_nodes_count;
        #define COUNT_EMPTY_LEAF_NODE ++empty_leaf_nodes_count;
        #define COUNT_TOTAL_TS(num_ts) total_ts_count+=num_ts; //actual ts inserted in index

        #define COUNT_CHECKED_NODE ++checked_nodes_count;
        #define COUNT_LOADED_NODE ++loaded_nodes_count;
        #define COUNT_LOADED_TS(num_ts) loaded_ts_count +=num_ts; //ts loaded to answer query
        #define COUNT_CHECKED_TS(num_ts) checked_ts_count +=num_ts; //ts loaded to answer query
  */

inline void SpatialIndex::RTree::RTree::stats_count_total_ts(int i)
{
  m_stats.total_ts_count += i;
}

inline void SpatialIndex::RTree::RTree::stats_count_total_nodes(int i)
{
  m_stats.total_nodes_count += i;
}

inline void SpatialIndex::RTree::RTree::stats_count_leaf_nodes(int i)
{
  m_stats.leaf_nodes_count += i;
}


inline void SpatialIndex::RTree::RTree::stats_count_checked_ts(int i)
{
  m_stats.checked_ts_count += i;
}

inline void SpatialIndex::RTree::RTree::stats_count_checked_nodes(int i)
{
  m_stats.checked_nodes_count += i;
}


inline void SpatialIndex::RTree::RTree::stats_count_partial_seq_input(void)
{
   ++m_stats.partial_seq_input_count;
}

 
inline void SpatialIndex::RTree::RTree::stats_count_partial_seq_output(void)
{
   ++m_stats.partial_seq_output_count;
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_rand_input(void)
{
   ++m_stats.partial_rand_input_count;
}

 
inline void SpatialIndex::RTree::RTree::stats_count_partial_rand_output(void)
{
   ++m_stats.partial_rand_output_count;
}

inline void SpatialIndex::RTree::RTree::stats_count_input_time_start(void)
{
   gettimeofday(&m_stats.input_time_start, NULL);   
}

inline void SpatialIndex::RTree::RTree::stats_count_output_time_start(void)
{
   gettimeofday(&m_stats.output_time_start, NULL);   
}

 inline void SpatialIndex::RTree::RTree::stats_count_total_time_start(void)
{
   gettimeofday(&m_stats.total_time_start, NULL);   
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_input_time_start(void)
{
   gettimeofday(&m_stats.partial_input_time_start, NULL);   
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_output_time_start(void)
{
   gettimeofday(&m_stats.partial_output_time_start, NULL);   
}

 inline void SpatialIndex::RTree::RTree::stats_count_partial_time_start(void)
{
   gettimeofday(&m_stats.partial_time_start, NULL);   
} 

inline void SpatialIndex::RTree::RTree::stats_count_input_time_end(void)
{
  gettimeofday(&m_stats.current_time, NULL);					
  m_stats.tS = m_stats.input_time_start.tv_sec*1000000 + (m_stats.input_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.total_input_time += (m_stats.tE -m_stats.tS); 
}

inline void SpatialIndex::RTree::RTree::stats_count_output_time_end(void)
{
  gettimeofday(&m_stats.current_time, NULL);					
  m_stats.tS = m_stats.output_time_start.tv_sec*1000000 + (m_stats.output_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.total_output_time += (m_stats.tE -m_stats.tS); 
}

inline void SpatialIndex::RTree::RTree::stats_count_total_time_end(void)
{
  gettimeofday(&(m_stats.current_time), NULL);					
  m_stats.tS = (m_stats.total_time_start).tv_sec*1000000 + (m_stats.total_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.total_time += (m_stats.tE -m_stats.tS); 
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_input_time_end(void)
{
  gettimeofday(&m_stats.current_time, NULL);					
  m_stats.tS = m_stats.partial_input_time_start.tv_sec*1000000 + (m_stats.partial_input_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.partial_input_time += m_stats.tE -m_stats.tS; 
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_output_time_end(void)
{
  gettimeofday(&m_stats.current_time, NULL);					
  m_stats.tS = m_stats.partial_output_time_start.tv_sec*1000000 + (m_stats.partial_output_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.partial_output_time += (m_stats.tE -m_stats.tS); 
}

inline void SpatialIndex::RTree::RTree::stats_count_partial_time_end(void)
{
  gettimeofday(&m_stats.current_time, NULL);					
  m_stats.tS = m_stats.partial_time_start.tv_sec*1000000 + (m_stats.partial_time_start.tv_usec); 
  m_stats.tE = m_stats.current_time.tv_sec*1000000 + (m_stats.current_time.tv_usec); 
  m_stats.partial_time += (m_stats.tE -m_stats.tS); 
}


inline void SpatialIndex::RTree::RTree::stats_update_idx_stats(void)
{
  m_stats.idx_total_time = m_stats.partial_time;
  m_stats.idx_total_input_time = m_stats.partial_input_time;  
  m_stats.idx_total_output_time = m_stats.partial_output_time;
  m_stats.idx_total_cpu_time = m_stats.partial_time -m_stats.partial_input_time - m_stats.partial_output_time;

  m_stats.idx_total_seq_input_count = m_stats.partial_seq_input_count;
  m_stats.idx_total_seq_output_count = m_stats.partial_seq_output_count;
  m_stats.idx_total_rand_input_count = m_stats.partial_rand_input_count;
  m_stats.idx_total_rand_output_count = m_stats.partial_rand_output_count;
  
}

inline void SpatialIndex::RTree::RTree::stats_update_query_stats(ts_type dist, int dataset_size)
{
     m_stats.query_total_time = m_stats.partial_time;
     m_stats.query_total_input_time = m_stats.partial_input_time;
     m_stats.query_total_output_time = m_stats.partial_output_time;
     m_stats.query_total_cpu_time = m_stats.partial_time -m_stats.partial_input_time - m_stats.partial_output_time;;

     m_stats.query_total_seq_input_count = m_stats.partial_seq_input_count;
     m_stats.query_total_seq_output_count = m_stats.partial_seq_output_count;
     m_stats.query_total_rand_input_count = m_stats.partial_rand_input_count;
     m_stats.query_total_rand_output_count = m_stats.partial_rand_output_count;
     m_stats.query_exact_distance = dist;

     m_stats.query_total_checked_ts_count = m_stats.checked_ts_count;
     m_stats.query_total_checked_nodes_count = m_stats.checked_nodes_count;

     m_stats.total_ts_count = dataset_size;
 
     m_stats.query_pruning_ratio = 1.0 - ((ts_type)m_stats.query_total_checked_ts_count/
					  m_stats.total_ts_count);
     
     
}

