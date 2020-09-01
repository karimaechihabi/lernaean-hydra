#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include "uts.h"
 #include <assert.h>

class Dataset
{
public:
    Dataset();
    Dataset(const char *pathname);


    Dataset(UTS &uts);

    void generate_table_normal(double mean, double stddev, std::vector<double> &table, unsigned long count);

    void synthetic(int N, int m, int n, double stddev, double mean2,  double stddev2);
    void synthetic2(int N, int m, int n, double mean,  double stddev, double outlier_probab, double outlier_stddev);
    void synthetic3(int N, int m, int n,  double stddev, double a, double b, std::vector<TS> &seeds);

    void loadFromFileTXT(const char *pathname);
    void loadFromFile(const char *pathname);
    
    void loadFromFileBin(const char *pathname, int  size, int length);
    TS  loadQueryFromFileBin(const char *pathname, int  size, int length);

  friend std::ostream& operator << (std::ostream&, const Dataset&);

    void dumpToFile(const char *pathname) const;

    TS walk(double stddev, int n);
    TS walk();

    TS & randomSample();

    void loadFromFileUCR(const char *pathname);
    void addPerturbation(int m, double stddev, double outlier_probab,double outlier_stddev);

    void retain(std::vector<int> &candidates);

    void update_counters();
    std::vector<UTS> utss;

    const UTS& operator[] (int uts_i) {
        assert(uts_i >= 0 && (unsigned int)uts_i < utss.size());
        return utss[uts_i];
    }

    std::vector<TS> queries;


    int N;
    int m;
    int n;
    double w_min;


};

#endif // DATASET_H
