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

// NOTE: Please read README.txt before browsing this code.
/** 
 Calculate paa.
 */
#define TS_SIZE 256

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
#include "flags.h"	   

// include library header file.
#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;
ts_type ts_euclidean_distance(ts_type * t, ts_type * s, int size) {
    ts_type distance = 0;
    while (size > 0) {
        size--;
        distance += (t[size] - s[size]) * (t[size] - s[size]);
    }
    distance = sqrtf(distance);
    return distance;
}
int paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment) {
    int s, i;
    for (s=0; s<segments; s++) {
        paa_out[s] = 0;
        for (i=0; i<ts_values_per_segment; i++) {
            paa_out[s] += ts_in[(s * ts_values_per_segment)+i];
        }
        paa_out[s] /= ts_values_per_segment;
    }
    return 1;
}

// example of a Visitor pattern.
// see RTreeQuery for a more elaborate example.
class MyVisitor : public IVisitor
{
private:
        std::ifstream *raf;
        ts_type *ts;
public:
        MyVisitor(std::ifstream& raf, ts_type *ts) {this->raf = &raf; this->ts = ts;}
	void visitNode(const INode& n) {}
    void visitData(byte *pData)
    {
      /*
        byte* pData = 0;
        uint32_t cLen = 0;
        d.getData(cLen, &pData);
      */
        ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
        ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
        //ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE, answer);
	

	//delete[] pData;
        //std::cout << d.getIdentifier() << "\t" << dist << "\t";
        // the ID of this data entry is an answer to the query. I will just print it to stdout.
    }

	void visitData(const IData& d)
	{
             byte* pData = 0;
	     uint32_t cLen = 0;
	     d.getData(cLen, &pData);
             ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
             ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
	  //   std::cout << d.getIdentifier() << "\t" << dist << "\t"; 
			// the ID of this data entry is an answer to the query. I will just print it to stdout.
	}

	void visitData(std::vector<const IData*>& v) {}
};

class DataSeriesNNComparator : public INearestNeighborComparator
{
private:
        std::ifstream *raf;
        ts_type *ts;
        ts_type mindist_ratio;
public:
        // RETURNS REAL DISTANCE
	ts_type getMinimumDistance(const SpatialIndex::IShape &q ,  const SpatialIndex::IData &d ) {
             byte* pData = 0;
	     uint32_t cLen = 0;
	     d.getData(cLen, &pData);
             ts_type *ts_leaf = reinterpret_cast<ts_type*>(pData);
             ts_type dist = ts_euclidean_distance(ts_leaf, this->ts, TS_SIZE);
             return dist;
	}
        // RETURNS LOWER BOUND
	ts_type getMinimumDistance(const SpatialIndex::IShape &q ,  const SpatialIndex::IShape &d) {
             return this->mindist_ratio * q.getMinimumDistance(d);
	}
        DataSeriesNNComparator(std::ifstream& raf, ts_type *ts, ts_type mindist_ratio) {
		this->raf = &raf;
                this->ts = ts;
                this->mindist_ratio = mindist_ratio;
        } 
};


int main(int argc, char** argv)
{
	try
	{
		if (argc != 7)
		{
			std::cerr << "Usage: " << argv[0] << " input_file tree_file capacity query_file paa_size loaded_data" << std::endl;
			return -1;	
		}

		uint32_t queryType = 0;

		std::ifstream fin(argv[1], std::ios::in | std::ios::binary);
		if (! fin)
		{
			std::cerr << "Cannot open data file " << argv[1] << "." << std::endl;
			return -1;
		}

		std::ifstream qin(argv[4], std::ios::in | std::ios::binary);
		if (! qin)
		{
			std::cerr << "Cannot open queries file " << argv[4] << "." << std::endl;
			return -1;
		}

		// Create a new storage manager with the provided base name and a 4K page size.
		std::string baseName = argv[2];
		IStorageManager* diskfile = StorageManager::createNewDiskStorageManager(baseName, 4096);
		StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false);
//                IStorageManager *file = StorageManager::createNewMemoryStorageManager();
			// applies a main memory random buffer on top of the persistent storage manager
			// (LRU buffer, etc can be created the same way).

		// Create a new, empty, RTree with dimensionality 2, minimum load 70%, using "file" as
		// the StorageManager and the RSTAR splitting policy.
                int dimensionality = atoi(argv[5]);
                int ts_size = TS_SIZE;
		id_type indexIdentifier;
		ISpatialIndex* tree = RTree::createNewRTree(*file, 0.7, atoi(argv[3]), atoi(argv[3]), dimensionality, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

		size_t count = 0;
		uint32_t op;

 		ts_type *ts = new ts_type[ts_size];
                ts_type *paa = new ts_type[dimensionality];
		while (fin)
		{
			if (! fin.good()) continue; // skip newlines, etc.


                        for(int i=0; i<ts_size; i++) { 
                            float val;
                            fin.read((char *) &val, sizeof(float));
                            ts[i] = (float) val;
			}

                        paa_from_ts(ts, paa, dimensionality, ts_size/dimensionality);

                        Point p = Point(paa, dimensionality);


			//os << p;
			//std::string data = os.str();
					// associate some data with this region. I will use a string that represents the
					// region itself, as an example.
					// NOTE: It is not necessary to associate any data here. A null pointer can be used. In that
					// case you should store the data externally. The index will provide the data IDs of
					// the answers to any query, which can be used to access the actual data from the external
					// storage (e.g. a hash table or a database table, etc.).
					// Storing the data in the index is convinient and in case a clustered storage manager is
					// provided (one that stores any node in consecutive pages) performance will improve substantially,
					// since disk accesses will be mostly sequential. On the other hand, the index will need to

			// manipulate the data, resulting in larger overhead. If you use a main memory storage manager,
					// storing the data externally is highly recommended (clustering has no effect).
					// A clustered storage manager is NOT provided yet.
					// Also you will have to take care of converting you data to and from binary format, since only
					// array of bytes can be inserted in the index (see RTree::Node::load and RTree::Node::store for
					// an example of how to do that).

 			tree->insertData(sizeof(ts_type) * TS_SIZE, reinterpret_cast<const byte*>(ts), p, count);
//                        tree->insertData(0,0,p,count);
  
			/*else if (op == QUERY)
			{
				plow[0] = x1; plow[1] = y1;
				phigh[0] = x2; phigh[1] = y2;

				MyVisitor vis;

				if (queryType == 0)
				{
					Region r = Region(plow, phigh, 2);
					tree->intersectsWithQuery(r, vis);
						// this will find all data that intersect with the query range.
				}
				else if (queryType == 1)
				{
					Point p = Point(plow, 2);
					tree->nearestNeighborQuery(10, p, vis);
						// this will find the 10 nearest neighbors.
				}
				else if(queryType == 2)
				{
					Region r = Region(plow, phigh, 2);
	tree->selfJoinQuery(r, vis);
				}
				else
				{
					Region r = Region(plow, phigh, 2);
					tree->containsWhatQuery(r, vis);
						// this will find all data that is contained by the query range.
				}
			}*/

			if ((count % 100000) == 0)
				std::cerr << count << std::endl;
                        if(count > atoi(argv[6])) break;

			count++;
		}

                fin.seekg(0);
                int qid = 0;
                int qmax = 100;

		while (qin)
		{
			if (! qin.good()) continue; // skip newlines, etc.

                        for(int i=0; i<ts_size; i++) { 
                            float val;
                            qin.read((char *) &val, sizeof(float));
                            ts[i] = (float) val;
			}

                        paa_from_ts(ts, paa, dimensionality, ts_size/dimensionality);

                        Point p = Point(paa, dimensionality);
        		struct timeval input_time_start, current_time;
                	gettimeofday(&input_time_start, NULL); 
                        MyVisitor vis(fin,ts);  
                        DataSeriesNNComparator cmp(fin,ts, sqrtf(ts_size/dimensionality));
 			tree->nearestNeighborQuery(1, p, vis, cmp);
                        gettimeofday(&current_time, NULL); 
                        ts_type tS = input_time_start.tv_sec*1000000 + (input_time_start.tv_usec); 
                        ts_type tE = current_time.tv_sec*1000000 + (current_time.tv_usec); 
                        std::cout << (tE - tS) << std::endl; 

                        qid++;    
                        if(qid >= qmax) break;
                }
		//std::ostringstream os;

                delete ts;
                delete paa;
		/*std::cerr << "Operations: " << count << std::endl;
		std::cerr << *tree;
		std::cerr << "Buffer hits: " << file->getHits() << std::endl;
		std::cerr << "Index ID: " << indexIdentifier << std::endl;

		bool ret = tree->isIndexValid();
		if (ret == false) std::cerr << "ERROR: Structure is invalid!" << std::endl;
		else std::cerr << "The stucture seems O.K." << std::endl;
                */
		delete tree;
		delete file;
		delete diskfile;
			// delete the buffer first, then the storage manager
			// (otherwise the the buffer will fail trying to write the dirty entries).
	}
	catch (Tools::Exception& e)
	{
		std::cerr << "******ERROR******" << std::endl;
		std::string s = e.what();
		std::cerr << s << std::endl;
		return -1;
	}

	return 0;
}
