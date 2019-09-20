#include "euclidean.h"
#include "common.h"

using namespace std;
using namespace boost;


Euclidean::Euclidean(const Dataset &_dataset) :   dataset(_dataset)
{    
}




struct mycompare_less {
    bool operator()(pair<int,double> const &a, pair<int,double> const &b) {
        return a.second < b.second;
    }
};



vector< pair<int, double> > Euclidean::eval(const TS &query, int K) {

    vector< pair<int, double> > ret ;

    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        const UTS &uts = dataset.utss[uts_i];
        ret.push_back(make_pair(uts_i, uts.dist_mean(query)));
    }

    sort(ret.begin(), ret.end(), mycompare_less());
    LOG << "Euclidaen sorted list: " << ret << "\n";


    if(ret.size() > (unsigned int)K) {
        ret.erase(ret.begin() + K, ret.end());
    }

    LOG << "Euclidean result set: " << ret << "\n";

    return ret;
}


int Euclidean::classify(const TS &query) {

    double min_dist = 1e5;
    int best_class = -1;

    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        const UTS &uts = dataset.utss[uts_i];

        double min = uts.dist_mean(query);

        //vector<double> dists = uts.dist(query);
        //double min = *min_element(dists.begin() , dists.end());

        //double x = 0;
        //for(int i = 0 ; i < dists.size() ; i++)
        //    x+= dists[i];
        //x/= (double(dists.size()));
        //min = x;


        if(min < min_dist) {
            min_dist = min;
            best_class = uts.class_label;
        }


    }

   // LOG << "XXX " << best_class << " " << min_dist << "\n";


    assert(best_class != -1);
    return best_class;


}


int Euclidean::classify_uema(const TS &query) {

    double min_dist = 1e5;
    int best_class = -1;

    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        const UTS &uts = dataset.utss[uts_i];

        double min = uts.dist_uema(query);

        if(min < min_dist) {
            min_dist = min;
            best_class = uts.class_label;
        }


    }

    assert(best_class != -1);
    return best_class;



}


int Euclidean::classify_bad(const TS &query) {

    double min_dist = 1e5;
    int best_class = -1;

    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        const UTS &uts = dataset.utss[uts_i];

        double min = uts.dist_mean(query);

        //vector<double> dists = uts.dist(query);
        //double min = *min_element(dists.begin() , dists.end());

        //double x = 0;
        //for(int i = 0 ; i < dists.size() ; i++)
        //    x+= dists[i];
        //x/= (double(dists.size()));
        //min = x;


        if(min < min_dist) {
            min_dist = min;
            best_class = uts.class_label;
        }


    }

   // LOG << "XXX " << best_class << " " << min_dist << "\n";


    assert(best_class != -1);
    return best_class;


}
