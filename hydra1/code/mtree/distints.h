#ifndef DISTINTS_H
#define DISTINTS_H

#include "common.h"
#include "dataset.h"
#include "mtree.h"

#include <set>
#include <vector>



#define PRUNING_EXACT_LSCAN 0

#define PRUNING_SPATIAL_LSCAN 1
#define PRUNING_METRIC_LSCAN 2
#define PRUNING_METRIC_MTREE 3

#define PRUNING_METRIC_PIVOT_RANDOM 0
#define PRUNING_METRIC_PIVOT_MAXDIST 1
#define PRUNING_METRIC_PIVOT_KMEANS 2



struct OverlappingIntervals {
    int uts_i;
    int a;

    int uts_j;
    int b;

    std::pair<ts_type,ts_type> split_points_i;
    std::pair<ts_type,ts_type> split_points_j;

    bool overlap;

};



struct DistanceInterval {
    ts_type lb;
    ts_type ub;
    ts_type w;
    bool tosplit;
    int index_lb;
    int index_ub;
    int refs_count;

};



template <typename Iter, typename Cont>
bool is_last(Iter iter, const Cont& cont)
{
    return (iter != cont.end()) && (next(iter) == cont.end());
}


typedef std::vector<DistanceInterval> DistanceIntervals;



class DistanceIntervalSets
{    
public:

    static const char *pruning_method_names[] ;
    static const char *pruning_metric_pivot_names [];



    //
    std::vector< std::vector<ts_type> > D;
    std::vector<DistanceIntervals> S;

    std::vector<int> splitted;


    // spatial
    std::vector< std::pair<TS,TS> > envelopes;


    bool activate_approx;
    ts_type approx_w_threshold; // intervals with weight below thi value are collapsed to their center value.

    int approx_count;
    // metric
    int n_pivots;
    std::vector< std::vector<TS> > pivots;
    std::vector< std::vector< std::pair<ts_type,ts_type> > >pivot_dists;


    // metric mtree
    MTREE mtree;

        void avg_k_pivots(const Dataset &dataset,int uts_i,  vector<TS> &uts_pivots, int n_pivots);

    //
    DistanceIntervalSets();

    //DistanceIntervals& operator[] (int uts_i) {
    //  assert(uts_i >= 0 && (unsigned int)uts_i < S.size());
    //        return S[uts_i];
    //  }

    std::vector< std::pair< std::pair<ts_type, ts_type>, ts_type> > get(int uts_i);


    void kmeans_pivots(const Dataset &dataset, int uts_i, vector<TS> &uts_pivots, int n_pivots);

    void init_dists(const Dataset &dataset, int uts_i, const TS &query);

    bool overlaps(int uts_i, int a, int uts_j, int b);
    void dist_refines(const Dataset &dataset, const TS &query, std::vector< std::pair<int,int>  > R);
    void dist_refine(const Dataset &dataset, int uts_i, int a, const TS &query);

    void init_exact_bounds(const Dataset &dataset, const TS &query);
    void init_spatial_bounds(const Dataset &dataset, const TS &query);
    void init_metric_bounds(const Dataset &dataset, const TS &query);
    void init_exact_bounds_max_refinement(const Dataset &dataset, const TS &query);

    int find_nearest_cluster(const Dataset &dataset, int uts_i, int sample_i, int *pivots, int n_pivots);


    void dumpToFile_S(const char *pathname);

    void build_spatial_index(const Dataset &dataset);
    void build_metric_index(const Dataset &dataset, int pivots_method, int n_pivots);
    void build_metric_mtree_index(const Dataset &dataset);

    int binary_search_le(int uts_i, ts_type key, int index_lb, int index_ub);
    void dumpToFile_D(const char *pathname, const Dataset &dataset, const TS &query);

    bool init_dist_prune(const Dataset &dataset, int uts_i,  const TS &query, ts_type  dist_ub);

    vector<int> mtree_find_topk(const Dataset &dataset, const TS &query, int K);

};

#endif // DISTINTS_H
