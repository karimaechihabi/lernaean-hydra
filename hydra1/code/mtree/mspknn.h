#ifndef MSPKNN_H
#define MSPKNN_H

#include "dataset.h"
#include "distints.h"
#include "elapsedtime.h"


#define FUNC_C_FIND_CANDIDATES_MS 0
#define FUNC_C_FIND_CANDIDATES_OPT 1
#define FUNC_R_FIND_SPLITS 0
#define FUNC_R_FIND_SPLITS_OPT 1


struct QNN_element {
    int uts_i;
    double p_lb;
    double p_ub;
    //int best_refine;
};

class MSPkNN
{



private:

    ElapsedTime time;

#define HV_SIZE 1000
    unsigned char hv[HV_SIZE];


    struct mycompare_qnn_dist_lb { // lowest first
        const DistanceIntervalSets  *S;
        mycompare_qnn_dist_lb(DistanceIntervalSets  *_S) { S = _S ;}
        bool operator()(QNN_element const &a, QNN_element const &b) {
            return S->S[a.uts_i][0].lb < S->S[b.uts_i][0].lb;
        }
    };

    struct mycompare_qnn_dist_ub { // lowest first
        const DistanceIntervalSets  *S;
        mycompare_qnn_dist_ub(DistanceIntervalSets  *_S) { S = _S ;}
        bool operator()(int const &a, int const &b) {
            return S->S[a][0].ub < S->S[b][0].ub;
        }
    };


    //
    const Dataset &dataset; // dataset reference
    //ProbabilityBounds B; // probability bounds

    // splits_opt
    std::vector<int> M; // M[uts_i] best distance interval in uts_i to improve uts_i bounds

    std::vector< QNN_element > QNN; // candidates for pknn


    /*
    XXX secondo me pknn e' piu veloce perche' non ci sono loop su 1000 elementi
    per via del candidato virtuale.  invece in mspknn ci sono sempre loop nel
    range [0,dataset.N] che *ammazzano* la performance nel confronto.

    soluzione: utilizzare QNN e QREF come liste di elementi da considerare
               invece che avere loop su [0,dataset.N].

    sistemare anche update_pnn_bounds in modo tale che consideri i primi K
               elementi oppure tutti da QNN. useremo la versione che funziona meglio.
*/


    //
    int func_C;
    int func_R;
    //

    int pnn_count;


    double critical_lb; // lb critical region
    double critical_ub; // ub critical region
    std::vector< std::pair<int, std::pair<double,double> > > T; // result set

    //
    void update_probability_bounds(); // update all probab. bounds
    std::pair<double,double> PNN_bounds(int uts_i); // estimate probab. bounds for uts_i

    std::vector< std::pair<int, std::pair<double,double> > > PkNN(const TS &query, int K); // evaluate PkNN query
    std::vector<int> find_candidates_ms(int K); // find candidates to refine probab. bounds
    std::vector<int> find_candidates_opt(int K); // find candidates to refine probab. bounds
    std::vector < std::pair<int,int> > find_splits(std::vector<int> C); // find best splits in C candidates
    std::vector < std::pair<int,int> > find_splits_opt(std::vector<int> C); // find best splits in C candidates

    std::pair<int,int> R1(int uts_i);
    std::pair<int,int> R2(int uts_i);
    std::pair<int,int> R3(int uts_i);
    std::pair<int,int> R4(int uts_i);

    void dist_refines(std::vector< std::pair<int,int> > R, const TS &query); // refine distance intervals in R
    void update_critical(int K); // updates critical_lb,critical_ub,T,D
    void update_T(int K);
    void init_probability_bounds(int K, const TS &query);
    void init_distance_bounds(int K, const TS &query);
    void init_candidates(int K, const TS &query, vector<int> &candidates);


public:

    DistanceIntervalSets  S; // distance interval sets
    //    bool classification_task;



    double getCandidatesRatio(int K, const TS &query);

    std::vector<int> getCandidates(int K, const TS &query);

    int pruning_method;


    MSPkNN(const Dataset &_dataset,  int _func_C, int _func_R, int _pruning_method, int _pivots_method, int npivots);
    MSPkNN(const Dataset &dataset, int func_C, int func_R, int pruning_method);
    std::vector< std::pair<int, std::pair<double,double> > > eval(const TS &query, int K);
    void eval_analyze(const TS &query, int K);

    int classify(const TS &query);


};

#endif // MSPKNN_H
