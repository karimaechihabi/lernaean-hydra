#include "pknn.h"
#include <algorithm>
#include "common.h"
#include <cmath>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

using namespace std;
using namespace boost;


extern int random_seed;



PkNN::PkNN(const Dataset &_dataset, int _pruning_method) :
    dataset(_dataset), pruning_method(_pruning_method) {

    pnn_count = 0;
    LOG << "PkNN pruning_method= " << DistanceIntervalSets::pruning_method_names[pruning_method] << "\n";

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



pair<ts_type,ts_type> PkNN::PNN_bounds(int uts_i) {

    pnn_count++;

    //LOG << "Updating PNN bounds for uts_i= " << uts_i << "\n";

    ts_type P_lb = 0, P_ub = 0;

    S.S[dataset.N][0].lb = last_lb; // update lowerbound for virtual object.

    for(unsigned int s_a = 0; s_a < S.S[uts_i].size(); s_a++) {


        ts_type p_lb = 1.0;
        ts_type p_ub = 1.0;


        for(vector< pair<int, pair<ts_type, ts_type> > >::iterator iter = Q.begin(); iter != Q.end(); iter++) {
            int uts_j = iter->first;

            if(uts_i ==uts_j)
                continue;

            ts_type sum_w_lb = 0;
            ts_type sum_w_ub = 0;

            for(unsigned int s_b = 0; s_b < S.S[uts_j].size(); s_b++) {

                if(S.S[uts_i][s_a].ub <=  S.S[uts_j][s_b].lb) { // lb
                    sum_w_lb += S.S[uts_j][s_b].w;
                }

                if(S.S[uts_i][s_a].lb <= S.S[uts_j][s_b].ub) { // ub
                    sum_w_ub += S.S[uts_j][s_b].w;
                }
            }


            p_lb *= sum_w_lb;
            p_ub *= sum_w_ub;

        }

        P_lb += S.S[uts_i][s_a].w * p_lb;
        P_ub += S.S[uts_i][s_a].w * p_ub;

    }

    assert(P_lb <= P_ub+ 1e-10);

    //LOG << "Plb,Pub = " << P_lb << " " << P_ub << "\n";
    assert(P_lb >= 0 && P_lb <=1 + 1e-10);
    assert(P_ub >= 0 && P_ub <=1 + 1e-10);

    return make_pair(P_lb, P_ub);

}





struct mycompare {
    bool operator()(intts_type const &a, intts_type const &b) {
        return a.second >b.second;
    }
};

struct mycompare_first {
    bool operator()(PBounds const &a, PBounds const &b) {
        return a.first  >b.first;
    }
};

struct mycompare_second {
    bool operator()(PBounds const &a, PBounds const &b) {
        return a.second >b.second;
    }
};














pair<int,int> PkNN::R1(int uts_i) { // lowerbound
    ts_type score_best = -1;
    pair<int,int> best = make_pair(-1,-1);


    for(unsigned int a = 0; a < S.S[uts_i].size(); a++) {

        if(S.S[uts_i][a].lb == S.S[uts_i][a].ub)
            continue;

        ts_type score = 0;

        for(vector< pair<int, pair<ts_type, ts_type> > >::iterator iter = Q.begin(); iter != Q.end(); iter++) {
            int uts_j = iter->first;

            if(uts_i == uts_j)
                continue;
            if(uts_j == dataset.N) // we never refine the vitual candidate
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

    //  LOG << "R1 score_best= " << score_best << " uts_i= " << uts_i << " ref= " << best << "\n";

    //LOG <<"\n";

    return best;
}

pair<int,int> PkNN::R3(int uts_i) {
    ts_type score_best = -1;
    pair<int,int> best = make_pair(-1,-1);

    for(vector< pair<int, pair<ts_type, ts_type> > >::iterator iter = Q.begin(); iter != Q.end(); iter++) {
        int uts_j = iter->first;


        if(uts_i == uts_j)
            continue;
        if(uts_j == dataset.N) // we never refine the vitual candidate
            continue;

        for(unsigned int b = 0; b < S.S[uts_j].size(); b++) {

            if(S.S[uts_j][b].lb == S.S[uts_j][b].ub)
                continue;

            ts_type score = 0;
            for(unsigned int a = 0; a < S.S[uts_i].size(); a++) {
                if(S.S[uts_i][a].ub > S.S[uts_j][b].lb &&
                        S.S[uts_i][a].lb < S.S[uts_j][b].ub)
                    score+= S.S[uts_i][a].w; //* (S.S[uts_i][a].ub - S.S[uts_i][a].lb);
            }
            score *= S.S[uts_j][b].w * (S.S[uts_j][b].ub - S.S[uts_j][b].lb);

            if(score > score_best) {
                score_best = score;
                best = make_pair(uts_j, b);
            }
        }
    }

    //   LOG << "R3 score_best= " << score_best  << " uts_i= " << uts_i << " ref= " << best << "\n";


    return best;
}





vector< pair<int,int> > PkNN::find_splits_opt(vector<int> C) {
    vector< pair<int,int> > R;


    for(vector<int>::iterator iter = C.begin(); iter != C.end(); iter++) {

        int uts_i = *iter;

        vector< pair<int,int> > refs;

        refs.push_back(R1(uts_i));
        refs.push_back(R3(uts_i));

        //  LOG << "refs: " << refs << "\n";

        for(vector< pair<int,int> >::iterator iter = refs.begin();
            iter != refs.end(); iter++) {
            pair<int,int> ref = *iter;

            if(ref.second == -1)
                continue;

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



void PkNN::dist_refines(vector< pair<int,int> > R, const TS &query) {
    S.dist_refines(dataset, query, R);
}




struct mycompare_second_inverse {
    bool operator()(std::pair<int,ts_type> const &a,std::pair<int,ts_type> const &b) {
        return a.second < b.second;
    }
};

struct mycompare_second_second {
    bool operator()(pair<int, pair<ts_type, ts_type> > const &a, pair<int, pair<ts_type, ts_type> > const &b) {
        return a.second.second > b.second.second;
    }
};



void PkNN::init_candidates(int K, const TS &query, vector<int> &candidates) {

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
        ts_type critical_distance_ub = S.S[candidates_all[K-1]][0].ub; // approx upper bound
        for(vector<int>::iterator iter = candidates_all.begin() ; iter != candidates_all.end() ; iter++) {
            if(S.S[*iter][0].lb <= critical_distance_ub) {
                //      LOG  << "adding to candidates uts_i= " << *iter << " dist bounds= " << S.S[*iter][0].lb << " " << S.S[*iter][0].ub << "\n";
                candidates.push_back(*iter);
            }
        }
    }

  //  ((Dataset *)(&dataset))->retain(candidates); // can be used , only if the loaded dataset is not used more than once.


}


void PkNN::init_mindist(int K, const TS &query, vector< pair<int, ts_type> > & min_dist) {

    vector<int > candidates;
    init_candidates(K, query, candidates);

    int size = candidates.size();
    for(int candidate_i = 0 ; candidate_i < size; candidate_i++) {
        int &uts_i = candidates[candidate_i];
        assert(S.S[uts_i].size() == 1);
        min_dist.push_back(make_pair(uts_i, S.S[uts_i][0].lb));
    }

    update_mindist(min_dist);
}

void PkNN::update_mindist(vector< pair<int, ts_type> > & min_dist) {
    sort(min_dist.begin(), min_dist.end(), mycompare_second_inverse());
    //LOG << "sorted min_dist= " << min_dist << "\n";
}



bool PkNN::outer_subregion(ts_type last_lb, pair<int,int> distint) {
    // uts_i is o_l in ihab's paper.

    return S.S[distint.first][distint.second].ub > last_lb;

}

void PkNN::update_priority_queue() {
    nth_element(Q.begin(), Q.begin(), Q.end(), mycompare_second_second());

}



void PkNN::init_virtual() {
    // create and add distance interval for virtual candidate.
    DistanceInterval d;
    d.lb = 0;
    d.ub = 1e20;
    d.w = 1;
    d.tosplit = false;
    DistanceIntervals distints;
    distints.push_back(d);
    S.S.push_back(distints); // S.S[dataset.N] represents virtual candidate.

    assert((int)S.S.size()  == dataset.N + 1);
    Q.push_back(make_pair(dataset.N, make_pair(0,1))); // adding virtual candidate.
}





vector< pair<int, pair<ts_type, ts_type> > > PkNN::PkNN_eval(const TS &query, int K) {
    int refinements_count = 0;

    ElapsedTime time;
    time.begin();

    vector< pair<int, ts_type> > min_dist; // min dist order
    vector< pair<int, pair<ts_type, ts_type> > > ret; // result set

    // init priority lists
    init_mindist(K, query, min_dist); //...using S initialization.

    //LOG << "mindist= " << min_dist << "\n";

    LOG << "TIME1 " << time.get() << "\n";

    init_virtual();

    //pair<int,int> pnn_to_udpate = make_pair(0,-1); // first reserved for virtual, second for new added candidate.

    int L = 0;

    LOG << "TIME2 " << time.get() << "\n";

    while((int)ret.size() < K) {

        L++;
        //LOG << "LOOP\n";

        assert(Q.size() > 0);

        //      LOG << "Q= " << Q << "\n";

        vector< int > pnn_updates; // uts_i to add to Q and to update.

        int uts_i = Q[0].first;
        //    LOG << "first is uts_i= " << uts_i << " upper_bound= " << Q[0].second.second << "\n";
        Q.erase(Q.begin());

        if(! (uts_i == dataset.N && min_dist.size() == 0)) // we add the virtual candidate only if not all candidates have been considered
            pnn_updates.push_back(uts_i);

        if(uts_i == dataset.N && min_dist.size() > 0) {


            int uts_j = min_dist[0].first;
            last_lb = min_dist[0].second;
            min_dist.erase(min_dist.begin());

            pnn_updates.push_back(uts_j); // candidate to be updated

        } else {

            if(uts_i == dataset.N) {
                // top candidate is the virtual candidate and all candidates added to Q.
                // we must take 2nd in the list.
                assert(Q.size() > 0);
                update_priority_queue();
                uts_i  = Q[0].first;
                Q.erase(Q.begin()); // this is the 2nd element! we have already removed the first one.
                pnn_updates.push_back(uts_i);

            }

            // at this point, uts_i granted to NOT be the virtual candidate.

            assert(uts_i != dataset.N);
            vector<int> C;
            C.push_back(uts_i);
            vector< pair<int,int> > R;
            //      LOG << "C= " << C << "\n";
            R = find_splits_opt(C);

            refinements_count+= R.size();

            if(R.size() == 0) {
                LOG << "Watning: BREAK\n";
                break;
            }

            //            assert(R.size() > 0);
            if(outer_subregion(last_lb, R[0]) && (int)min_dist.size() > 0) {
                int uts_j = min_dist[0].first;
                last_lb = min_dist[0].second;
                min_dist.erase(min_dist.begin());
                pnn_updates.push_back(uts_j);
            }

            // refinement
            dist_refines(R, query);

        }


        // now, add elements to Q and update their probab bounds
        for(vector< int >::iterator iter = pnn_updates.begin(); iter != pnn_updates.end() ; iter++) {
            //     LOG << "pnn_updates: adding " << *iter << "\n";
            Q.push_back(make_pair(*iter, make_pair(-1,-1)));
        }
        for(unsigned int i = 0 ;  i < pnn_updates.size(); i++)
            Q[Q.size()-i-1].second = PNN_bounds(Q[Q.size()-i-1].first); // we must have all acive candidates already in Q to update their bounds.

        // LOG << "Q after adds: " << Q << "\n";
        update_priority_queue();


        bool best = true;
        for(vector< pair<int, pair<ts_type, ts_type> > >::iterator iter = Q.begin() ; iter != Q.end() ; iter++) {
            pair< int, pair<ts_type,ts_type> > bounds_j = *iter;
            if(Q[0].first == bounds_j.first)
                continue;

            if(Q[0].second.first < bounds_j.second.second) {
                best  = false;
                break;
            }
        }


        if(best) {
            assert(Q[0].first != dataset.N);
            ret.push_back(Q[0]);

            // we cannot eliminate it! as it is still an "active" element in refiments.
            // we make sure that we will never pick it again but we keep it active.
            Q[0].second.first = -2;
            Q[0].second.second = -2;
            update_priority_queue();
            //Q.erase(Q.begin());
        }

    }

    LOG << "TIME " << time.get() << "\n";

    //LOG << "Q= " << Q << "\n";
    //LOG << "ret= " << ret << "\n";

    LOG << "refinements_count= " << refinements_count << "\n";
    LOG << "N= " << dataset.N << " Qsize= " << Q.size() << "\n";
    LOG << "loops_count= " << L << "\n";
    LOG << "pnn_count= " << pnn_count << "\n";
    //LOG << "Q= " << Q << "\n";


    return ret;
}






void PkNN::eval_analyze(const TS &query, int K) {
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

vector< pair<int, pair<ts_type,ts_type> > > PkNN::eval(const TS &query, int K) {

    LOG << "=== PkNN begin ===\n";
    vector< pair<int, pair<ts_type,ts_type> > > ret = PkNN_eval(query, K);

    LOG << "PkNN result set: ";
    for(vector<  pair<int, pair<ts_type,ts_type> > >::iterator iter = ret.begin(); iter != ret.end(); iter++) {
        int uts_i = iter->first;
        LOGL << uts_i << ":[" << iter->second.first << "," << iter->second.second << "] ";
    }
    LOGN;

    LOG << "=== PkNN end ===\n\n";

    return ret;
}






int PkNN::classify(const TS &query) {

    vector< pair<int, pair<ts_type,ts_type> > > PkNN_result = eval(query, 1);

    assert(PkNN_result.size() > 0);
    int nn_class_label = dataset.utss[PkNN_result[0].first].class_label;

    return nn_class_label;

}
