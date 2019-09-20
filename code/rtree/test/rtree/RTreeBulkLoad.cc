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

/* Modified by Karima Echihabi (karima.echihabi@gmail.com)
 * to support PAA and gather detailed statistics
 */


// NOTE: Please read README.txt before browsing this code.
/**
  Calculate paa.
 */
#define TS_SIZE 256
//#define SUMMARIZATION_SIZE 8
//#define USE_PAA
//#define USE_FFT
//#define USE_DWT
//#DEFINE USE_SAX
#include "flags.h"
#include "stats.h"


#include <typeinfo>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <cmath>
#include <sys/time.h>
#include <spatialindex/SpatialIndex.h>


struct stats_info full_stats;


ts_type ts_euclidean_distance(ts_type * t, ts_type * s, int size)
{
    ts_type distance = 0;
    while (size > 0)
    {
        size--;
        distance += (t[size] - s[size]) * (t[size] - s[size]);
    }
    distance = sqrtf(distance);
 
    return distance;
}
#ifdef USE_PAA
int paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment)
{
    int s, i;
    for (s=0; s<segments; s++)
    {
        paa_out[s] = 0;
        for (i=0; i<ts_values_per_segment; i++)
        {
            paa_out[s] += ts_in[(s * ts_values_per_segment)+i];
        }
        paa_out[s] /= ts_values_per_segment;
    }
    return 1;
}
#endif


using namespace SpatialIndex;

ts_type answer = 1000000000;

class BinaryFileStream : public IDataStream
{
private:
    int records_read;
    int max_records;
public:
    BinaryFileStream(std::string inputFile, int max_records) : m_pNext(0)
    {
        this->records_read = 0;
        this->max_records = max_records;
        m_fin.open(inputFile.c_str());

        if (!m_fin)
            throw Tools::IllegalArgumentException("Input file not found.");

        readNextEntry();
    }

    virtual ~BinaryFileStream()
    {
        if (m_pNext != 0) delete m_pNext;
    }

    virtual IData* getNext()
    {
        if (m_pNext == 0) return 0;

        RTree::Data* ret = m_pNext;
        m_pNext = 0;
        readNextEntry();
        return ret;
    }

    virtual bool hasNext()
    {
        return (m_pNext != 0);
    }

    virtual uint32_t size()
    {
        throw Tools::NotSupportedException("Operation not supported.");
    }

    virtual void rewind()
    {
        this->records_read = 0;
        if (m_pNext != 0)
        {
            delete m_pNext;
            m_pNext = 0;
        }

        m_fin.seekg(0, std::ios::beg);
        readNextEntry();
    }

    void readNextEntry()
    {
        id_type id;
        uint32_t op;
        
        ts_type ts[TS_SIZE];
        ts_type summarization[TS_SIZE];
        
        if (this->records_read < this->max_records)
        {
            for(int i=0; i<TS_SIZE; i++)
            {
                float val;
                m_fin.read((char *) &val, sizeof(float));
                ts[i] = (ts_type) val;
            }

#ifdef USE_PAA
            paa_from_ts(ts, summarization, SUMMARIZATION_SIZE, TS_SIZE/SUMMARIZATION_SIZE);
#endif // USE_PAA

#ifdef USE_FFT
            fft_from_ts(ts, summarization, TS_SIZE, 1);
#endif // USE_FFT
            
#ifdef USE_DWT
            dwt_from_ts(ts, summarization, TS_SIZE);
#endif // USE_DWT

            this->records_read++;
            Region r(summarization, summarization, SUMMARIZATION_SIZE);
            m_pNext = new RTree::Data(sizeof(ts_type)*TS_SIZE, reinterpret_cast<byte*>(ts), r, id);
        }
    }

    std::ifstream m_fin;
    RTree::Data* m_pNext;
};

// example of a Visitor pattern.
// see RTreeQuery for a more elaborate example.
class MyVisitor : public IVisitor
{
private:
    ts_type *ts;
public:
    MyVisitor(ts_type *ts)
    {
        this->ts = ts;
    }
    void visitNode(const INode& n) {
        
    }

    void visitData(byte *pData)
    {
        ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
        ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
        //ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE, answer);
	
        if(answer > dist) {
            answer = dist;
        }
    }
  
    void visitData(const IData& d)
    {
        byte* pData = 0;
        uint32_t cLen = 0;
        d.getData(cLen, &pData);
        ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
        ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
	
        if(answer > dist) {
            answer = dist;
        }
	delete[] pData;
    }

    void visitData(std::vector<const IData*>& v) {}
};

class DataSeriesNNComparator : public INearestNeighborComparator
{
private:
    ts_type *ts;
    ts_type *summarization;
    ts_type mindist_ratio;
public:
    // RETURNS REAL DISTANCE
    ts_type getMinimumDistance(const SpatialIndex::IShape &q ,  const SpatialIndex::IData &d )
    {
        byte* pData = 0;
        uint32_t cLen = 0;
        d.getData(cLen, &pData);
        ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
        ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
        return dist;
    }
    // RETURNS LOWER BOUND
    ts_type getMinimumDistance(const SpatialIndex::IShape &q ,  const SpatialIndex::IShape &d)
    {
        #ifdef USE_PAA
        return this->mindist_ratio * q.getMinimumDistance(d);
        #endif // USE_PAA
        #ifdef USE_FFT
        return sqrt(2)/sqrt(TS_SIZE) * q.getMinimumDistance(d);
        #endif // USE_FFT
        #ifdef USE_DWT
        return q.getMinimumDistance(d);
        #endif
    }
    DataSeriesNNComparator( ts_type *ts, ts_type *summarization, ts_type mindist_ratio)
    {
        this->ts = ts;
        this->summarization = summarization;
        this->mindist_ratio = mindist_ratio;
    }
};


int main(int argc, char** argv)
{
#ifdef USE_PAA
  //std::cerr << "USING PAA." << std::endl;
#endif
#ifdef USE_FFT
    std::cerr << "USING FFT." << std::endl;
#endif
#ifdef USE_DWT
    std::cerr << "USING DWT." << std::endl;
#endif

 
  stats_init();
  stats_count_total_time_start(); 

  bool print_leaves= false;
    try
    {
      /*
        if (argc != 5)
        {
            std::cerr << "Usage: " << argv[0] << " input_file index_path dataset_size capacity mode [query:1|i:0]" << std::endl;
            std::cerr << "Usage: " << argv[0] << " query_file index_path queries dataset_size mode [query:1|i:0]" << std::endl;
            return -1;
	    }
       */
      
        if(argc == 6 && atoi(argv[5]) == 0)
        {
            stats_reset_partial_counters();
            stats_count_partial_time_start();

            stats_count_partial_rand_input();
            stats_count_partial_input_time_start();
	  
            //std::ifstream fin(argv[1], std::ios::ate | std::ios::binary);
            //int act_size = fin.tellg() / (sizeof(float) * TS_SIZE);
            int size = atoi(argv[3]);

            stats_count_partial_input_time_end();
	    
            //std::cerr << "FILE SIZE: " << size << std::endl;
            BinaryFileStream stream(argv[1], size);

	    std::string index_filename = "/index";
            std::string baseName = argv[2] + index_filename;
	    std::string fullname = baseName + ".idx";

            IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
            StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 2441406, false);
            //StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 5000000000, false);
            id_type indexIdentifier;

	    /*
            ISpatialIndex* tree = RTree::createAndBulkLoadNewRTree(
                                      RTree::BLM_STR, stream, *file, 0.7, atoi(argv[4]), atoi(argv[4]), SUMMARIZATION_SIZE, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);
	    */

            ISpatialIndex* tree = RTree::createAndBulkLoadNewRTree(
								   RTree::BLM_STR, argv[1], atoi(argv[3]),*file, 0.7, atoi(argv[4]), atoi(argv[4]), SUMMARIZATION_SIZE, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);
	    
	    tree->stats_update_idx_stats();	    
	    tree->print_index_stats(argv[1], fullname.c_str(), print_leaves);

            //fin.close();	      
            delete tree;
            delete file;
            delete diskfile;

	    //stats_count_partial_time_end();	    
            //stats_update_preprocessing_stats();
            //print_preprocessing_stats(full_stats, argv[1]);  	    
        }
	
        else if(argc == 6 && atoi(argv[5]) == 1)
        {
	  
            std::ifstream qin(argv[1], std::ios::in | std::ios::binary);
            if (! qin)
            {
                std::cerr << "Cannot open queries file " << argv[4] << "." << std::endl;
                return -1;
            }

	    std::string index_filename = "/index";
            std::string baseName = argv[2] + index_filename;
	    std::string fullname = baseName + ".idx";

            IStorageManager* diskfile = StorageManager::loadDiskStorageManager(baseName);
            StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 2441406, false);
            ISpatialIndex* tree = RTree::loadRTree(*file, 1);

    
	    cout << "index file loaded" << endl;
 	    //tree->stats_init();

	    
	    print_leaves = true;
            tree->stats_init_leaf_counters();
            tree->stats_get_leaf_counters();
	    tree->print_index_stats(argv[1], fullname.c_str(), print_leaves);
	    
            tree->stats_reset_partial_counters();
 	    tree->stats_count_partial_time_start();
	    
            ts_type ts[TS_SIZE];
            ts_type summarization[SUMMARIZATION_SIZE];
            int qmax = atoi(argv[3]);
            unsigned int qid = 0;
            while (qin)
            { 	 
                tree->stats_reset_query_counters();

                answer = 1000000000;
                if (! qin.good()) continue; // skip newlines, etc.

                for(int i=0; i<TS_SIZE; i++)
                {
                    float val;
                    qin.read((char *) &val, sizeof(float));
                    ts[i] = (float) val;
                }

                #ifdef USE_PAA
                paa_from_ts(ts, summarization, SUMMARIZATION_SIZE, TS_SIZE/SUMMARIZATION_SIZE);
                #endif // USE_PAA
                #ifdef USE_FFT
                fft_from_ts(ts, summarization, TS_SIZE, 1);
                #endif // USE_FFT
                #ifdef USE_DWT
                dwt_from_ts(ts, summarization, TS_SIZE);
                #endif // USE_DWT

                Point p = Point(summarization, SUMMARIZATION_SIZE);
                //struct timeval input_time_start, current_time;
                //gettimeofday(&input_time_start, NULL);
                MyVisitor vis(ts);
                DataSeriesNNComparator cmp(ts, summarization, sqrtf(TS_SIZE/SUMMARIZATION_SIZE));
                //DataSeriesNNComparator cmp(ts, summarization, TS_SIZE/SUMMARIZATION_SIZE);
		
                tree->nearestNeighborQueryOptimized(1, p, vis, cmp);
                tree->stats_count_partial_time_end();

                qid++;
                tree->stats_update_query_stats(answer, atoi(argv[4]));
		
                tree->print_query_stats(argv[1], qid);
           	tree->stats_reset_partial_counters();
                tree->stats_count_partial_time_start();
                if(qid >= qmax) break;
            }
            tree->stats_count_partial_time_end();
            tree->stats_clean_leaf_counters();

        }
        else if(argc == 8 && atoi(argv[7]) == 2)
        {
            stats_reset_partial_counters();
            stats_count_partial_time_start();

            stats_count_partial_rand_input();
            stats_count_partial_input_time_start();
	  
            int dataset_size = atoi(argv[2]);

            stats_count_partial_input_time_end();
	    
            BinaryFileStream stream(argv[1], dataset_size);

	    std::string index_filename = "/index";
            std::string baseName = argv[5] + index_filename;
	    std::string fullname = baseName + ".idx";

            IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
            StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 2441406, false);
;
            id_type indexIdentifier;

            ISpatialIndex* tree = RTree::createAndBulkLoadNewRTree(RTree::BLM_STR, argv[1], atoi(argv[2]),*file,
								   0.7, atoi(argv[6]), atoi(argv[6]), SUMMARIZATION_SIZE,
								   SpatialIndex::RTree::RV_RSTAR, indexIdentifier);
	    
	    tree->stats_update_idx_stats();	    
	    tree->print_index_stats(argv[1], fullname.c_str(), print_leaves);

	    print_leaves = true;
            tree->stats_init_leaf_counters();
            tree->stats_get_leaf_counters();
	    tree->print_index_stats(argv[1], fullname.c_str(), print_leaves);
	    
            tree->stats_reset_partial_counters();
 	    tree->stats_count_partial_time_start();
	    
            ts_type ts[TS_SIZE];
            ts_type summarization[SUMMARIZATION_SIZE];
            int qmax = atoi(argv[4]);
            unsigned int qid = 0;

            std::ifstream qin(argv[3], std::ios::in | std::ios::binary);
	    
            while (qin)
            { 	 
                tree->stats_reset_query_counters();

                answer = 1000000000;
                if (! qin.good()) continue; // skip newlines, etc.

                for(int i=0; i<TS_SIZE; i++)
                {
                    float val;
                    qin.read((char *) &val, sizeof(float));
                    ts[i] = (float) val;
                }

                #ifdef USE_PAA
                paa_from_ts(ts, summarization, SUMMARIZATION_SIZE, TS_SIZE/SUMMARIZATION_SIZE);
                #endif // USE_PAA
                #ifdef USE_FFT
                fft_from_ts(ts, summarization, TS_SIZE, 1);
                #endif // USE_FFT
                #ifdef USE_DWT
                dwt_from_ts(ts, summarization, TS_SIZE);
                #endif // USE_DWT

                Point p = Point(summarization, SUMMARIZATION_SIZE);
                MyVisitor vis(ts);
                DataSeriesNNComparator cmp(ts, summarization, sqrtf(TS_SIZE/SUMMARIZATION_SIZE));
		
                tree->nearestNeighborQuery(1, p, vis, cmp);
                tree->stats_count_partial_time_end();
                qid++;
                tree->stats_update_query_stats(answer, atoi(argv[3]));      
		
                tree->print_query_stats(argv[1], qid);
           	tree->stats_reset_partial_counters();
                tree->stats_count_partial_time_start();
                if(qid >= qmax) break;
            }
            tree->stats_count_partial_time_end();
            tree->stats_clean_leaf_counters();
	    
            delete tree;
            delete file;
            delete diskfile;

        }
	
	else
	{
            std::cerr << "Usage: " << argv[0] << " input_file index_directory  dataset_size capacity mode [query:1|i:0]" << std::endl;
            std::cerr << "Usage: " << argv[0] << " query_file index_directory queries_size dataset_size mode [query:1|i:0]" << std::endl;
            std::cerr << "Usage: " << argv[0] << " input_file dataset_size query_file queries_size index_directory capacity mode [index and query:2|query:1|i:0]" << std::endl;
	    return -1;
	}

	
    }
    catch (Tools::Exception& e)
    {
        std::cerr << "******ERROR******" << std::endl;
        std::string s = e.what();
        std::cerr << s << std::endl;
        return -1;
    }

    stats_count_total_time_end();
    fprintf(stderr,"Sanity check: combined times should be less than: %f secs \n",
      	   full_stats.counters.total_time/1000000);
    
    return 0;
}

void print_query_stats(struct stats_info full_stats, unsigned int query_num, char * queries)
{
	
        printf("Query_total_input_time_secs\t%lf\t%s\t%u\n",
	       full_stats.query_total_input_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_output_time_secs\t%lf\t%s\t%u\n",
	       full_stats.query_total_output_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_cpu_time_secs\t%lf\t%s\t%u\n",
	       full_stats.query_total_cpu_time/1000000,	       
	       queries,
	       query_num
	     );
	
        printf("Query_total_time_secs\t%lf\t%s\t%u\n",
	       full_stats.query_total_time/1000000,	       
	       queries,
	       query_num
	     );

        printf("Query_total_seq_input_count\t%llu\t%s\t%d\n",
	       full_stats.query_total_seq_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_seq_output_count\t%llu\t%s\t%d\n",
	       full_stats.query_total_seq_output_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_input_count\t%llu\t%s\t%d\n",
	       full_stats.query_total_rand_input_count,
	       queries,
	       query_num
	       );
	
        printf("Query_total_rand_output_count\t%llu\t%s\t%d\n",
	       full_stats.query_total_rand_output_count,
	       queries,
	       query_num
	       );

        printf("Query_exact_distance\t%f\t%s\t%d\n",
	       full_stats.query_exact_distance,
	       queries,
	       query_num
	       );

	double pruning_ratio = 1.0 - (double)full_stats.query_accessed_bytes/full_stats.dataset_byte_size;
	
        printf("Query_pruning_ratio_level\t%f\t%s\t%d\n",
	       pruning_ratio,
	       queries,
	       query_num
	       );
}

void print_preprocessing_stats(struct stats_info stats, char * dataset)
{
        //  id = -1 for index and id = query_id for queries
        int id = -1;
 	
        printf("Preprocessing_total_input_time_secs\t%lf\t%s\t%d\n",
	       full_stats.preprocessing_total_input_time/1000000,
	       dataset,
	       id
	     ); 
        printf("Preprocessing_total_output_time_secs\t%lf\t%s\t%d\n",
	       full_stats.preprocessing_total_output_time/1000000,
	       dataset,
	       id);
        printf("Preprocessing_total_cpu_time_secs\t%lf\t%s\t%d\n",
	       full_stats.preprocessing_total_cpu_time/1000000,
	       dataset,
	       id);	
        printf("Preprocessing_total_time_secs\t%lf\t%s\t%d\n",
	       full_stats.preprocessing_total_time/1000000,
	       dataset,
	       id);
	
        printf("Preprocessing_total_seq_input_count\t%llu\t%s\t%d\n",
	       full_stats.preprocessing_total_seq_input_count,
	       dataset,
	       id);
	
        printf("Preprocessing_total_seq_output_count\t%llu\t%s\t%d\n",
	       full_stats.preprocessing_total_seq_output_count,
	       dataset,
	       id);
	
        printf("Preprocessing_total_rand_input_count\t%llu\t%s\t%d\n",
	       full_stats.preprocessing_total_rand_input_count,
	       dataset,
	       id);
	
        printf("Preprocessing_total_rand_output_count\t%llu\t%s\t%d\n",
	       full_stats.preprocessing_total_rand_output_count,
	       dataset,
	       id); 

	double size_MB =  (full_stats.program_byte_size)*1.0/(1024*1024);

	printf("Index_size_MB\t%lf\t%s\t%d\n",
	       size_MB,
	       dataset,
	       id);

	printf("Total_ts_count\t%u\t%s\t%d\n",
	       full_stats.total_ts_count,	       
	       dataset,
	       id);				

}
