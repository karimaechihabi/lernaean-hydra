#ifndef PKNN_H
#define PKNN_H

#include "dataset.h"
#include "distints.h"
#include "elapsedtime.h"


typedef std::pair<int,ts_type> intts_type;
typedef std::vector< std::vector<ts_type> > Dists;
typedef std::pair<ts_type,ts_type> PBounds;
typedef std::vector< PBounds > ProbabilityBounds;


class PkNN
{

private:

    struct mycompare_qnn_dist_ub { // lowest first
        const DistanceIntervalSets  *S;
        mycompare_qnn_dist_ub(DistanceIntervalSets  *_S) { S = _S ;}
        bool operator()(int const &a, int const &b) {
            return S->S[a][0].ub < S->S[b][0].ub;
        }
    };


    //
    const Dataset &dataset; // dataset reference


    //

    int pruning_method;
    //

    std::vector< std::pair<int, std::pair<ts_type, ts_type> > > Q; // priority queue ("active" candidates.)



    //
    std::pair<ts_type,ts_type> PNN_bounds(int uts_i); // estimate probab. bounds for uts_i
    std::vector< std::pair<int, std::pair<ts_type, ts_type> > > PkNN_eval(const TS &query, int K); // evaluate PkNN query
    std::vector < std::pair<int,int> > find_splits_opt(std::vector<int> C); // find best splits in C candidates

    void dist_refines(std::vector< std::pair<int,int> > R, const TS &query); // refine distance intervals in R

    void init_candidates(int K, const TS &query, vector<int> &candidates);
    void init_mindist(int K, const TS &query, std::vector< std::pair<int, ts_type> > & min_dist);
    void update_mindist(std::vector< std::pair<int, ts_type> > & min_dist);
    bool outer_subregion(ts_type last_lb, std::pair<int,int> distint);
    void update_priority_queue();

    std::pair<int,int> R1(int uts_i);
    std::pair<int,int> R3(int uts_i);


    int pnn_count;

    ts_type last_lb;

    void init_virtual();

public:

    DistanceIntervalSets  S; // distance interval sets
    //    bool classification_task;


    PkNN(const Dataset &_dataset, int _pruning_method);


    std::vector< std::pair<int, std::pair<ts_type,ts_type> > > eval(const TS &query, int K);
    void eval_analyze(const TS &query, int K);

    int classify(const TS &query);


};

#endif // MSPKNN_H
