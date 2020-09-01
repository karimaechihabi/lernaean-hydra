#include "ts.h"
#include "common.h"
#include "stats.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <vector>
#include <sstream>
#include <math.h>

using namespace std;
using namespace boost;


TS::TS()
{
}


TS::TS(int n)
{

    values.resize(n);

}


/*
ts_type *TS::double() {
    ts_type *x=new ts_type[values.size()];
    for(unsigned int i = 0 ; i < values.size();i++)
        x[i] = values[i];
    return x;
}
*/
TS TS::reversed() const {

    TS ts;
    ts.values = values;
    reverse(ts.values.begin() , ts.values.end());
    return ts;

}



void TS::normalize() {

    ts_type sample_mean = 0;
    ts_type sample_stddev = 0;

    unsigned int size = values.size();

    for(unsigned int t = 0 ; t < size; t++)
        sample_mean+= values[t];

    //LOG << "xxxx " << sample_mean << "\n";

    sample_mean /= ((ts_type)size);

    //LOG << "yyy " << sample_mean << "\n";


    for(unsigned int t = 0 ; t < size; t++)
        sample_stddev+= POW2(sample_mean - values[t]);

    sample_stddev /= ((ts_type)size);

    if(sample_stddev == 0) // special case when 'n' == 1 . the normalized TS will be a single zero valued point.
        sample_stddev = 1;

    sample_stddev = sqrt(sample_stddev);

    //LOG << "sample_mean= " << sample_mean << " sample_stddev= " << sample_stddev << "\n";

    assert(!isnan(sample_stddev));
    assert(!isnan(sample_mean));

    for(unsigned int t = 0 ; t < size; t++)
        values[t] = (values[t] - sample_mean) / sample_stddev;


}


void TS::loadFromFile(ifstream &f, int n) {
    ts_type series[n];
    stats_count_partial_input_time_start();
    f.read(reinterpret_cast<char *>(series), sizeof(ts_type)*n);
    stats_count_partial_input_time_end();
    values = vector<ts_type>(series, series+n);
}

void TS::loadFromString(string &s) {

    stringstream  ss(s);
    ts_type value;

    while(!ss.eof()) {
        ss >> value;
        values.push_back(value);
    }

}

void TS::dumpToFile(ofstream &ofs) const {

    ts_type series[values.size()];
    std::copy(values.begin(), values.end(), series);
    ofs.write(reinterpret_cast<char *>(series), sizeof(ts_type)*values.size());

}



std::ostream& operator << (std::ostream& out, const TS& ts) {

    out << ts.values;
    return out;
}



ts_type TS::dist(const TS &ts) const {

    assert(values.size() == ts.values.size());

    ts_type ret = 0;

    unsigned int size = values.size();

    for(unsigned int i = 0; i < size; i++)
        ret += POW2(values[i] - ts.values[i]);

    ret= sqrt(ret);

    //LOG << "me= " << values << "\n";
    //LOG << "ts= " << ts.values << "\n";

    assert(!isnan(ret));

    return ret;
}


ts_type TS::dist(const TS &ts, ts_type early_abandoning_threshold) const {

    assert(values.size() == ts.values.size());

    ts_type ret = 0;

    for(unsigned int i = 0; i < values.size(); i++) {
        ret += POW2(values[i] - ts.values[i]);
        if(ret > early_abandoning_threshold)
            break;
    }

    ret= sqrt(ret);

    return ret;
}


ts_type TS::dist2(const TS &ts, ts_type early_abandoning_threshold) const {

    assert(values.size() == ts.values.size());

    ts_type ret = 0;

    int size = values.size() ;

    for(int i = 0; i < size; i++) {
        ret += POW2(values[i] - ts.values[i]);
        if(ret > early_abandoning_threshold)
            break;
    }

    // no sqrt !

    return ret;
}



ts_type TS::dist2cut(const TS &ts, ts_type early_abandoning_threshold, int s) const {

    assert(values.size() == ts.values.size());

    ts_type ret = 0;

    for(int i = 0; i < s; i++) {
        ret += POW2(values[i] - ts.values[i]);
        if(ret > early_abandoning_threshold)
            break;
    }

    // no sqrt !

    return ret;
}




TS TS::avg(const TS &ts) const {

    TS ts_avg;

    assert(ts.values.size() == values.size());

    for(unsigned int l = 0 ; l < values.size() ; l++)
        ts_avg.values.push_back((ts.values[l] + values[l]) / 2.0);

    return ts_avg;

}


vector<ts_type> TS::dists(const vector<TS> &tss) const {
    vector<ts_type> dists;

    for(unsigned int i = 0; i < tss.size(); i++)
        dists.push_back(dist(tss[i]));

    return dists;
}

TS TS::lb(const TS &ts) const {

    TS ret;

    assert(values.size() == ts.values.size());

    for(unsigned int i  = 0; i < values.size(); i++) {
        if(values[i] < ts.values[i])
            ret.values.push_back(values[i]);
        else
            ret.values.push_back(ts.values[i]);
    }

    return ret;

}

TS TS::ub(const TS &ts) const {

    TS ret;

    assert(values.size() == ts.values.size());

    for(unsigned int i  = 0; i < values.size(); i++) {
        if(values[i] > ts.values[i])
            ret.values.push_back(values[i]);
        else
            ret.values.push_back(ts.values[i]);
    }

    return ret;

}

