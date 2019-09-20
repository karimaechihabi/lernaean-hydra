#include "distints.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <vector>

#include <boost/algorithm/minmax.hpp>
#include <boost/algorithm/minmax_element.hpp>

using namespace std;
using namespace boost;


const char *DistanceIntervalSets::pruning_method_names[] = { "EXACT_LSCAN", "SPATIAL_LSCAN", "METRIC_LSCAN", "METRIC_MTREE", 0 };
const char *DistanceIntervalSets::pruning_metric_pivot_names[] = { "PIVOT_RANDOM", "PIVOT_MAXDIST", "PIVOT_KMEANS", 0 };



DistanceIntervalSets::DistanceIntervalSets() : activate_approx(false), approx_w_threshold(-1)
{
}


void DistanceIntervalSets::init_exact_bounds(const Dataset &dataset, const TS &query) {

    D.clear();
    D.resize(dataset.N);

    S.clear();

    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        init_dists(dataset, uts_i, query);
        sort(D[uts_i].begin(), D[uts_i].end());
        DistanceInterval distint;

        pair< vector<ts_type>::const_iterator, vector<ts_type>::const_iterator > result =
	  std::minmax_element(D[uts_i].begin(), D[uts_i].end());
        distint.lb = *(result.first);
        distint.ub = *(result.second);

        distint.w = 1.0;
        distint.tosplit = false;
        distint.index_lb = 0;
        distint.index_ub = dataset.m-1;
        distint.refs_count = 0;
        //    LOG << "S_i min=" << distint.lb << " max=" << distint.ub << "\n";
        DistanceIntervals distints;
        distints.push_back(distint);
        S.push_back(distints);
    }


    //    D.clear();
    //    D.resize(dataset.N);

}


vector< pair< pair<ts_type, ts_type>, ts_type> > DistanceIntervalSets::get(int uts_i) {

    vector< pair< pair<ts_type, ts_type>, ts_type> > ret;

    for(DistanceIntervals::iterator iter = S[uts_i].begin(); iter != S[uts_i].end(); iter++) {

        DistanceInterval distint_orig = *iter;

        pair< pair<ts_type, ts_type>, ts_type> distint;

        distint.first.first = distint_orig.lb;
        distint.first.second =distint_orig.ub;
        distint.second = distint_orig.w;

        ret.push_back(distint);

    }

    return ret;


}


bool DistanceIntervalSets::init_dist_prune(const Dataset &dataset, int uts_i, const TS &query, ts_type dist_ub) {
    // if the lower bound of all distance samples is GREATER than dist_ub, then we can prune uts_i.
    // returns TRUE if pruned, FALSE otherwise. If returns FALSE, then D[uts_i] is initialized.

    D[uts_i] = vector<ts_type>(dataset.m, 0);

    ts_type dist_ub2 = dist_ub*dist_ub;

    //int step = dataset.n;

    ts_type min;
    for(int t = 0 ; t < dataset.n ; t++) {
        min = 1e5;

        if(t == dataset.n-1) {
            min = 1e5;
            for(int sample_i = 0 ; sample_i < dataset.m ; sample_i++) {
                D[uts_i][sample_i]+= POW2(dataset.utss[uts_i].samples[sample_i].values[t] - query.values[t]);
                min = MIN(D[uts_i][sample_i], min);
            }
            if(min > dist_ub2) {
                return true;
            }
        } else {
            for(int sample_i = 0 ; sample_i < dataset.m ; sample_i++) {
                D[uts_i][sample_i]+= POW2(dataset.utss[uts_i].samples[sample_i].values[t] - query.values[t]);
            }
        }


    }

    for(int sample_i = 0 ; sample_i < dataset.m ; sample_i++) {
        D[uts_i][sample_i] = sqrt(D[uts_i][sample_i]);
    }

    pair< vector<ts_type>::const_iterator, vector<ts_type>::const_iterator > result =
      std::minmax_element(D[uts_i].begin(), D[uts_i].end());
    S[uts_i][0].lb = *(result.first);
    S[uts_i][0].ub = *(result.second);
    S[uts_i][0].tosplit = false;

    S[uts_i][0].refs_count++;

    return false;
}



void DistanceIntervalSets::dist_refine(const Dataset &dataset, int uts_i, int a, const TS &query) {

    assert(uts_i < dataset.N);

    if(D[uts_i].size() == 0) { // the 1st refinement consists in using the exact distance bounds.

        init_dists(dataset, uts_i, query);

        pair< vector<ts_type>::const_iterator, vector<ts_type>::const_iterator > result =
	  std::minmax_element(D[uts_i].begin(), D[uts_i].end());
        S[uts_i][a].lb = *(result.first);
        S[uts_i][a].ub = *(result.second);

        S[uts_i][a].tosplit = false;
        S[uts_i][a].refs_count++;
        return;
    } else {

        if(S[uts_i].size() == 1) {
            // second refinement! sort elements.
            sort(D[uts_i].begin(), D[uts_i].end());
        }



    }

    if(!S[uts_i][a].tosplit) {
        // it happens if we use PRUNING_EXACT_LSCAN in the init_distance_bounds function . we shouldnt split, otherwise the ub of the critical region is wrong!
        return;
    }



    //    LOG << "S[uts_i] = " << get(uts_i) << "\n";
    //   LOG << "Refining S uts_i=" << uts_i << " a=" << a << ":  " << get(uts_i)[a] << "\n";
    //  LOG << "D: " << D[uts_i] << "\n";

    DistanceInterval distint = S[uts_i][a];

    // LOG << "DISTINT uts_i= " << uts_i << " refcount= " << distint.refs_count << "\n";

    //activate_approx = true;
    //approx_w_threshold = 0.1 ; // equivalent to 100 buckets.


    if(activate_approx && distint.w <= approx_w_threshold) { // approximation
        FATAL("not active.");
        assert(approx_w_threshold > 0);
        approx_count++;
        DistanceInterval distint_new;
        distint_new.index_lb= distint.index_lb;
        distint_new.index_ub= distint.index_ub;
        distint_new.lb = D[uts_i][distint_new.index_ub] + (D[uts_i][distint_new.index_ub] - D[uts_i][distint_new.index_lb]) / 2.0;
        distint_new.ub = distint_new.lb;
        distint_new.w = distint.w;
        distint_new.tosplit = false;
        distint_new.refs_count = distint.refs_count+1;
        S[uts_i][a] = distint_new;
        return;
    }

    //ts_type mid_point =  (distint.lb + distint.ub) / 2.0;
    ts_type mid_point = D[uts_i][distint.index_lb] + (D[uts_i][distint.index_ub] - D[uts_i][distint.index_lb]) / 2.0;

    //LOG << "mid point= " << mid_point << " dist_refine index_lb= " << distint.index_lb << " index_ub= " << distint.index_ub << "\n";

    int split_index = binary_search_le(uts_i, mid_point, distint.index_lb, distint.index_ub);

    //LOG << "split_index= " << split_index << "\n";

    DistanceInterval distint_left;
    DistanceInterval distint_right;

    if(split_index == -1 || split_index == distint.index_ub) { //  all samples in one interval (we use left)
        distint_left.index_lb= distint.index_lb;
        distint_left.index_ub= distint.index_ub;
        distint_left.lb = D[uts_i][distint_left.index_lb];
        distint_left.ub = D[uts_i][distint_left.index_ub];
        distint_left.w = distint.w;
        distint_left.tosplit = false;
        distint_left.refs_count = distint.refs_count+1;
        distint_right.w = 0; // this ensures that we do not add a left interval later in this function.
    } else { // some samples in left interval, some samples in right interval
        distint_left.index_lb= distint.index_lb;
        distint_left.index_ub= split_index;
        distint_left.lb = D[uts_i][distint_left.index_lb];
        distint_left.ub = D[uts_i][distint_left.index_ub];
        distint_left.tosplit = false;
        distint_left.refs_count = distint.refs_count+1;

        ts_type w_left = ((ts_type)(distint_left.index_ub - distint_left.index_lb + 1)) /
                ((ts_type)(distint.index_ub - distint.index_lb + 1));
        //LOG << "w_left= " << w_left << "\n";

        distint_left.w = distint.w * w_left;
        distint_right.index_lb= split_index+1;
        distint_right.index_ub= distint.index_ub;
        distint_right.lb = D[uts_i][distint_right.index_lb];
        distint_right.ub = D[uts_i][distint_right.index_ub];
        distint_right.tosplit = false;
        distint_right.refs_count = distint.refs_count+1;
        distint_right.w = distint.w * (1.0 - w_left);
    }

    //LOG << "distint_left: index_lb= " << distint_left.index_lb << " index_ub= " << distint_left.index_ub << " w= " << distint_left.w << "\n";
    //LOG << "distint_right: index_lb= " << distint_right.index_lb << " index_ub= " << distint_right.index_ub << " w= " << distint_right.w << "\n";

    assert(distint_left.w > 0 || distint_right.w > 0);

    if(distint_left.w > 0 && distint_right.w > 0) {
        // overwrite S[uts_i][a] with left interval
        S[uts_i][a] = distint_left;
        S[uts_i].push_back(distint_right);
    } else {
        // overwrite S[uts_i][a] with left interval.
        // there are either left,right intervals, or only left interval. cannot be that there is only right interval.
        S[uts_i][a] = distint_left;
    }

    //    LOG << "After refinement: S[uts_i] = " << get(uts_i) << "\n";

}


// returns the largest index of point X s.t. X <= key (within point indexes index_lb, index_ub).
// if key lower than lowest index, then returns -1.
int DistanceIntervalSets::binary_search_le(int uts_i, ts_type key, int index_lb, int index_ub) {
    int left = index_lb;
    int right = index_ub;
    int mid = 0;

    //LOG << "starting left= " << left << " right= " << right << "\n";

    if(key > D[uts_i][index_ub])
        return index_ub;

    if(key < D[uts_i][index_lb])
        return -1;

    while (left < right) {
        mid = left + (right - left)/2;

        //        LOG << "mid= " << mid << "\n";

        assert(index_lb <= mid && mid <= index_ub);
        assert(index_lb <= left && left <= index_ub);
        assert(index_lb <= right && right <= index_ub);

        if (D[uts_i][mid] < key){
            left = mid+1;
            //          LOG << "left= " << left << "\n";
        } else if (D[uts_i][mid] > key){
            right = mid-1;
            //        LOG << "right= " << right << "\n";
        }
        else { // key == mid point
            while(mid+1 <= index_ub && D[uts_i][mid+1] == key) // take care of duplicates...
                mid++;
            return mid;
        }
    }

    //    LOG << "end left= " << left << " right= " << right << "\n";

    while(right-1 >= index_lb && D[uts_i][right] > key)
        right--;

    assert(D[uts_i][right] <= key);
    if(right < index_ub)
        assert(D[uts_i][right+1] > key);

    return right;

}


void DistanceIntervalSets::dist_refines(const Dataset &dataset, const TS &query, vector< pair<int, int> > R) {
    // split intervals

    for(vector< pair<int,int> >::iterator iter = R.begin(); iter != R.end(); iter++) {
        dist_refine(dataset, iter->first, iter->second, query);
    }

}





bool DistanceIntervalSets::overlaps(int uts_i, int a, int uts_j, int b) {

    if( WITHIN(S[uts_i][a].lb, S[uts_j][b].lb, S[uts_j][b].ub) ||
            WITHIN(S[uts_i][a].ub, S[uts_j][b].lb, S[uts_j][b].ub) ||
            WITHIN(S[uts_j][b].lb, S[uts_i][a].lb, S[uts_i][a].ub)) {
        //    LOG << "Intervals " << S[uts_i][a].lb << "," << S[uts_i][a].ub << " AND " <<
        //        S[uts_j][b].lb << "," << S[uts_j][b].ub << " overlap\n";
        return true;
    } else {
        // LOG << "Intervals " << S[uts_i][a].lb << "," << S[uts_i][a].ub << " AND " <<
        //    S[uts_j][b].lb << "," << S[uts_j][b].ub << " DO NOT overlap\n";
        return false;
    }


}

void DistanceIntervalSets::init_exact_bounds_max_refinement(const Dataset &dataset, const TS &query) {

    D.clear();
    D.resize(dataset.N);

    S.clear();

    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        DistanceIntervals distints;
        init_dists(dataset, uts_i, query);
        sort(D[uts_i].begin(), D[uts_i].end());

        for(int sample_i = 0 ; sample_i < dataset.m; sample_i+=1) {
            DistanceInterval distint;
            distint.lb = D[uts_i][sample_i];
            distint.ub =D[uts_i][sample_i];
            distint.w = 1.0 / ((ts_type)dataset.m);
            distint.tosplit = false;
            distint.index_lb = 0;
            distint.index_ub = dataset.m-1;
            distint.refs_count = 0;
            distints.push_back(distint);
        }

        S.push_back(distints);
    }

}

void DistanceIntervalSets::init_dists(const Dataset &dataset, int uts_i, const TS &query) {
    //vector<ts_type> dists = dataset.utss[uts_i].dist(query);
    //sort(dists.begin(), dists.end());
    D[uts_i] = dataset.utss[uts_i].dist(query);

}

void DistanceIntervalSets::init_spatial_bounds(const Dataset &dataset, const TS &query) {

    D.clear();
    D.resize(dataset.N);


    assert(envelopes.size() ==(unsigned int) dataset.N);

    S.clear();

    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        DistanceInterval d;


        const TS &lb = envelopes[uts_i].first;
        const TS &ub = envelopes[uts_i].second;

        d.lb = 0;
        d.ub = 0;
        d.w = 1;
        d.tosplit = false;
        d.index_lb = 0;
        d.index_ub = dataset.m-1;
        d.refs_count = 0;


        for(int t = 0 ; t < dataset.n; t++) {

            ts_type d1 = POW2(query.values[t] - lb.values[t]);
            ts_type d2 = POW2(query.values[t] - ub.values[t]);

            if(query.values[t] < lb.values[t] || query.values[t] > ub.values[t]) {
                d.lb+= MIN(d1,d2);
            }

            d.ub += MAX(d1,d2);

        }

        d.lb = sqrt(d.lb);
        d.ub = sqrt(d.ub);

        //        ts_type optimal_lb = *min_element(D[uts_i].begin(), D[uts_i].end());
        //        ts_type optimal_ub = *max_element(D[uts_i].begin(), D[uts_i].end());

        //      assert(d.lb <= optimal_lb);
        //    assert(d.ub >=  optimal_ub);


        DistanceIntervals distints;
        distints.push_back(d);
        S.push_back(distints);
    }


}


void DistanceIntervalSets::init_metric_bounds(const Dataset &dataset, const TS &query) {

    D.clear();
    D.resize(dataset.N);


    assert(pivots.size() == (unsigned int) dataset.N);

    S.clear();

    for(int uts_i = 0 ; uts_i < dataset.N; uts_i++) {
        DistanceInterval d;

        d.w = 1;
        d.tosplit = false;
        d.index_lb = 0;
        d.index_ub = dataset.m-1;
        d.refs_count = 0;

        for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
            ts_type pd = query.dist(pivots[uts_i][pivot_i]);

            if(pivot_i == 0) { // if first loop
                //d.lb = MAX3(0, pd - pivot_dists[uts_i][pivot_i].second, pivot_dists[uts_i][pivot_i].first - pd);
                d.lb = MAX(0, pd - pivot_dists[uts_i][pivot_i].second);
                d.ub = pd + pivot_dists[uts_i][pivot_i].second;
            } else {
                //d.lb = MAX(d.lb, MAX3(0, pd - pivot_dists[uts_i][pivot_i].second, pivot_dists[uts_i][pivot_i].first - pd));
                d.lb = MAX(d.lb, MAX(0, pd - pivot_dists[uts_i][pivot_i].second));
                d.ub = MIN(d.ub, pd + pivot_dists[uts_i][pivot_i].second);
            }
        }





        //ts_type optimal_lb = *min_element(D[uts_i].begin(), D[uts_i].end());
        //        ts_type optimal_ub = *max_element(D[uts_i].begin(), D[uts_i].end());

        //assert(d.lb <= optimal_lb);
        //assert(d.ub >=  optimal_ub);

        DistanceIntervals distints;
        distints.push_back(d);
        S.push_back(distints);
    }



}


void DistanceIntervalSets::dumpToFile_S(const char *pathname) {

    ofstream fout;
    fout.open(pathname, ios::binary);

    LOG << "Duming S (distance interval sets) to file: " << pathname << "\n";

    if (!fout.is_open()) {
        FATAL("unable to open file " << pathname);
    }

    int N = S.size();

    fout.write(reinterpret_cast<char *>(&N), sizeof(int));

    for(int uts_i = 0 ; uts_i < N ; uts_i++) {

        int intervals_count_i = S[uts_i].size();

        fout.write(reinterpret_cast<char *>(&intervals_count_i), sizeof(int));

        for(int interval_i = 0; interval_i < intervals_count_i ; interval_i++) {
            DistanceInterval dist_int = S[uts_i][interval_i];
            fout.write(reinterpret_cast<char *>(&dist_int.lb), sizeof(ts_type));
            fout.write(reinterpret_cast<char *>(&dist_int.ub), sizeof(ts_type));
            fout.write(reinterpret_cast<char *>(&dist_int.w), sizeof(ts_type));
        }

    }

    fout.close();

}



void DistanceIntervalSets::dumpToFile_D(const char *pathname, const Dataset &dataset, const TS &query) {

    ofstream fout;
    fout.open(pathname, ios::binary);

    LOG << "Duming D (distance samples) to file: " << pathname << "\n";

    if (!fout.is_open()) {
        FATAL("unable to open file " << pathname);
    }

    fout.write(reinterpret_cast<const char *>(&dataset.N), sizeof(int));
    fout.write(reinterpret_cast<const char *>(&dataset.m), sizeof(int));

    for(int uts_i = 0 ; uts_i < dataset.N ; uts_i++) {
        init_dists(dataset, uts_i, query);
        for(int sample_i = 0 ; sample_i < dataset.m; sample_i++) {
            fout.write(reinterpret_cast<char *>(& D[uts_i][sample_i]), sizeof(ts_type));
        }

    }

    fout.close();

}


void DistanceIntervalSets::build_metric_mtree_index(const Dataset &dataset) {
  mtree.load(dataset, (ts_type)0.1);

}




void DistanceIntervalSets::build_spatial_index(const Dataset &dataset) {

    envelopes.clear();
    for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
        envelopes.push_back(make_pair(dataset.utss[uts_i].lb(), dataset.utss[uts_i].ub()));
    }

}

int DistanceIntervalSets::find_nearest_cluster(const Dataset &dataset, int uts_i, int sample_i, int *pivots, int n_pivots) {

    ts_type min_dist = 1e20;
    int nearest_i = -1;

    int s = MAX(dataset.n * 0.1,1);

    for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
        const TS &pivot = dataset.utss[uts_i].samples[pivots[pivot_i]];
        ts_type dist = dataset.utss[uts_i].samples[sample_i].dist2cut(pivot, min_dist, s); // dist2 ! no sqrt, so min_dist doesnt need POW2.
        if(dist <= min_dist) {
            min_dist = dist;
            nearest_i = pivot_i;
        }

    }

    return nearest_i;

}

void DistanceIntervalSets::avg_k_pivots(const Dataset &dataset, int uts_i, vector<TS> &uts_pivots, int n_pivots) {

    if(n_pivots == 1) {
        uts_pivots.push_back(dataset.utss[uts_i].avg());
        return;
    }


    boost::variate_generator< boost::mt19937, boost::uniform_int<> >
            randomly(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0, dataset.m-1));

    int pivots[n_pivots];


    bool selected[dataset.m];
    memset(selected, false, sizeof(bool)*dataset.m);
    for(int pivot_i = 0 ; pivot_i < n_pivots ; pivot_i++) {
        while(true) {
            int candidate_i = randomly();
            if(!selected[candidate_i]) {
                selected[candidate_i] = true;
                pivots[pivot_i] = candidate_i;
                break;
            }
        }
    }


    vector < int > clusters[n_pivots];

    for(int sample_i = 0 ; sample_i < dataset.m; sample_i++) {
        int nearest_cluster_i = find_nearest_cluster(dataset, uts_i, sample_i, pivots, n_pivots);
        clusters[nearest_cluster_i].push_back(sample_i);
    }

    for(int pivot_i = 0 ; pivot_i < n_pivots ; pivot_i++) {
        //LOG << "pivot_i= " << pivot_i << " size= " << clusters[pivot_i].size() << "\n";
        uts_pivots.push_back(dataset.utss[uts_i].avg(clusters[pivot_i]));
    }


}








void DistanceIntervalSets::kmeans_pivots(const Dataset &dataset, int uts_i, vector<TS> &uts_pivots, int n_pivots) {

    //LOG << "kmeans_pivots uts_i= " << uts_i << "\n";

    if(n_pivots == 1) {
        uts_pivots.push_back(dataset.utss[uts_i].avg());
        return;
    }


    uts_pivots.resize(n_pivots);


    // initialization : k random centroids.

    vector<TS> &centroids = uts_pivots;
    for(int pivot_i = 0 ; pivot_i < n_pivots ; pivot_i++)
        centroids[pivot_i] = dataset.utss[uts_i].samples[pivot_i];

    // clear assignments...
    int assignments[dataset.m];
    memset(assignments, 0, sizeof(int) * dataset.m);

    vector<int> clusters[n_pivots];


    // assignment step.

    bool converged = false;

    int iteration = 0 ;

    while(!converged) {

        iteration++;

   //     LOG << "iteration uts_i= " << uts_i << " count= " << iteration << "\n";

        converged = true;

        // clear clusters....
        for(int pivot_i = 0 ; pivot_i < n_pivots ; pivot_i++) {
            clusters[pivot_i].clear();
        }

        for(int sample_i = 0 ; sample_i < dataset.m; sample_i++) {
            ts_type min_dist = 1e20;
            int best_pivot_i = -1;
            for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
                ts_type dist = centroids[pivot_i].dist(dataset.utss[uts_i].samples[sample_i]);
                if(dist < min_dist) {
                    min_dist = dist;
                    best_pivot_i = pivot_i;
                }

            //    LOG << "best_pivot_i= " << best_pivot_i << " previous= " << assignments[sample_i] << "\n";
            }


            if(assignments[sample_i] != best_pivot_i) {
                converged = false;
            }

            assignments[sample_i] = best_pivot_i;
            clusters[best_pivot_i].push_back(sample_i);


        }

        //LOG << "ASSIGNMENTS: ";
        //for(int sample_i = 0 ; sample_i < dataset.m; sample_i++)
          //  cout << assignments[sample_i] << " " ;
        //cout << "\n";



        // update step.

        for(int pivot_i = 0 ; pivot_i < n_pivots ; pivot_i++) {
            centroids[pivot_i] = dataset.utss[uts_i].avg(clusters[pivot_i]);
        }


    }
}



void DistanceIntervalSets::build_metric_index(const Dataset &dataset, int pivots_method, int n_pivots) {

    this->n_pivots = n_pivots;

    pivot_dists.clear();
    pivots.clear();
    assert(dataset.m >= n_pivots);


    for(int uts_i = 0; uts_i < dataset.N; uts_i++) {
        vector<TS> uts_pivots;


        switch(pivots_method) {
        case PRUNING_METRIC_PIVOT_RANDOM:
            for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
                uts_pivots.push_back(dataset.utss[uts_i].samples[pivot_i]); // random pivot.
            }
            break;
        case PRUNING_METRIC_PIVOT_MAXDIST:
            for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
                uts_pivots.push_back(dataset.utss[uts_i].random_mvp_pivot());
            }
            break;
        //case PRUNING_METRIC_PIVOT_AVG:
          //  avg_k_pivots(dataset, uts_i, uts_pivots, n_pivots);
            //break;

        case PRUNING_METRIC_PIVOT_KMEANS:
            kmeans_pivots(dataset, uts_i, uts_pivots, n_pivots);
            break;

        default:
            FATAL("WTF!?");
        }

        // compute distance bounds between each pivot and all samples
        vector< pair<ts_type,ts_type> >uts_pivot_dists;


        for(int pivot_i = 0 ; pivot_i < n_pivots; pivot_i++) {
            vector<ts_type> dists = dataset.utss[uts_i].dist(uts_pivots[pivot_i]);

            pair< vector<ts_type>::const_iterator, vector<ts_type>::const_iterator > result =
	      std::minmax_element(dists.begin(), dists.end());

            uts_pivot_dists.push_back(make_pair(*(result.first), *(result.second)));
        }

        pivots.push_back(uts_pivots);
        pivot_dists.push_back(uts_pivot_dists);

    }

}


std::vector<int> DistanceIntervalSets::mtree_find_topk(const Dataset &dataset, const TS &query, int K) {



    pair< vector<int> , vector< pair< ts_type,ts_type> > >  candidates_bounds = mtree.find_topk(query, K);

    D.clear();
    D.resize(dataset.N);
    S.resize(dataset.N);

    for(unsigned int candidate_i = 0 ; candidate_i < candidates_bounds.first.size() ; candidate_i++) {

        DistanceInterval d;

        d.w = 1;
        d.tosplit = false;
        d.index_lb = 0;
        d.index_ub = dataset.m-1;
        d.refs_count = 0;
        d.lb = candidates_bounds.second[candidate_i].first;
        d.ub = candidates_bounds.second[candidate_i].second;

        DistanceIntervals distints;
        distints.push_back(d);

        S[candidates_bounds.first[candidate_i]] = distints;

    }

    return candidates_bounds.first;

}
