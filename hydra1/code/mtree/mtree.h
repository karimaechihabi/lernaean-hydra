#ifndef MTREE_H
#define MTREE_H


#include "common.h"
#include "dataset.h"
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include <boost/math/distributions/skew_normal.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>


#include "boost/heap/priority_queue.hpp"


using namespace std;
using namespace boost;


struct Vertex{
    TS pivot; // pivot series
    ts_type radius; // max distance between pivot and all UTS below this node.
    int count; // how many UTS is this node representing?

    int depth; // depth : if leaf => 0, if root => max depth value

    // to be cleaned before next query:
    bool dropped; // dropped node candidate

    ts_type dist_min; // minimum distance to query in sub-tree
    ts_type dist_max; // maximum distance to query in sub-tree
    ts_type dist; // distance between pivot and query



    ts_type dist_parent; // distance to parent node

    // if 'count' is 1, then leaves[vertexid] is the index of the UTS in dataset.
};

struct Edge{
    int x;
    int src;
    int dst;
};


// http://stackoverflow.com/questions/5056520/algorithm-for-selecting-all-edges-and-vertices-connected-to-one-vertex
// http://www.boost.org/doc/libs/1_50_0/libs/graph/doc/quick_tour.html
// http://www.boost.org/doc/libs/1_50_0/libs/graph/doc/graph_concepts.html
// http://www.boost.org/doc/libs/1_50_0/libs/graph/doc/adjacency_list.html
// http://www.boost.org/doc/libs/1_50_0/libs/graph/doc/using_adjacency_list.html


typedef boost::adjacency_list<  // adjacency_list is a template depending on :
boost::listS,               //  The container used for egdes : here, std::list.
boost::vecS,                //  The container used for vertices: here, std::vector.
boost::directedS,           //  directed or undirected edges ?.
Vertex,                     //  The type that describes a Vertex.
Edge                        //  The type that describes an Edge
> MyGraph;

/* From http://www.boost.org/doc/libs/1_47_0/libs/graph/doc/adjacency_list.html
In general, if you want your vertex and edge descriptors to be stable (never invalidated)
then use listS or setS for the VertexList and OutEdgeList template parameters of adjacency_list.
If you are not as concerned about descriptor and iterator stability, and are more concerned about
memory consumption and graph traversal speed, use vecS for the VertexList and/or OutEdgeList
template parameters. */


typedef MyGraph::vertex_descriptor VertexID;
typedef MyGraph::edge_descriptor   EdgeID;



struct minDistMaxNodeOrder
{

    minDistMaxNodeOrder(MyGraph *_g) { this->g = _g; };
    bool operator() (const VertexID & ID1, const VertexID & ID2) const
    {
        return ((Vertex)((*g)[ID1])).dist_max > ((Vertex)((*g)[ID2])).dist_max;
    }

    MyGraph *g;

};


typedef boost::heap::priority_queue<VertexID, boost::heap::compare<minDistMaxNodeOrder> > myPriorityQueue;



class MTREE
{
public:
    MTREE();

    VertexID root;
    map<VertexID,int> leaves;
    MyGraph g;

    vector<VertexID> kdist_candidates;
    vector<VertexID> kdist_lb_candidates;

    Edge &addChild(VertexID fatherID, VertexID childID);
    VertexID insertNode(int uts_i);
    void create_leaves(const Dataset &dataset, vector<VertexID> &active_nodes);
    void load(const Dataset &dataset, ts_type ratio);
    void load(const Dataset &dataset, int num_samples);
    void load_UTS(UTS &dataset, ts_type ratio);
    pair<int,ts_type> find_nearest(int node_i, int *samples, int n_samples, vector<VertexID> &active_nodes);
    void print();
    pair< vector<int> , vector< pair<ts_type,ts_type> > > find_topk(const TS &q, int K);
    void update_kdist(int K, ts_type &kdist);
    void update_kdist_lb(int K, ts_type &kdist_lb);

    int internal_pruned_count;
    void clean_vector(vector<VertexID> &V, ts_type kdist);
    void clean_vector_lb(vector<VertexID> &V, ts_type kdist_lb);

    vector<int> range_query(TS &q, ts_type dist_max);


};

#endif // MTREE_H
