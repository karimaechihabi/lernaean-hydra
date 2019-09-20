#include "mtree.h"
#include "stats.h"


MTREE::MTREE()
{

}


void MTREE::load_UTS(UTS &uts,ts_type ratio) {

    Dataset dataset(uts);
    //LOG << "dataset N= " << dataset.N << "\n";
    load(dataset, ratio);

}


void MTREE::load(const Dataset &dataset,ts_type ratio) {

    vector<VertexID> active_nodes;

    //LOG << "Creating the leaves ....\n";
    create_leaves(dataset, active_nodes);
    assert((int)active_nodes.size() == dataset.N);


  //  LOG << "active_nodes= " << active_nodes.size() << "\n";

    while(active_nodes.size() > 1) {


        vector<VertexID> active_nodes_next;

        int N = active_nodes.size();

        // step1: sampling of nodes.
        //LOG << "Sampling of nodes ....\n";
        boost::variate_generator< boost::mt19937, boost::uniform_int<> >
                randomly(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0, N-1));

        int n_samples = ((double)(N)) * ratio;
        
        //LOG << "Loading... N= " << N << " n_samples= " << n_samples << "\n";
        if(n_samples == 0 || n_samples == N) { // we create one single root node.
            n_samples = 1;
        }

        //LOG << "#MTREE LOAD nActiveNodes== " << active_nodes.size() << " nSamples= " << n_samples << "\n" ;

        //LOG << "n_samples= " << n_samples << "\n";

        int samples[n_samples];

        //bool selected[N];
	bool * selected = new bool[N];
        memset(selected, false, sizeof(bool)*N);
        for(int sample_i = 0 ; sample_i < n_samples ; sample_i++) {
            while(true) {
                int candidate_i = randomly();
                if(!selected[candidate_i]) {
                    selected[candidate_i] = true;
                    samples[sample_i] = candidate_i;
                    break;
                }
            }
        }

        //        LOG << "selected samples: ";
        //      for(int sample_i = 0 ; sample_i < n_samples ; sample_i++)
        //        cout << samples[sample_i] << " ";
        //    cout << "\n";

        // step2: associate all non-sampled nodes to its "closest" sampled node.

        vector < pair<int,ts_type> > clusters[n_samples];


        for(int node_i = 0 ; node_i < N; node_i++) {
            if(selected[node_i]) // selected as sample! skip it.
                continue;
            pair<int,ts_type> nearest = find_nearest(node_i, samples, n_samples, active_nodes); // samples = set of possible fathers...
         //   LOG << "nearest.first= " << nearest.first << "\n";
            clusters[nearest.first].push_back(make_pair(node_i, nearest.second));
        }


        // step3: add supernodes and samples
        for(int sample_i = 0 ; sample_i < n_samples; sample_i++) {

            VertexID fatherID = -1;

            if(clusters[sample_i].size() > 0) { // we insert a new node as father.
                fatherID = insertNode(-1);
                Vertex &father = g[fatherID];
                father.count = 0;
                father.radius = 0;
                father.pivot = g[(active_nodes[samples[sample_i]])].pivot;

               ts_type radius = 0;

                father.depth = g[(active_nodes[samples[sample_i]])].depth+1;

                clusters[sample_i].push_back(make_pair(samples[sample_i], 0));// add also the sampled point!

                // this loop updates 'radius' and 'count' statistics in the new father node.
                for(unsigned int child_i = 0 ; child_i < clusters[sample_i].size(); child_i++) {
                    VertexID childID = active_nodes[ (clusters[sample_i][child_i].first)];
                    Vertex &child = g[childID];
                    //        LOG << father.radius << " "<< child.radius  << " "<<clusters[sample_i][child_i].second << "\n";
                   ts_type radius_i = father.radius + child.radius + clusters[sample_i][child_i].second;
                    if(radius_i > radius)
                        radius = radius_i;
                    father.count+= child.count;
                    child.dist_parent = clusters[sample_i][child_i].second;
                    addChild( fatherID ,childID);
                }

                father.radius = radius;

                // LOG << "radius " << radius << "\n";
            } else {
                // we use the leaf as it is, to be appended to the root node in the next iterations
                // (it may become the root node.).

                fatherID =active_nodes[samples[sample_i]];
            //    LOG << "xxxx n_samples= " << n_samples << " active_nodes_size= " << active_nodes.size() << " candidate_i= " << samples[sample_i] << " fatherID= " << fatherID << "\n";

            }

            //LOG << "fatherID= " << fatherID << "\n";

            // add the fatherID to the next level of nodes...
            active_nodes_next.push_back(fatherID);
        }

        // prepare to the next level of nodes .
        active_nodes = active_nodes_next;
        delete [] selected;
    }

    assert(active_nodes.size() == 1);

    root = active_nodes[0];
    g[root].dist_parent = 0;

    //LOG << "ROOT is node ID=" << active_nodes[0] << " , max_depth= " << g[root].depth << "\n";


    //  print();
}

void MTREE::load(const Dataset &dataset,int n_samples) {

    vector<VertexID> active_nodes;

    fprintf(stderr,"Creating the leaves ....\n" );
    //LOG << "Creating the leaves ....\n";
    create_leaves(dataset, active_nodes);
    assert((int)active_nodes.size() == dataset.N);

    fprintf(stderr,"active_nodes = %d\n", active_nodes.size()  );   
  //  LOG << "active_nodes= " << active_nodes.size() << "\n";

    while(active_nodes.size() > 1) {


        vector<VertexID> active_nodes_next;

        int N = active_nodes.size();

        // step1: sampling of nodes.
        //LOG << "Sampling of nodes ....\n";
        boost::variate_generator< boost::mt19937, boost::uniform_int<> >
                randomly(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0, N-1));

        //int n_samples = ((double)(N)) * ratio;
        
        //LOG << "Loading... N= " << N << " n_samples= " << n_samples << "\n";
        if(n_samples == 0 || n_samples == N) { // we create one single root node.
            n_samples = 1;
        }

        //LOG << "#MTREE LOAD nActiveNodes== " << active_nodes.size() << " nSamples= " << n_samples << "\n" ;
        fprintf(stderr,"n_samples = %d\n", n_samples );   
        //LOG << "n_samples= " << n_samples << "\n";

        int samples[n_samples];

        //bool selected[N];
	bool * selected = new bool[N];
        memset(selected, false, sizeof(bool)*N);
        for(int sample_i = 0 ; sample_i < n_samples ; sample_i++) {
            while(true) {
                int candidate_i = randomly();
                if(!selected[candidate_i]) {
                    selected[candidate_i] = true;
                    samples[sample_i] = candidate_i;
                    break;
                }
            }
        }

        //        LOG << "selected samples: ";
        //      for(int sample_i = 0 ; sample_i < n_samples ; sample_i++)
        //        cout << samples[sample_i] << " ";
        //    cout << "\n";

        // step2: associate all non-sampled nodes to its "closest" sampled node.

        vector < pair<int,ts_type> > clusters[n_samples];


        for(int node_i = 0 ; node_i < N; node_i++) {
            if(selected[node_i]) // selected as sample! skip it.
                continue;
            pair<int,ts_type> nearest = find_nearest(node_i, samples, n_samples, active_nodes); // samples = set of possible fathers...
         //   LOG << "nearest.first= " << nearest.first << "\n";
            clusters[nearest.first].push_back(make_pair(node_i, nearest.second));
        }


        // step3: add supernodes and samples
        for(int sample_i = 0 ; sample_i < n_samples; sample_i++) {

            VertexID fatherID = -1;

            if(clusters[sample_i].size() > 0) { // we insert a new node as father.
                fatherID = insertNode(-1);
                Vertex &father = g[fatherID];
                father.count = 0;
                father.radius = 0;
                father.pivot = g[(active_nodes[samples[sample_i]])].pivot;

               ts_type radius = 0;

                father.depth = g[(active_nodes[samples[sample_i]])].depth+1;

                clusters[sample_i].push_back(make_pair(samples[sample_i], 0));// add also the sampled point!

                // this loop updates 'radius' and 'count' statistics in the new father node.
                for(unsigned int child_i = 0 ; child_i < clusters[sample_i].size(); child_i++) {
                    VertexID childID = active_nodes[ (clusters[sample_i][child_i].first)];
                    Vertex &child = g[childID];
                    //        LOG << father.radius << " "<< child.radius  << " "<<clusters[sample_i][child_i].second << "\n";
                   ts_type radius_i = father.radius + child.radius + clusters[sample_i][child_i].second;
                    if(radius_i > radius)
                        radius = radius_i;
                    father.count+= child.count;
                    child.dist_parent = clusters[sample_i][child_i].second;
                    addChild( fatherID ,childID);
                }

                father.radius = radius;

                // LOG << "radius " << radius << "\n";
            } else {
                // we use the leaf as it is, to be appended to the root node in the next iterations
                // (it may become the root node.).

                fatherID =active_nodes[samples[sample_i]];
            //    LOG << "xxxx n_samples= " << n_samples << " active_nodes_size= " << active_nodes.size() << " candidate_i= " << samples[sample_i] << " fatherID= " << fatherID << "\n";

            }

            //LOG << "fatherID= " << fatherID << "\n";

            // add the fatherID to the next level of nodes...
            active_nodes_next.push_back(fatherID);
        }

        // prepare to the next level of nodes .
        active_nodes = active_nodes_next;
        delete [] selected;
    }

    assert(active_nodes.size() == 1);

    root = active_nodes[0];
    g[root].dist_parent = 0;

    //LOG << "ROOT is node ID=" << active_nodes[0] << " , max_depth= " << g[root].depth << "\n";


    //  print();
}


pair<int,ts_type> MTREE::find_nearest(int node_i, int *samples, int n_samples, vector<VertexID> &active_nodes) {

   ts_type min_dist = 1e20;
    int nearest_i = -1;

    VertexID childID= active_nodes[node_i];
    const TS &child_pivot = ((const Vertex &)g[childID]).pivot;

    int s = MAX(child_pivot.values.size() * 0.1,1);

    for(int sample_i = 0 ; sample_i < n_samples; sample_i++) {
        VertexID fatherID= active_nodes[ samples[sample_i]];
        const TS &father_pivot = ((const Vertex &)g[fatherID]).pivot;
        //double dist = child_pivot.dist2(father_pivot, min_dist); // dist2 ! no sqrt, so min_dist doesnt need POW2.
       ts_type dist = child_pivot.dist2cut(father_pivot, min_dist, s); // dist2 ! no sqrt, so min_dist doesnt need POW2.
        if(dist <= min_dist) {
            min_dist = dist;
            nearest_i = sample_i;
        }

    }

    // compute EXACT distance ... without dist2cut .

    VertexID fatherID= active_nodes[ samples[nearest_i]];
    const TS &father_pivot = ((const Vertex &)g[fatherID]).pivot;
    min_dist = child_pivot.dist(father_pivot);

    //  LOG << "find_nearest: " << make_pair(nearest_i, min_dist) << "\n";
    return make_pair(nearest_i, min_dist);

}



Edge & MTREE::addChild(VertexID fatherID, VertexID childID) {

    EdgeID eID;
    bool ok;

    //    LOG << "Adding edge " << fatherID << " <- " << childID << "\n";

    boost::tie(eID, ok) = boost::add_edge(fatherID, childID, g);
    assert(ok);

    Edge &edge =g[eID];

    return edge;
}


VertexID MTREE::insertNode(int uts_i) {

    VertexID vID = add_vertex(g); // vID is the index of a new Vertex
    Vertex &node = g[vID];

    node.dist_max = -1;
    node.dist_min = -1;
    node.dist = -1;
    node.dist_parent = -1;
    node.dropped = false;
    node.depth = 0;

    return vID;
}


void MTREE::create_leaves(const Dataset &dataset, vector<VertexID> &active_nodes) {
    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        VertexID nodeID = insertNode(uts_i);
        Vertex &node = g[nodeID];

        leaves[nodeID] = uts_i; // set corresponding uts
        active_nodes.push_back(nodeID); // add to list of active nodes

        node.pivot = ((const UTS &)dataset.utss[uts_i]).avg();
        vector<ts_type> dists = ((const UTS &)dataset.utss[uts_i]).dist(node.pivot);
        node.radius = *max_element(dists.begin(), dists.end());
        //     LOG <<  "radius= " << node.radius << "\n";
        node.count = 1;
    }
}




void MTREE::print() {

    LOG << "Nodes:\n";

    MyGraph::vertex_iterator vertexIt, vertexEnd;
    boost::tie(vertexIt, vertexEnd) = vertices(g);
    for (; vertexIt != vertexEnd; ++vertexIt){
        VertexID vertexID = *vertexIt; // dereference vertexIt, get the ID
        Vertex & node = g[vertexID];
        LOG << "Node ID=" << vertexID << " count=" << node.count << " radius=" << node.radius  ;
        if(node.count == 1) {
            cout <<  "(leaf, uts_i= " << leaves[vertexID] << ")\n";
        } else {
            cout << "(internal node)\n";
        }
    }


    MyGraph::edge_iterator edgeIt, edgeEnd;
    boost::tie(edgeIt, edgeEnd) = edges(g);

    LOG << "Edges:\n";

    for (; edgeIt != edgeEnd; ++edgeIt){
        EdgeID edgeID = *edgeIt;
        //        Edge & edge = g[edgeID];

        VertexID sourceID = source(edgeID, g);
        VertexID targetID = target(edgeID, g);

        LOG << sourceID << " <- " << targetID << "\n";

        //Vertex &node1 = g[nodeIds[edge.node1]];
        //Vertex &node2 = g[nodeIds[edge.node2]];
        //cout << "Edge " << node1.id << " -> " << node2.id << " P=" << edge.P << "\n";
    }

    //    custom_bfs_visitor vis;
    //   breadth_first_search(graph, vertex(0, graph), visitor(vis));

}



pair< vector<int> , vector< pair<ts_type,ts_type> > > MTREE::find_topk(const TS &q, int K) {

    myPriorityQueue pq(&g);


    assert(kdist_candidates.size() == 0);

    vector<VertexID> result_set;

    // init root...
    Vertex &root_node = g[root];
   ts_type dist = q.dist(root_node.pivot);
    root_node.dist_min = MAX( dist - root_node.radius, 0);
    root_node.dist_max = dist + root_node.radius;
    root_node.dist = dist;
    pq.push(root);

    kdist_candidates.push_back(root);

    // done.

    // init kdist...

   ts_type kdist;

    //   ts_type kdist_lb = 0; // if node.dist_lb < kdist_lb, then the node subtree is part of the result-set.


    if(K == 1) {
        kdist = root_node.dist_max; //xxx why this?
    } else {
        kdist = 1e20;
    }

    // done.

    int loops_count = 0;
    internal_pruned_count = 0;
    unsigned long long checked_ts = 0;
    
    while(!pq.empty()) {

        loops_count++;

        VertexID nodeID = *(pq.begin());
        pq.pop();
        Vertex &node = g[nodeID];

        //LOG << "PQ NodeID= " << nodeID << " " << node.dropped << "\n";

        if(node.dropped) // already marked as pruned during update_kdist! pruned!
            continue;

        assert(node.dist_min != -1);
        assert(node.dist_max != -1);

        if(node.count > 1) {  // internal node...

            //LOG << "nodecount= " << node.count << "\n";

            if(kdist < node.dist_min) { // to be pruned!
                //  LOG << "pruned1\n";
                //     LOG << "PRUNED count= " << node.count << " depth= " << node.depth << "\n";
                if(node.count > 0)
                    internal_pruned_count+= node.count;
                continue;
            }


            //      LOG << "dist_max= " << node.dist_max << " kdist_lb= " << kdist_lb << "\n";

            /*
            if(node.dist_max < kdist_lb) { // it also implies that node.dist_max < kdist, so
                // to be added to result set!

                LOG << "KDISTLB!!!\n";

                {
                    // add sub-tree leaves to priority queue. they will be used to update kdist,
                    // cutting the cost of traversing the subtree up to the leaves.

                    vector<VertexID> tovisit;
                    tovisit.push_back(nodeID);

                    while(tovisit.size() > 0) {
                        VertexID visit_nodeID = tovisit[0];
                        tovisit.erase(tovisit.begin());

                        MyGraph::adjacency_iterator neighbourIt, neighbourEnd;
                        boost::tie(neighbourIt, neighbourEnd) = adjacent_vertices( visit_nodeID, g );

                        for (; neighbourIt != neighbourEnd; ++neighbourIt){
                            VertexID childID = *neighbourIt;
                            Vertex & child = g[childID];
                            if(child.count == 1) { // if a leaf...

                               ts_type dist = q.dist(child.pivot);
                                child.dist = child.dist_min = child.dist_max = dist;

                                pq.push(childID);
                            }  else { // if an internal node...
                                tovisit.push_back(childID);
                            }
                        }
                    }
                }

                result_set.push_back(nodeID);
                continue;
            }
            */

            // we check again for all che children. what cannot be pruned is added to the priority queue.

            MyGraph::adjacency_iterator neighbourIt, neighbourEnd;
            boost::tie(neighbourIt, neighbourEnd) = adjacent_vertices( nodeID, g );

            //LOG << "checking children\n";

            for (; neighbourIt != neighbourEnd; ++neighbourIt){
                VertexID childID = *neighbourIt;
                Vertex & child = g[childID];

                // MAX(0,node.dist - child.dist_parent) = lower bound of distance between query and child pivot
                // MAX(0,node.dist - child.dist_parent) - child.radius = lower bound of distance between child sub-tree
                if(node.dist - child.dist_parent - child.radius > kdist) { // MAX(0,... removed because not needed.
                    // pruned!
                    continue;
                }

		//checked_ts +=child.count;
		
               ts_type dist = q.dist(child.pivot, POW2(kdist + child.radius)); // threshold from  MAX( dist - child.radius, 0) and rearranging elements in inequality, and changing sign to both sides (and from < to >.)
                child.dist_min = MAX( dist - child.radius, 0);
                child.dist_max = dist + child.radius;
                child.dist = dist;

                if(kdist < child.dist_min) { // to be pruned!
                    //  LOG << "child pruned\n";
                    //    LOG << "PRUNED count= " << child.count << " depth= " << node.depth << "\n";

                    if(child.count > 0)
                        internal_pruned_count+= child.count;
                    continue;
                }

                //LOG << "child not pruned\n";
                // not pruned, add it to the priority queue.

                pq.push(childID);

                if(child.dist_max <= kdist) { // valid candidate to tighten kdist bound.
                    kdist_candidates.push_back(childID);
                    update_kdist(K, kdist);

                    //    kdist_lb_candidates.push_back(childID);
                    //    update_kdist_lb(K, kdist_lb);

                }



            }

        } else { // leaf...

            if(kdist < node.dist_min) { // to be pruned! kdist may have been updated since we added this node to the priority queue. this is why it may be eligible for pruning.
                //   LOG << "PRUNED count= " << node.count << "\n";
                continue;
            }

            assert(g[nodeID].dropped == false);
            result_set.push_back(nodeID);

        }



    }

    update_kdist(K, kdist);
    
    //LOG << " exact_distance = " << kdist << "\n";
    //LOG << "result_set= " << result_set << " kdist= " << kdist << "\n";
    //LOG << "loops_count= " << loops_count << " internal_pruned= " << internal_pruned_count << "\n";

    
    // now, creating result set from 'result_set'....

    // uts_i , dist bounds of uts_i .
    pair< vector<int> , vector< pair<ts_type,ts_type> > >  candidates_bounds;


    // all nodes in subtrees part of result set..

    while(result_set.size() > 0) {
        VertexID nodeID = *result_set.begin(); // use it as a simple buffer. first in first out.
        Vertex &node = g[nodeID];
        result_set.erase(result_set.begin());

        //LOG << "results_set.dist_min= " << g[nodeID].dist_min <<  "\n";

        //        LOG << "nodeID" << nodeID << "\n";

        if(g[nodeID].dropped || g[nodeID].dist_min > kdist)
            continue;

        assert(g[nodeID].count == 1);

        candidates_bounds.first.push_back(leaves[nodeID]);
        candidates_bounds.second.push_back(make_pair(node.dist_min, node.dist_max));

    }

    stats_count_partial_time_end();
    stats_update_query_stats(kdist,internal_pruned_count);
    
    return candidates_bounds;

}



struct mycompare_active_dist_max { // lowest first
    const MyGraph  *g;
    mycompare_active_dist_max(MyGraph  *_g) { g = _g;}
    bool operator()(VertexID const &a, VertexID const &b) {
        return ((Vertex)((*g)[a])).dist_max < ((Vertex)((*g)[b])).dist_max;
    }
};

struct mycompare_active_dist_min { // lowest first
    const MyGraph  *g;
    mycompare_active_dist_min(MyGraph  *_g) { g = _g;}
    bool operator()(VertexID const &a, VertexID const &b) {
        return ((Vertex)((*g)[a])).dist_min < ((Vertex)((*g)[b])).dist_min;
    }
};


void MTREE::clean_vector(vector<VertexID> &V,ts_type kdist) {

    // remove dropped or pruned elements and set 'dropped' flag on nodes.

    vector<VertexID> V2;

    for(vector<VertexID>::iterator iter = V.begin() ;  iter != V.end() ; iter++) {
        if(g[*iter].dist_min > kdist) {
            g[*iter].dropped = true;
        }

        if(!g[*iter].dropped) { // if not dropped, retain it...
            V2.push_back(*iter);
        }

    }

    V = V2;
}


void MTREE::clean_vector_lb(vector<VertexID> &V,ts_type kdist_lb) {

    // remove dropped or pruned elements .

    vector<VertexID> V2;

    for(vector<VertexID>::iterator iter = V.begin() ;  iter != V.end() ; iter++) {

        if(!g[*iter].dropped) { // if not dropped ...
            V2.push_back(*iter);
        }

    }

    V = V2;
}


void MTREE::update_kdist(int K,ts_type &kdist) {

    //LOG << "kdist_candidates= " << kdist_candidates.size() << " active_nodes= " << active_nodes.size() << "\n";
    clean_vector(kdist_candidates, kdist);

    int KK = MIN((int)kdist_candidates.size()-1, K-1);

    // step1 compute kth item

    //  LOG << "KK= " << KK << "\n";
    nth_element(kdist_candidates.begin(), kdist_candidates.begin() + KK, kdist_candidates.end() , mycompare_active_dist_max(&g));

    // step2 compute kth item looking at count in Vertex count, shoulld be < then kth item as in step1

    // http://www.cplusplus.com/reference/algorithm/nth_element/
    //nth_element : Sort element in range
    //Rearranges the elements in the range [first,last), in such a way that the element at the nth position is the element that would be in that position in a sorted sequence.
    //The other elements are left without any specific order, except that none of the elements preceding nth are greater than it, and none of the elements following it are less.

    // this means that we must check only the first K-1 elements to improve the bound looking at 'count' of nodes.

    // sort(kdist_candidates.begin(), kdist_candidates.end() , mycompare_active_dist_max(&g));


    //for(unsigned int i = 0 ; i < kdist_candidates.size() ; i++)
    //LOG << "i= " << i << " dist_min= " << g[kdist_candidates[i]].dist_min << " dist_max= " << g[kdist_candidates[i]].dist_max << " count= " << g[kdist_candidates[i]].count << "\n";


    // get first upper bound.
   ts_type critical_dist_ub = g[kdist_candidates[KK]].dist_max;

    if(KK+1 < (int)kdist_candidates.size()-1) // if there are entries after the KKth element, drop them.
        kdist_candidates.erase(kdist_candidates.begin()+KK+1, kdist_candidates.end());

    //  improve it considering 'count' of nodes. cost is O(K). this requires kdist_candidates sorted.
    /*int count = 0;
    int i;
    for(i = 0 ; i <= KK; i++) {
        Vertex &node = g[kdist_candidates[i]];
        count+= node.count;
        if(count >=K) {
            critical_dist_ub = g[kdist_candidates[i]].dist_max;
            break;
        }
    }*/

    assert(critical_dist_ub != -1);

    // update kdist

    // LOG << "kdist= " << kdist << " critical_dist_ub= " << critical_dist_ub << " size= " << kdist_candidates.size() << "\n";

    assert(critical_dist_ub <= kdist); // kdist can only decrease as kdist bound becomes tighter!

    kdist = critical_dist_ub;



}





void MTREE::update_kdist_lb(int K,ts_type &kdist_lb) {

    clean_vector_lb(kdist_lb_candidates, kdist_lb);

    int KK = MIN((int)kdist_lb_candidates.size()-1, K);

    LOG << "kdist_lb_candidates size= " << kdist_lb_candidates.size() << " KK= " << KK << "\n";

    if(KK == -1)
        return;

    nth_element(kdist_lb_candidates.begin(), kdist_lb_candidates.begin() + KK, kdist_lb_candidates.end() , mycompare_active_dist_min(&g));


   ts_type critical_dist_lb = g[kdist_lb_candidates[KK]].dist_min;


    for(unsigned int i = 0 ; i < kdist_lb_candidates.size() ; i++)
        LOG << "i= " << i << " dist_min= " << g[kdist_lb_candidates[i]].dist_min << " dist_max= " << g[kdist_lb_candidates[i]].dist_max << " count= " << g[kdist_lb_candidates[i]].count << "\n";

    LOG << "critical_dist_lb= " << critical_dist_lb << " kdist_lb= " << kdist_lb << "\n";

    assert(critical_dist_lb >= kdist_lb);
    kdist_lb = critical_dist_lb;

    if(KK+1 < (int)kdist_lb_candidates.size()-1) // if there are entries after the KKth element, drop them.
        kdist_lb_candidates.erase(kdist_lb_candidates.begin()+KK+1, kdist_lb_candidates.end());

}


vector<int> MTREE::range_query(TS &q,ts_type search_radius) {

    vector<VertexID> queue;
    vector<int> candidates;

    // init root...
    Vertex &root_node = g[root];
    root_node.dist = q.dist(root_node.pivot);
    queue.push_back(root);

    while(queue.size() > 0) {
        VertexID nodeID = queue[0];
        queue.erase(queue.begin());
        Vertex &node = g[nodeID];

        if(node.count > 1) {// if internal node....

            MyGraph::adjacency_iterator neighbourIt, neighbourEnd;
            boost::tie(neighbourIt, neighbourEnd) = adjacent_vertices( nodeID, g );

            for (; neighbourIt != neighbourEnd; ++neighbourIt){
                VertexID childID = *neighbourIt;
                Vertex & child = g[childID];

                if(node.dist - child.dist_parent - child.radius > search_radius) { // pruned!
  //                  LOG << "pruned\n";
                    continue;
                }

                child.dist = q.dist(child.pivot, POW2(search_radius + child.radius));

                if(child.dist > search_radius + child.radius) { // to be pruned!
//                    LOG << "pruned\n";
                    continue;
                }

                if(child.count == 1) {
                    if(q.dist(child.pivot) <= search_radius)
                        candidates.push_back(leaves[childID]);
                } else {
                    queue.push_back(childID); // not pruned, add it to the queue of nodes to be visited.
                }

            }

        } else { // if leaf...
            FATAL("cannot be.");

        }

    }

    return candidates;

}
