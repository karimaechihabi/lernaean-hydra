#include "mspknn.h"
#include <algorithm>
#include "common.h"
#include <cmath>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using namespace std;
using namespace boost;


extern int random_seed;


struct mycompare_qnn_p_ub {
    bool operator()(QNN_element const &a, QNN_element const &b) {
        return a.p_ub > b.p_ub;
    }
};

struct mycompare_qnn_p_lb {
    bool operator()(QNN_element const &a, QNN_element const &b) {
        return a.p_lb > b.p_lb;
    }
};






MSPkNN::MSPkNN(const Dataset &_dataset,  int _func_C, int _func_R, int _pruning_method,
               int _pivots_method, int _npivots) :
    dataset(_dataset), func_C(_func_C), func_R(_func_R), pruning_method(_pruning_method) {

    assert(pruning_method == PRUNING_METRIC_LSCAN);

    pnn_count = 0;
    LOG << "MSPkNN func_C= " << func_C << " func_R= " << func_R << " pruning_method= " <<
           DistanceIntervalSets::pruning_method_names[pruning_method] << " pivots_method= " <<
           DistanceIntervalSets::pruning_metric_pivot_names[_pivots_method] << " npivots= " << _npivots << "\n";


    S.build_metric_index(dataset, _pivots_method, _npivots);

}




MSPkNN::MSPkNN(const Dataset &_dataset,  int _func_C, int _func_R, int _pruning_method) :
    dataset(_dataset), func_C(_func_C), func_R(_func_R), pruning_method(_pruning_method) {

    pnn_count = 0;
    LOG << "MSPkNN func_C= " << func_C << " func_R= " << func_R << " pruning_method= " << DistanceIntervalSets::pruning_method_names[pruning_method] << "\n";

    switch(pruning_method) {
    case PRUNING_EXACT_LSCAN:
        // nothing to do.
        break;

    case PRUNING_SPATIAL_LSCAN:
        S.build_spatial_index(dataset);
        break;

    case PRUNING_METRIC_LSCAN:
        S.build_metric_index(dataset, PRUNING_METRIC_PIVOT_KMEANS, 1);
        break;

    case PRUNING_METRIC_MTREE:
        S.build_metric_mtree_index(dataset);
        break;

    default:
        FATAL("WTF");

    }

}



pair<double,double> MSPkNN::PNN_bounds(int uts_i) {

    pnn_count++;

    double P_lb = 0, P_ub = 0;

    //  for(int uts_j = 0; uts_j < dataset.N; uts_j++) {
    //    LOG << "S uts_i = " << uts_j << " " << S.get(uts_j) << "\n";
    // }

    double best_score = -1;
    M[uts_i] = -1; // updated

    unsigned int size = S.S[uts_i].size();

    double xxx = 0;

    for(unsigned int s_a = 0; s_a < size; s_a++) {

        double score = 0;

        double p_lb = 1.0;
        double p_ub = 1.0;

        bool stop_lb = false;

        for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
            QNN_element &qnn_element = *iter;
            int & uts_j = qnn_element.uts_i;

            if(uts_i ==uts_j)
                continue;


            double sum_w_lb = 0;
            double sum_w_ub = 0;

            unsigned int size2= S.S[uts_j].size();

            for(unsigned int s_b = 0; s_b < size2; s_b++) {
                if(!stop_lb && S.S[uts_i][s_a].ub <  S.S[uts_j][s_b].lb) { // lb
                    sum_w_lb += S.S[uts_j][s_b].w;
                }
                if(S.S[uts_i][s_a].lb < S.S[uts_j][s_b].ub) { // ub
                    sum_w_ub += S.S[uts_j][s_b].w;
                    if(S.S[uts_i][s_a].ub > S.S[uts_j][s_b].lb) {
                        score+=S.S[uts_j][s_b].w;
                    }
                }
            }

            p_lb *= sum_w_lb;
            p_ub *= sum_w_ub;

            if(!stop_lb && p_lb == 0)
                stop_lb = true;


        }

        P_lb += S.S[uts_i][s_a].w * p_lb;
        P_ub += S.S[uts_i][s_a].w * p_ub;

        if(P_lb > critical_ub) {
            // part of result-set! we can stop here.
            break;
        }

        xxx+= S.S[uts_i][s_a].w;

        if(P_ub + (1-xxx) < critical_lb) {
             M[uts_i] = -2;
             // 1-xxx = upperbound of residual probability to sum up.
            // probability upperbound always lower than P_ub + ....
            break;
        }


        if(S.S[uts_i][s_a].lb != S.S[uts_i][s_a].ub) {
            score*= S.S[uts_i][s_a].w * (S.S[uts_i][s_a].ub - S.S[uts_i][s_a].lb);
            //LOG << "score_Mi uts_i= " << uts_i << " s_a= " << s_a << " " << score  << "\n";
            if(score > best_score) {
                best_score = score;
                M[uts_i] = s_a;
            }
        }

    }


    //LOG << "Plb,Pub = " << P_lb << " " << P_ub << "\n";
    assert(P_lb >= 0 && P_lb <=1 + 1e-10);
    assert(P_ub >= 0 && P_ub <=1 + 1e-10);

    return make_pair(P_lb, P_ub);

}



double MSPkNN::getCandidatesRatio(int K, const TS &query) {

    vector<int> candidates;
    init_candidates(K, query, candidates);

    return ((double)(candidates.size())) / ((double)dataset.N);

}


vector<int> MSPkNN::getCandidates(int K, const TS &query) {


    vector<int> candidates(dataset.N);
    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        candidates[uts_i] = uts_i;
    }

    nth_element(candidates.begin(), candidates.begin() + K-1, candidates.end() , mycompare_qnn_dist_ub(&S));
    double critical_distance_ub = S.S[candidates[K-1]][0].ub; // upper bound based on metric bounds

    vector<int> candidates2;

    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        if(S.S[uts_i][0].lb <= critical_distance_ub)
            candidates2.push_back(uts_i);
    }

    return candidates2;

}

void MSPkNN::init_probability_bounds(int K, const TS &query) {

    critical_lb = 0;
    critical_ub = 1;

    for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
        QNN_element &qnn_element = *iter;
        M[qnn_element.uts_i] = -2; // to be updated
        pair<double,double> pnn_bounds = PNN_bounds(qnn_element.uts_i);
        qnn_element.p_lb = pnn_bounds.first;
        qnn_element.p_ub = pnn_bounds.second;
    }


    cout << "#QNN n= " << dataset.n << " N= " << dataset.N << " m= " << dataset.m << " size= " << QNN.size() << "\n";


    LOG << "QNN size after initialization= " << QNN.size() << "/" << dataset.N << "\n";
    assert(QNN.size() > 0);

}

void MSPkNN::update_probability_bounds() {

    for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {

        QNN_element &qnn_element = *iter;

        // if not within critical region, do NOT update bounds.
        if(qnn_element.p_ub == 0 || qnn_element.p_ub < critical_lb || qnn_element.p_lb > critical_ub)
            continue;

        // if bounds cannot be improved, do NOT try to improve bounds.
        if(qnn_element.p_lb == qnn_element.p_ub)
            continue;

        M[qnn_element.uts_i] = -2; // to be updated
        pair<double,double> pnn_bounds = PNN_bounds(qnn_element.uts_i);

        qnn_element.p_lb = pnn_bounds.first;
        qnn_element.p_ub = pnn_bounds.second;
    }
}


vector<int> MSPkNN::find_candidates_ms(int K) {
    vector<int> C;

    vector<int> double_crossers;
    vector<int> left_crossers;
    vector<int> right_crossers;

    boost::variate_generator<boost::mt19937, boost::uniform_int<> >
            randomly(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0,QNN.size())); // not -1 because it gets % before used.

    double max_overlap = -1;
    int uts_max_overlap = -1;

    for(vector< QNN_element >::iterator iter = QNN.begin() ; iter != QNN.end() ; iter++) {
        QNN_element &qnn_element = *iter;

        if(qnn_element.p_lb == qnn_element.p_ub) // no need to refine this!
            continue; // also needed to avoid infinite loops.

        if(qnn_element.p_ub == 0) // if prob. upperbound == 0, then this is for sure not a candidate to refine.
            continue;

        //  LOG << "B[" << uts_i << "] = " << B[uts_i] << "\n";

        if( qnn_element.p_lb < critical_lb && critical_ub < qnn_element.p_ub) {
            double_crossers.push_back(qnn_element.uts_i);
        }

        if(qnn_element.p_lb < critical_lb) {
            left_crossers.push_back(qnn_element.uts_i);
        }

        if(critical_ub < qnn_element.p_ub) {
            right_crossers.push_back(qnn_element.uts_i);
        }

        double overlap = OVERLAP(critical_lb, critical_ub, qnn_element.p_lb, qnn_element.p_ub);
        if(overlap > max_overlap) {
            max_overlap = overlap;
            uts_max_overlap = qnn_element.uts_i;
        }

    }


    if(double_crossers.size() > 0) {
        // double crosser
        //      LOG << "double crosser\n";
        C.push_back(double_crossers[randomly() % double_crossers.size()]);
    } else {
        if(left_crossers.size() > 0 && right_crossers.size() > 0) {
            //            LOG << "left right crosser\n";
            // left and right crossers
            C.push_back(left_crossers[randomly() % left_crossers.size()]);
            C.push_back(right_crossers[randomly() % right_crossers.size()]);
        } else {
            //        LOG << "maximal overlapping crosser\n";
            // maximal overlapping here.
            //assert(uts_max_overlap != -1);
            if(uts_max_overlap != -1)
                C.push_back(uts_max_overlap);
        }
    }

    return C;

}


vector<int> MSPkNN::find_candidates_opt(int K) {
    vector<int> C;

    //    for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
    //      LOG << "uts_" << uts_i << " B= " << B[uts_i] << " len(C_i)= " << S.S[uts_i].size() << "/" << dataset.m << "\n";
    //    }


    for(vector< QNN_element >::iterator iter = QNN.begin() ; iter != QNN.end() ; iter++) {
        QNN_element &qnn_element = *iter;

        if(qnn_element.p_lb == qnn_element.p_ub) // no need to refine this!
            continue; // also needed to avoid infinite loops.

        if(qnn_element.p_ub == 0) // if prob. upperbound == 0, then this is for sure not a candidate to refine.
            continue;

        if( WITHIN(qnn_element.p_lb, critical_lb, critical_ub) ||
                WITHIN(qnn_element.p_ub, critical_lb, critical_ub) ||
                WITHIN(critical_lb, qnn_element.p_lb, qnn_element.p_ub))
            C.push_back(qnn_element.uts_i); // if it overlaps with the critical interval...

    }

    return C;

}



vector< pair<int,int> > MSPkNN::find_splits(vector<int> C) {
    vector< pair<int,int> > R;


    //    for(int uts_j = 0; uts_j < dataset.N; uts_j++) {  LOG << "S uts_i = " << uts_j << " " << S.get(uts_j) << "\n"; }



    for(vector<int>::iterator iter = C.begin(); iter != C.end(); iter++) {
        int uts_i = *iter;
        pair<int,int>  sa, sb;
        double w_best = -1;

        for(unsigned int a = 0; a < S.S[uts_i].size(); a++) {

            for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
                QNN_element &qnn_element = *iter;
                int &uts_j = qnn_element.uts_i;

                if(uts_i == uts_j)
                    continue;

                for(unsigned int b = 0; b < S.S[uts_j].size(); b++) {

                    double W = S.S[uts_i][a].w*(S.S[uts_i][a].ub - S.S[uts_i][a].lb)  + S.S[uts_j][b].w*(S.S[uts_j][b].ub - S.S[uts_j][b].lb);
                    //double W = S.S[uts_i][a].w  + S.S[uts_j][b].w;

                    if(W <= w_best)
                        continue;

                    if(S.overlaps(uts_i, a, uts_j, b)) {
                        // LOG << "OVERLAP\n";
                        sa = make_pair(uts_i, a);
                        sb = make_pair(uts_j, b);
                        w_best = W;
                    }

                }
            }
        }

        if(w_best > 0) {

            if(S.S[sa.first][sa.second].lb != S.S[sa.first][sa.second].ub &&
                    S.S[sa.first][sa.second].tosplit == false) {
                S.S[sa.first][sa.second].tosplit = true;
                R.push_back(sa);
            }

            if(S.S[sb.first][sb.second].lb != S.S[sb.first][sb.second].ub &&
                    S.S[sb.first][sb.second].tosplit == false) {
                S.S[sb.first][sb.second].tosplit = true;
                R.push_back(sb);
            }

        }

    }

    //    LOG << "R " << R << "\n";
    return R;
}





pair<int,int> MSPkNN::R1(int uts_i) { // lowerbound
    double score_best = -1;
    pair<int,int> best = make_pair(-1,-1);

    //FATAL("not used");

    //LOG << "R1 " << uts_i << "\n";

    for(unsigned int a = 0; a < S.S[uts_i].size(); a++) {

        if(S.S[uts_i][a].lb == S.S[uts_i][a].ub)
            continue;

        double score = 0;

        for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
            QNN_element &qnn_element = *iter;
            int & uts_j = qnn_element.uts_i;


            if(uts_i == uts_j)
                continue;
            for(unsigned int b = 0; b < S.S[uts_j].size(); b++) {
                if(S.S[uts_i][a].ub > S.S[uts_j][b].lb &&
                        S.S[uts_i][a].lb < S.S[uts_j][b].ub) {
                    //LOG << "matching: uts_i,a=" << S.S[uts_i][a].lb << "," << S.S[uts_i][a].ub <<
                    //     " uts_j,b=" << S.S[uts_j][b].lb <<"," << S.S[uts_j][b].ub << "\n" ;
                    score+= S.S[uts_j][b].w; // * (S.S[uts_j][b].ub - S.S[uts_j][b].lb);
                }
            }
        }
        score*= S.S[uts_i][a].w * (S.S[uts_i][a].ub - S.S[uts_i][a].lb);

        //  LOG << "score  uts_i= " << uts_i << " a= " << a << " score= " << score << "\n";
        if(score > score_best) {
            score_best = score;
            best = make_pair(uts_i, a);
        }
    }

    //LOG << "R1 score_best= " << score_best << " uts_i= " << uts_i << " ref= " << best << "\n";

    //LOG <<"\n";

    return best;
}

pair<int,int> MSPkNN::R3(int uts_i) {
    double score_best = -1;
    pair<int,int> best = make_pair(-1,-1);

    for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
        QNN_element &qnn_element = *iter;
        int & uts_j = qnn_element.uts_i;

        if(uts_i == uts_j)
            continue;

        unsigned int size = S.S[uts_j].size();

        for(unsigned int b = 0; b < size; b++) {

            if(S.S[uts_j][b].lb == S.S[uts_j][b].ub)
                continue;

            double score = 0;

            unsigned int size2= S.S[uts_i].size();
            for(unsigned int a = 0; a < size2; a++) {
                if(S.S[uts_i][a].ub > S.S[uts_j][b].lb &&
                        S.S[uts_i][a].lb < S.S[uts_j][b].ub) {
                    score+= S.S[uts_i][a].w; //* (S.S[uts_i][a].ub - S.S[uts_i][a].lb);
                }
            }
            score *= S.S[uts_j][b].w * (S.S[uts_j][b].ub - S.S[uts_j][b].lb);

            if(score > score_best) {
                score_best = score;
                best = make_pair(uts_j, b);
            }
        }
    }

    //  LOG << "R3 score_best= " << score_best  << " uts_i= " << uts_i << " ref= " << best << "\n";


    return best;
}





vector< pair<int,int> > MSPkNN::find_splits_opt(vector<int> C) {
    vector< pair<int,int> > R;


    //for(int uts_j = 0; uts_j < dataset.N; uts_j++) {  LOG << "S uts_i = " << uts_j << " " << S.get(uts_j) << "\n"; }


    for(vector<int>::iterator iter = C.begin(); iter != C.end(); iter++) {

        int uts_i = *iter;

        vector< pair<int,int> > refs;


        if(M[uts_i] ==-2)
            M[uts_i] = R1(uts_i).second;

        refs.push_back(make_pair(uts_i, M[uts_i]));
        refs.push_back(R3(uts_i));

        //pair<int,int> r1 = R1(uts_i);
        //LOG << "Mi[uts_i]= " << M[uts_i] << "\n";
        //assert(r1.second == M[uts_i]);

        for(vector< pair<int,int> >::iterator iter = refs.begin();
            iter != refs.end(); iter++) {
            pair<int,int> ref = *iter;

            if(ref.second == -1)
                continue;

            //LOG << "for uts_i " << uts_i << " refine " << ref << ": " << S.S[ref.first][ref.second].lb << " " << S.S[ref.first][ref.second].ub << " w " << S.S[ref.first][ref.second].w << "\n";

            if(S.S[ref.first][ref.second].lb != S.S[ref.first][ref.second].ub &&
                    S.S[ref.first][ref.second].tosplit == false) {
                S.S[ref.first][ref.second].tosplit = true;
                R.push_back(ref);
            }

        }

    }

    //LOG << "RRR " << R << "\n";

    return R;
}



void MSPkNN::dist_refines(vector< pair<int,int> > R, const TS &query) {
    S.dist_refines(dataset, query, R);
}





void MSPkNN::update_T(int K) {

    for(vector< QNN_element >::iterator iter = QNN.begin(); iter != QNN.end(); iter++ ) {
        QNN_element &qnn_element = *iter;

        if(qnn_element.p_ub == 0) {
            // this needed when probabs are [0,1],[0,1],[0,0],...,[0,0].
            // when this happens, c,d = 0,0 and all candidates enter T (erroneously.)
            continue;
        }

        if(critical_lb <= qnn_element.p_lb)
            T.push_back(make_pair(qnn_element.uts_i, make_pair(qnn_element.p_lb, qnn_element.p_ub)));

    }


}

void MSPkNN::update_critical(int K) {

    //sort(QNN.begin(), QNN.end(), mycompare_qnn_p_ub());
    //critical_ub = QNN[ MIN((unsigned int)K,QNN.size()-1)].p_ub;
    //sort(QNN.begin(), QNN.end(), mycompare_qnn_p_lb());
    //critical_lb = QNN[K-1].p_lb;

    nth_element(QNN.begin(), QNN.begin() + MIN((unsigned int)K,QNN.size()-1), QNN.end(), mycompare_qnn_p_ub());
    critical_ub = QNN[ MIN((unsigned int)K,QNN.size()-1)].p_ub;

    nth_element(QNN.begin(), QNN.begin() + K -1 , QNN.end(), mycompare_qnn_p_lb());
    critical_lb = QNN[K-1].p_lb;



    // we cannot drop elements in QNN that are not within the critical
    // region because they can still influence the bounds of elements
    // in the critical region.

    //   LOG << "critical region= " << critical_lb << ", " << critical_ub << "\n";
}


void MSPkNN::init_candidates(int K, const TS &query, vector<int> &candidates) {

    switch(pruning_method) {
    case PRUNING_EXACT_LSCAN:
        S.init_exact_bounds(dataset, query);
        assert((int)S.S.size() == dataset.N); // this not for mtree.
        break;

    case PRUNING_SPATIAL_LSCAN:
        S.init_spatial_bounds(dataset, query);
        assert((int)S.S.size() == dataset.N); // this not for mtree.
        break;

    case PRUNING_METRIC_LSCAN:
        S.init_metric_bounds(dataset, query);
        assert((int)S.S.size() == dataset.N); // this not for mtree.
        break;

    case PRUNING_METRIC_MTREE:
        // nothing to do.
        break;

    default:
        FATAL("WTF");

    }

    if(pruning_method == PRUNING_METRIC_MTREE) {
        candidates= S.mtree_find_topk(dataset, query, K);

    } else{
        vector<int> candidates_all(dataset.N);
        for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
            candidates_all[uts_i] = uts_i;
        }
        nth_element(candidates_all.begin(), candidates_all.begin() + K-1, candidates_all.end() , mycompare_qnn_dist_ub(&S));
        double critical_distance_ub = S.S[candidates_all[K-1]][0].ub; // approx upper bound
        for(vector<int>::iterator iter = candidates_all.begin() ; iter != candidates_all.end() ; iter++) {
            if(S.S[*iter][0].lb <= critical_distance_ub) {
                //      LOG  << "adding to candidates uts_i= " << *iter << " dist bounds= " << S.S[*iter][0].lb << " " << S.S[*iter][0].ub << "\n";
                candidates.push_back(*iter);
            }
        }
    }


    //((Dataset *)(&dataset))->retain(candidates); // can be used , only if the loaded dataset is not used more than once.

}

void MSPkNN::init_distance_bounds(int K, const TS &query) { // this function initializes 'QNN' .

    vector<int> candidates;
    init_candidates(K, query, candidates);

    LOG << "#CANDIDATES count_all= " << dataset.N <<  " count= " << candidates.size() << " pruning_method= " << DistanceIntervalSets::pruning_method_names[pruning_method] << "\n";

    LOG << "#TIMESTEPS init_distance_bounds.lookup_candidates= " << time.get() << "\n";

    //LOG << "candidates= " << candidates << "\n";

    // now candidates initilized using pruning.

    // compute exact critical_distance_ub and prune on exact bounds.


    double min = 1e5, max = -1e5;

    for(vector<int>::iterator iter = candidates.begin() ; iter != candidates.end() ; iter++) {
        int uts_i = *iter;
        //  LOG << "refine " << uts_i <<"\n";
        S.dist_refine(dataset, uts_i, 0, query); // update approximated bounds to exact bounds
  //      LOG << uts_i << " " << S.S[uts_i][0].lb << " " << S.S[uts_i][0].ub << "\n";
        min = MIN(min, S.S[uts_i][0].lb);
        max = MAX(S.S[uts_i][0].ub, max);
    }

    cout << "#DIST n= " << dataset.n<<" min= " << min << " max= "<< max << " diff= " << max - min << "\n";

    nth_element(candidates.begin() , candidates.begin() + K-1, candidates.end() , mycompare_qnn_dist_ub(&S));
    double critical_distance_ub = S.S[candidates[K-1]][0].ub; // exact upper bound

//    LOG << "#CRITICAL n= " << dataset.n << " critical_distance_ub= " << critical_distance_ub << "\n";

    //LOG << "critical_distance_ub= " << critical_distance_ub << "\n";

    for(vector<int>::iterator iter = candidates.begin() ; iter != candidates.end() ; iter++) {
        //  LOG << "uts_i= " << *iter << " dist_lb= " <<S.S[*iter][0].lb << " dist_ub= " << S.S[*iter][0].ub << "\n";
        if(S.S[*iter][0].lb <= critical_distance_ub) {
            QNN_element qnn_element;
            qnn_element.uts_i = *iter;
            QNN.push_back(qnn_element);
        }
    }

    //LOG << "qnn size= " << QNN.size() << "\n";
    //exit(0);

}


vector< pair<int, pair<double,double> > > MSPkNN::PkNN(const TS &query, int K) {

    time.begin();

    M.resize(dataset.N, -2); // -2: not updated and to update, -1: nothing , >=0: distance interval to refine


    //    M.resize(dataset.N, -2); // -2: not updated and to update, -1: nothing , >=0: distance interval to refine

    S.approx_count = 0;
    int refinements_count = 0;

    // M.resize(dataset.N, true);
    //  B.resize(dataset.N, make_pair(0,1));

    LOG << "#TIMESTEPS time1=" << time.get() << "\n";

    init_distance_bounds(K, query);

    LOG << "#TIMESTEPS time2=" << time.get() << "\n";

    init_probability_bounds(K, query);

    LOG << "#TIMESTEPS time3=" << time.get() << "\n";

    update_critical(K);

    LOG << "#TIMESTEPS time4=" << time.get() << "\n";

    pnn_count = 0;

    //for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
    //  LOG << "uts_" << uts_i << " B= " << B[uts_i] << " len(C_i)= " << S.S[uts_i].size() << "/" << dataset.m << "\n";
    //}
    //LOG << "-----------------\n";

    int L = 0;
    while(critical_lb <= critical_ub) {
        L++;

        //     LOG << "LOOP " << L << " RANDOM_SEED " << random_seed << " refinements_count= " << refinements_count << "\n";

        vector<int> C;
        switch(func_C) {
        case FUNC_C_FIND_CANDIDATES_MS:
            C = find_candidates_ms(K);
            break;
        case FUNC_C_FIND_CANDIDATES_OPT:
            C = find_candidates_opt(K);
            break;
        default:
            FATAL("unknown func_C");
        }
        //LOG << "C= " << C << "\n";




        vector< pair<int,int> > R;
        switch(func_R) {
        case FUNC_R_FIND_SPLITS:
            R = find_splits(C);
            break;
        case FUNC_R_FIND_SPLITS_OPT:
            R = find_splits_opt(C);
            break;
        default:
            FATAL("unknown func_R");
        }
        //LOG << "R= " << R << "\n";

        if(R.empty()) {
            LOG << "Warning: R is empty. c,d = " << critical_lb << ", " << critical_ub << "\n";
            update_probability_bounds();
            update_critical(K);
            break;
        }

        refinements_count+= R.size();
        dist_refines(R, query);

        update_probability_bounds();

        //        for(vector< pair<int,int> >::iterator iter = R.begin(); iter != R.end(); iter++) {
        //            M[iter->first] = false;
        //        }


        update_critical(K);

        //for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
        //  LOG << "uts_" << uts_i << " B= " << B[uts_i] << " len(C_i)= " << S.S[uts_i].size() << "/" << dataset.m << "\n";
        //}

        //LOG << "\n\n";

    }

    update_T(K);

    //      for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
    //     LOG << "uts_" << uts_i << " B= " << B[uts_i] << " len(C_i)= " << S.S[uts_i].size() << "/" << dataset.m << "\n";
    //  }

    LOG << "#TIMESTEPS time5=" << time.get() << "\n";



    cout << "#STATS n= " << dataset.n << " N= " << dataset.N << " m= " << dataset.m <<
        " refinements_count= " << refinements_count <<
        " approx_count= " << S.approx_count <<
        " loops_count= " << L <<
        " pnn_count= " << pnn_count <<
        " qnn_size= " << QNN.size() << "\n";

    return T;

}






void MSPkNN::eval_analyze(const TS &query, int K) {
    LOG << "eval_analyze\n";

    dataset.dumpToFile("mydataset.bin");

    S.build_spatial_index(dataset);
    S.build_metric_index(dataset, PRUNING_METRIC_PIVOT_KMEANS, 1);

    S.init_exact_bounds(dataset, query);
    S.dumpToFile_S("S_exact.bin");

    S.init_spatial_bounds(dataset, query);
    S.dumpToFile_S("S_spatial.bin");

    S.init_metric_bounds(dataset, query);
    S.dumpToFile_S("S_metric.bin");

    S.dumpToFile_D("D.bin", dataset, query);

    //XXX e' possibile usare l'envelope come pivot "speciali"
    //XXX sui quali e' possibile avere anche pruning spaziale.
    //XXX da fare con metodo "combined".


}

vector< pair<int, pair<double,double> > > MSPkNN::eval(const TS &query, int K) {

    LOG << "=== MSPkNN begin ===\n";
    vector< pair<int, pair<double,double> > > T = PkNN(query, K);

    LOG << "MSPkNN result set: ";
    for(vector< pair<int, pair<double,double> > >::iterator iter = T.begin(); iter != T.end(); iter++) {
        LOGL << iter->first << ":[" << iter->second.first << "," << iter->second.second << "] ";
    }
    LOGN;

    LOG << "=== MSPkNN end ===\n\n";

    return T;
}






int MSPkNN::classify(const TS &query) {

    vector< pair<int, pair<double,double> > > mspknn_result = eval(query, 1);

    assert(mspknn_result.size() > 0);
    int nn_class_label = dataset.utss[mspknn_result[0].first].class_label;

    return nn_class_label;

}
