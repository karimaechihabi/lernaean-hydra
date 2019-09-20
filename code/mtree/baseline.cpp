#include "baseline.h"
#include <algorithm>
#include "common.h"

using namespace std;
using namespace boost;


Baseline::Baseline(const Dataset &_dataset) : dataset(_dataset)
{
}


Dists Baseline::eval_dists(const TS &query) {
    Dists dists;

    //LOG << "Determining distances...\n";
    for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
        dists.push_back(dataset.utss[uts_i].dist(query));
        // LOG << "Distances: " << "d(Q,X_" << uts_i << ")= " << dists[uts_i] << "\n";
    }

    return dists;
}



ts_type Baseline::PNN(const TS &query, int uts_i) {

    //LOG << uts_i << "\n";

    ts_type probab = 0.0;

    for(int s = 0; s < dataset.m; s++) {

        ts_type p = 1;
        for(int j = 0; j < dataset.N; j++) {
            if(uts_i ==j)
                continue;
            ts_type count = 0;
            for(int s2 = 0; s2 < dataset.m; s2++) {
                if(dists[uts_i][s] < dists[j][s2])
                    count++;
            }
            p *= count/ ((ts_type) dataset.m);
        }

        p *= 1.0/ ((ts_type) dataset.m);
        probab+= p;

    }

    return probab;

}

struct mycompare {
    bool operator()(intts_type const &a, intts_type const &b) {
        return a.second >b.second;
    }
};


vector< pair<int,ts_type> > Baseline::PkNN(const TS &query, int K) {

    vector< pair<int,ts_type>  > probs;

    //LOG "Query: " << query << "\n";

    dists = eval_dists(query);

    for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
        pair<int,ts_type> ret = make_pair(uts_i, PNN(query, uts_i));
        if(ret.second > 0)
            probs.push_back(ret);
    }

    sort(probs.begin(), probs.end(), mycompare());
    //LOG << "PkNN sorted list: " << probs << "\n";

    if(probs.size() > (unsigned int)K) {
        probs.erase(probs.begin() + K, probs.end());
    }

    //LOG << "Baseline result set: " << probs << "\n";



    return probs;


}


vector< pair<int,ts_type> > Baseline::eval(const TS &query, int K) {

    LOG << "=== Baseline begin ===\n";
    vector< pair<int,ts_type> > ret = PkNN(query, K);
    LOG << "=== Baseline end ===\n\n";

    return ret;

}
