#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <vector>

#include "dataset.h"
#include "uts.h"
#include "common.h"
#include "elapsedtime.h"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include <boost/math/distributions/skew_normal.hpp>
#include <sys/stat.h>

using namespace std;
using namespace boost;

#define RANDOM_GENERATOR boost::lagged_fibonacci607
//#define RANDOM_GENERATOR boost::rand48
//rand48


Dataset::Dataset()
{
}


void Dataset::retain(std::vector<int> &candidates) {

    if((int)candidates.size() == N) // nothing to drop.
        return;

    for(int uts_i = 0 ; uts_i < N ; uts_i++) {
        utss[uts_i].class_label =0;
    }

    int size = candidates.size();

    for(int candidate_i = 0 ; candidate_i < size; candidate_i++) {
        utss[candidates[candidate_i]].class_label = 1;
    }

    for(int uts_i = 0 ; uts_i < N ; uts_i++) {
        if(utss[uts_i].class_label == 0) {
            utss[uts_i] = UTS(uts_i); // drop samples ! freeing memory...
        }
    }


}

Dataset::Dataset(const char *pathname) {
    loadFromFile(pathname);
}


Dataset::Dataset(UTS &uts) {

    for(int sample_i = 0 ; sample_i < uts.m ; sample_i++) {
        UTS uts2(sample_i);
        uts2.samples.push_back(uts.samples[sample_i]);
        uts2.update_counters();
        utss.push_back(uts2);

    }

    update_counters();

}


TS Dataset::walk(double stddev, int n) {


    TS ts;

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev));

    double prev = vsample_normal();


    while((int)ts.values.size() < n) {
        double point = prev + vsample_normal();
        ts.values.push_back(point);
        prev = point;
    }

    ts.normalize();

    return ts;
}


TS Dataset::walk() {
    return walk(1.0, n);
}

vector<TS> noisySamples(TS seed_series, int m, double mean, double stddev, double probab) {
    vector<TS> samples;

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(mean, stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal_outlier(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::uniform_real<> >
            vsample_uniform01(RANDOM_GENERATOR(RANDOM_SEED), boost::uniform_real<>(0,1));

    bool outlier;

    for(int sample_i = 0 ; sample_i < m; sample_i++) {
        TS sample;
        outlier = false;

        if(probab < 1.0 && vsample_uniform01() > probab)
            outlier = true;

        for(unsigned int l = 0; l < seed_series.values.size(); l++) {
            double v = seed_series.values[l];
            v+= vsample_normal();
            if(outlier)
                v+= vsample_normal_outlier();
            sample.values.push_back(v);
        }
        samples.push_back(sample);
    }

    return samples;


}



vector<TS> noisySamples2(TS seed_series, int m, double mean, double stddev,
                         double outlier_probab, double outlier_stddev) {

    // not used and expected to be...
    assert(mean == 0.0);
    assert(outlier_probab == 1.0);

    //LOG << "Adding perturbation  : m= " << m << " mean= " << mean << " stddev= " << stddev <<
    //     " outlier_probab= " << outlier_probab << " outlier_stddev= " << outlier_stddev << "\n";

    vector<TS> samples;


    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal_outlier(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, outlier_stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::uniform_real<> >
            vsample_uniform01(RANDOM_GENERATOR(RANDOM_SEED), boost::uniform_real<>(0,1));

    bool outlier;

    for(int sample_i = 0 ; sample_i < m; sample_i++) {
        TS sample;
        outlier = false;

        if(outlier_probab > 0 && vsample_uniform01() > 1.0 - outlier_probab)
            outlier = true;

        for(unsigned int l = 0; l < seed_series.values.size(); l++) {
            double v = seed_series.values[l];
            v+= vsample_normal();
            if(outlier)
                v+= vsample_normal_outlier();
            sample.values.push_back(v);
        }
        samples.push_back(sample);
    }

    return samples;


}



vector<TS> noisySamples3(TS seed_series, int m, double stddev) {

    //LOG <<  "m= " << m << " stddev= " << stddev << "\n";

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal_stddev(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev));

    vector<TS> samples;

    for(int sample_i = 0 ; sample_i < m; sample_i++) {
        TS sample;

        double stddev_sample = abs(vsample_normal_stddev());

        //LOG << "stddev_sample= " << stddev_sample << "\n";
        boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
                vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev_sample));

        for(unsigned int l = 0; l < seed_series.values.size(); l++) {
            double v = seed_series.values[l];
            v+= vsample_normal();
            sample.values.push_back(v);
        }
        samples.push_back(sample);
    }

    return samples;
}


void Dataset::update_counters() {

    N = utss.size();

    if(N > 0) {
        m = utss[0].samples.size();
        if(m > 0) {
            n = utss[0].samples[0].values.size();
        } else {
            n = 0;
        }
    } else {
        m  = 0;
        n = 0;
    }

    if(m > 0)
        w_min = 1.0/((double)m);
    else
        w_min = 0;
}


void Dataset::synthetic(int N, int m, int n, double stddev, double mean2,  double stddev2) {

    TS seed_series = walk(stddev, n);
    seed_series.normalize();

    for(int uts_i = 0; uts_i < N; uts_i++) {
        UTS uts(uts_i);
        uts.samples = noisySamples(seed_series, m, mean2 * (1+(double)uts_i), stddev2, 1.0);
        uts.update_counters();
        utss.push_back(uts);

        if(uts_i == 0) {
            vector<TS> uts_queries =noisySamples(seed_series, m, mean2 * (1+(double)uts_i), stddev2, 1.0);
            queries.insert(queries.end(), uts_queries.begin(), uts_queries.end());
        }
    }

    update_counters();
}


void Dataset::generate_table_normal(double mean, double stddev, vector<double> &table, unsigned long count) {

    table.resize(count);

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(mean, stddev));

    unsigned long size = 0;
    while(size < count) {
        table[size++] = vsample_normal();
    }

}

void Dataset::synthetic2(int N, int m, int n, double mean,  double stddev,
                         double outlier_probab, double outlier_stddev) {

    unsigned long mb = (N * m * n) * sizeof(double) /  ( 1024 * 1024);


    LOG << "Generating dataset .... expected memory size= " << mb << " Mb.\n";



    ElapsedTime time;
    time.begin();

    char pathname[1024];
    sprintf(pathname, "datasets/synthetic/synthetic_%d_%d_%d_%.2f_%.2f_%.2f_%.2f.dat", N, m, n, mean, stddev,outlier_probab, outlier_stddev);

    struct stat buf;
    int ret = stat( pathname, &buf );
    bool exists = (ret == 0);

    if(exists) {
        LOG << "File alraedy exists! loading.... " << pathname << "\n";
        this->loadFromFile(pathname);

        LOG << "Done. t=  " << time.t << " ms (" << time.t / (1000*60) << " mins)\n";
        return;

    }


    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(mean, stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal_outlier(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, outlier_stddev));

    boost::variate_generator<RANDOM_GENERATOR, boost::uniform_real<> >
            vsample_uniform01(RANDOM_GENERATOR(RANDOM_SEED), boost::uniform_real<>(0,1));

    vector<double> normal_samples;

    unsigned long table_normal_size = m*n*(N*0.01+1); // number of doubles in one UTS.
    generate_table_normal(mean, stddev, normal_samples, table_normal_size);
    unsigned long pos = 0;


    utss.resize(N, UTS(0,m,n));

    for(int uts_i = 0; uts_i < N; uts_i++) {
        TS seed_series(n); // construction of seed series.
        {
            double prev = vsample_normal();
            double point;

            for(int t = 0 ; t < n ; t++) {
                point = prev + vsample_normal();
                seed_series.values[t] = prev + vsample_normal();
                prev = point;
            }

            seed_series.normalize();
        }

        //        vector<TS> samples; // = = noisySamples2(seed_series, m, mean, stddev, outlier_probab, outlier_stddev);
        {

            //          samples.resize(m, TS(n)); // allocating all the required space at once.

            bool outlier;
            double v;

            for(int sample_i = 0 ; sample_i < m; sample_i++) {
                outlier = false;

                if(outlier_probab > 0 && vsample_uniform01() > 1.0 - outlier_probab)
                    outlier = true;

                for(int t = 0 ; t < n ; t++) {
                    v = seed_series.values[t];
                    //LOG << "XXX " << (pos++)%table_normal_size << "\n";
                    v+= normal_samples[(pos++)%table_normal_size]; //  vsample_normal();
                    if(outlier)
                        v+= vsample_normal_outlier();
                    utss[uts_i].samples[sample_i].values[t] = v;
                }

            }

        }

        utss[uts_i].update_counters();
        utss[uts_i].normalize();

        utss[uts_i].id = uts_i;


    }

    update_counters();

    time.end();

    LOG << "Done. t=  " << time.t << " ms (" << time.t / (1000*60) << " mins)\n";

    LOG << "Dumping to file ....\n";
    this->dumpToFile(pathname);

    LOG << "Done.\n";


}





void Dataset::synthetic3(int N, int m, int n,  double stddev, double a, double b, vector<TS> &seeds) {

    unsigned long mb = (N * m * n) * sizeof(double) /  ( 1024 * 1024);

    LOG << "Generating dataset .... expected memory size= " << mb << " Mb.\n";

    ElapsedTime time;
    time.begin();

    char pathname[1024];
    sprintf(pathname, "datasets/synthetic/synthetic3_%d_%d_%d_%.2f_%.2f.dat", N, m, n, 0.0, stddev);

    struct stat buf;
    int ret = stat( pathname, &buf );
    bool exists = (ret == 0);


    if(0 && exists) {
        LOG << "File alraedy exists! loading.... " << pathname << "\n";
        this->loadFromFile(pathname);
        time.end(false);
        LOG << "Done. t=  " << time.t << " ms (" << time.t / (1000*60) << " mins)\n";
        return;
    }

    boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
            vsample_normal(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev));

    utss.resize(N, UTS(0,m,n));

    TS seed_series(n); // construction of seed series.
    {
        double prev = vsample_normal();
        double point;

        for(int t = 0 ; t < n ; t++) {
            point = prev + vsample_normal();
            seed_series.values[t] = point;
            prev = point;
        }

        seed_series.normalize();
    }

    seeds.push_back(seed_series);

    // now, N uncertain series on seed_series varying stddev. nn is the one with lowest stddev.


    for(int uts_i = 0; uts_i < N; uts_i++) {

      double  stddev2 = stddev*a+ stddev*b*uts_i;

        //LOG << "xxx stddev2 " << stddev2 << "\n";

        boost::variate_generator<RANDOM_GENERATOR, boost::normal_distribution<> >
                vsample_normal2(RANDOM_GENERATOR(RANDOM_SEED), boost::normal_distribution<>(0, stddev2));

        utss[uts_i].class_label = uts_i;

        {
            double v;

            for(int sample_i = 0 ; sample_i < m; sample_i++) {
                for(int t = 0 ; t < n ; t++) {
                    v = seed_series.values[t];
                    v+= vsample_normal2();
                    utss[uts_i].samples[sample_i].values[t] = v;
                }

            }

        }

        utss[uts_i].update_counters();
        //utss[uts_i].normalize();

        utss[uts_i].id = uts_i;


    }

    update_counters();

    time.end();

    LOG << "Done. t=  " << time.t << " ms (" << time.t / (1000*60) << " mins)\n";

    LOG << "Dumping to file ....\n";
    this->dumpToFile(pathname);

    LOG << "Done.\n";


}

void Dataset::dumpToFile(const char *pathname) const {

    LOG << "Dumping to file " << pathname << "\n";
    ofstream fout;
    fout.open(pathname, ios::binary);

    if (!fout.is_open()) {
        FATAL("unable to open file " << pathname);
    }

    //LOG << "AAA N= " << N << " m= " << m << " n= " << n << "\n";

    //    LOG << "size of three ints: " << sizeof(int) * 3 << "\n";
    //    LOG << "size of one double: " << sizeof(double) << "\n";

    fout.write(reinterpret_cast<const char *>(&N), sizeof(int));
    fout.write(reinterpret_cast<const char *>(&m), sizeof(int));
    fout.write(reinterpret_cast<const char *>(&n), sizeof(int));

    for(int uts_i = 0 ; uts_i < N; uts_i++)
        utss[uts_i].dumpToFile(fout);

    fout.close();

}


void Dataset::loadFromFileUCR(const char *pathname) {

    string line;

    //LOG << "Reading UCR dataset: " << pathname << "\n";

    ifstream f(pathname);
    assert(f.is_open());

    if (!f.is_open()) {
        FATAL("unable to open file " << pathname);
    }


    int uts_i = 0;

    while(getline(f, line)) {
        trim(line);

        if(line.size() == 0)
            continue;

        // we have a series to load.. and it starts with its class label (as float, to be converted to int.)

        UTS uts(uts_i++);
        TS sample;

        sample.loadFromString(line);
        uts.class_label = sample.values[0];
        sample.values.erase(sample.values.begin());
        uts.samples.push_back(sample);
        uts.update_counters();

        uts.normalize();

        utss.push_back(uts);

    }

    update_counters();

    f.close();


}




TS Dataset::loadQueryFromFileBin(const char *pathname, int  size, int length) {

    TS ts;

    //LOG << "Reading UCR dataset: " << pathname << "\n";

    ifstream f(pathname);
    assert(f.is_open());

    if (!f.is_open()) {
        FATAL("unable to open file " << pathname);
    }


    int uts_i = 0;

    for (int i=0; i< size ; ++i)
    {
        ts.loadFromFile(f, length);
    }

    f.close();

    return ts;
}


void Dataset::loadFromFileBin(const char *pathname, int  size, int length) {

    string line;

    //LOG << "Reading UCR dataset: " << pathname << "\n";

    ifstream f(pathname);
    assert(f.is_open());

    if (!f.is_open()) {
        FATAL("unable to open file " << pathname);
    }


    int uts_i = 0;
    TS sample;
    
    for (int i=0; i< size ; ++i)
    {
        UTS uts(uts_i++);

        sample.loadFromFile(f, length);
        uts.samples.push_back(sample);
        uts.update_counters();

        //uts.normalize();

        utss.push_back(uts);

    }

    update_counters();

    f.close();


}

void Dataset::addPerturbation(int m, double stddev, double outlier_probab,
                              double outlier_stddev) {

    // we assume that each UTS is one single seed series.
    for(int uts_i = 0 ; uts_i < N ; uts_i++) {
        assert(utss[uts_i].samples.size() == 1);

        UTS uts(uts_i);

        int class_label = utss[uts_i].class_label;

        uts.samples = noisySamples3(utss[uts_i].samples[0], m, stddev);

        //  uts.samples = noisySamples2(utss[uts_i].samples[0], m, 0, stddev, outlier_probab, outlier_stddev);

        //uts.samples = noisySamples(utss[uts_i].samples[0], m, 0, stddev, 1);

        uts.update_counters();
        uts.normalize();
        uts.class_label = class_label;
        utss[uts_i] = uts;
    }

    update_counters(); // perturbation_m changes m.


}


void Dataset::loadFromFile(const char *pathname) {

    cout << "Loading UTS dataset: " << pathname << "\n";

    ifstream f(pathname, ios::binary);
    assert(f.is_open());

    if (!f.is_open()) {
        FATAL("unable to open file " << pathname);
    }


    f.read(reinterpret_cast<char *>(&N), sizeof(int));
    f.read(reinterpret_cast<char *>(&m), sizeof(int));
    f.read(reinterpret_cast<char *>(&n), sizeof(int));

    // LOG << "BBB N= " << N << " m= " << m << " n= " << n << "\n";

    for(int uts_i = 0 ; uts_i < N; uts_i++) {
        UTS uts(uts_i);
        uts.loadFromFile(f, m, n);
        utss.push_back(uts);
    }

    // randomzing order of uncertain series.
    random_shuffle(utss.begin(), utss.end());
    for(int uts_i = 0 ; uts_i < N; uts_i++)
        utss[uts_i].id = uts_i;
    // done.



    f.get(); // try to read one more byte, it must set error bit.
    assert(f.eof()); // we check that the error bit is set.

    update_counters();

    f.close();

}


void Dataset::loadFromFileTXT(const char *pathname) {

    cout << "Loading UTS dataset: " << pathname << "\n";

    ifstream f(pathname);
    assert(f.is_open());

    while(!f.eof()) {
        int id = utss.size();
        UTS uts(id);
        uts.loadFromFileTXT(f);
        if(uts.m >0) // if the uts is not empty, i.e. contains samples
            utss.push_back(uts);
    }

    f.close();

    update_counters();

    LOG << "Dataset loaded: N=" << N << " m=" << m << " n=" << n << "\n";

}

TS & Dataset::randomSample() {

    boost::variate_generator< RANDOM_GENERATOR, boost::uniform_int<> >
            randomly(RANDOM_GENERATOR(RANDOM_SEED), boost::uniform_int<>(0,queries.size()-1));
    //LOG << queries[randomly()] << "\n";
    return queries[randomly()];
    //return queries[randomly()];
    //return utss[randomly() % N].samples[randomly() % m];

}


std::ostream& operator << (std::ostream& out, const Dataset& dataset) {

    LOG << "Dataset: N=" << dataset.N << " m=" << dataset.m << " n=" << dataset.n << "\n";

    for(unsigned int i = 0; i < dataset.utss.size(); i++) {
        out << dataset.utss[i];
    }

    return out;
}



