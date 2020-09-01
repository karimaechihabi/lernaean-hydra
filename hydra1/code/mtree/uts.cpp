#include "uts.h"
#include <string>
#include "common.h"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"


using namespace std;
using namespace boost;

UTS::UTS(int _id) : id(_id), m(0), n(0), class_label(-1)
{
}

UTS::UTS(int _id, int _m, int _n) : id(_id), m(_m), n(_n), class_label(-1)
{

    samples.resize(m, TS(n));

}


void UTS::normalize() {

    for(int sample_i = 0 ; sample_i < m; sample_i++)
        samples[sample_i].normalize();

}


ts_type UTS::dist_uema(const TS &ts) const {
    TS uts_avg = this->avg();
    TS uts_stddev = this->samples_stddev();

    ts_type dist = 0;

    int w = 5;

    for(int t = 0 ; t < this->n; t++) {
        ts_type mean = 0;
        ts_type count = 0;
        for(int t2 = t -w; t2 <= t+w; t2++) {
            if(t2 < 0 ||t2 >= this->n)
                continue;

            ts_type weight = exp(-ABS(t2-t)) / uts_stddev.values[t2];
            //LOG << "weight " << weight <<  " " << uts_stddev.values[t2] << "\n";
            mean+= uts_avg.values[t] * weight;
            count+= weight;

        }

        mean /= count;

        dist += pow(mean - ts.values[t], 2);

    }

    return sqrt(dist);
}




TS UTS::random_mix_pivot() const {
    TS ts ;

    boost::variate_generator< boost::mt19937, boost::uniform_int<> >
            random_sample(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0,m-1));

    for(int l = 0 ; l < n ; l++) {
        ts.values.push_back(samples[random_sample()].values[l]);
    }

    return ts;

}

TS UTS::random_mvp_pivot() const {
    TS ts ;

    boost::variate_generator< boost::mt19937, boost::uniform_int<> >
            random_sample(boost::mt19937(RANDOM_SEED), boost::uniform_int<>(0,m-1));

    int random_sample_i = random_sample();

    int best_i = random_sample_i;
    ts_type largest_distance = 0;

    for(int l = 0 ; l < m ; l++) {
        if(l == random_sample_i)
            continue;

        ts_type dist = samples[random_sample_i].dist(samples[l]);
        if(dist > largest_distance) {
            best_i = l;
            largest_distance = dist;
        }

    }



    return samples[best_i];

}


void UTS::loadFromFile(ifstream &f, int m, int n) {

    ts_type cl;
    f.read(reinterpret_cast<char *>(&cl), sizeof(ts_type));

    class_label = cl;

    //LOG << "class_label= " << class_label << "\n";

    for(int sample_i = 0 ; sample_i < m ; sample_i++) {
        TS sample;
        sample.loadFromFile(f, n);

        if(sample_i > 100)
            continue;

        samples.push_back(sample);
    }


    update_counters();

}

void UTS::loadFromFileTXT(ifstream &ifs) {

    string line;

    while (getline(ifs, line)) {
        trim(line);

        if(line.size() == 0)
            continue;

        if(line.compare("--") == 0)
            break;

        TS sample;
        sample.loadFromString(line);
        samples.push_back(sample);

    }

    update_counters();


}




void UTS::dumpToFile(ofstream &ofs) const {

    ts_type cl = class_label;

    ofs.write(reinterpret_cast<const char *>(&cl), sizeof(ts_type));

    for(int sample_i = 0 ; sample_i < m ; sample_i++) {
        samples[sample_i].dumpToFile(ofs);
    }

}



std::ostream& operator << (std::ostream& out, const UTS& uts) {

    for(int sample_i = 0; sample_i < uts.m; sample_i++) {
        out << "#" << uts.id << " (L" << uts.class_label << ") " << sample_i << "/" << (uts.m-1) << ": " <<  uts.samples[sample_i] << "\n";
    }

    return out;
}




vector<ts_type> UTS::dist(const TS &ts) const {
    vector<ts_type> ret(m);

    for(int sample_i = 0; sample_i < m; sample_i++) {
        ret[sample_i] = samples[sample_i].dist(ts);
    }


    return ret;
}


ts_type UTS::dist_mean(const TS &ts) const {
    ts_type mean = 0 ;

    for(unsigned int i = 0; i < samples.size(); i++) {
        mean+= samples[i].dist(ts);
    }
    mean /= (ts_type)m;

    ts_type mean2 = avg().dist(ts);
    return mean2;

    //ts_type mean2 = avg().dist(ts);
    //LOG << "mean1= " << mean << " mean2= " << mean2 << "\n";

    return mean;
}


TS UTS::lb() const {

    TS ret;

    for(int i  = 0; i < n; i++) {

        ts_type min = 1e20;

        for(int l = 0; l < m; l++) {
            if(samples[l].values[i] < min)
                min = samples[l].values[i];
        }

        ret.values.push_back(min);
    }

    return ret;

}



TS UTS::avg(vector<int> samples_subset) const {

    int size = samples_subset.size();

    TS ret(n);

    for(int i  = 0; i < n; i++) {

        ts_type point = 0;
        for(int sample_l = 0; sample_l < size; sample_l++)
            point+= samples[samples_subset[sample_l]].values[i];
        point /= ((ts_type)size);

        ret.values[i] = point;
    }

    return ret;
}


TS UTS::samples_stddev() const {
    TS uts_avg = this->avg();
    TS uts_stddev;

    for(int i  = 0; i < n; i++) {
        ts_type stddev = 0;
        for(int l = 0; l < m; l++)
            stddev += pow(samples[l].values[i] - uts_avg.values[i], 2);
        stddev = sqrt(stddev) / (n-1);
        uts_stddev.values.push_back(stddev);
    }

    return uts_stddev;

}


TS UTS::avg() const {

    if(m == 1)
        return samples[0];

    TS ret(n);

    for(int i  = 0; i < n; i++) {

        ts_type point = 0;
        for(int l = 0; l < m; l++)
            point+= samples[l].values[i];
        point /= ((ts_type)m);

        //ret.values.push_back(point);
        ret.values[i] = point;
    }

    return ret;

}




void UTS::update_counters() {

    if(samples.size() == 0) {
        n = 0;
        m = 0;
    } else {
        m = samples.size();
        n = samples[0].values.size();
    }

}


TS UTS::ub() const {

    TS ret;

    for(int i  = 0; i < n; i++) {

        ts_type max = -1e20;

        for(int l = 0; l < m; l++) {
            if(samples[l].values[i] > max)
                max = samples[l].values[i];
        }

        ret.values.push_back(max);
    }

    return ret;

}


vector<int> UTS::range_search(TS &query , ts_type  dist_threshold) {

    vector<int> result_set;

    for(int sample_i = 0 ; sample_i < m ; sample_i++)
        if(query.dist(samples[sample_i]) <= dist_threshold)
            result_set.push_back(sample_i);

    return result_set;
}
