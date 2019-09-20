#ifndef UTS_H
#define UTS_H

#include "ts.h"

#include <vector>


class UTS
{
public:
    UTS(int);

    UTS(int,int,int);

    void loadFromFile(std::ifstream &f, int m, int n);
    void loadFromFileTXT(std::ifstream &ifs);
    void dumpToFile(std::ofstream &ofs) const;
    friend std::ostream& operator << (std::ostream&, const UTS&);
    std::vector<ts_type> dist(const TS &ts) const;
    ts_type dist_mean(const TS &ts) const;
    ts_type dist_uema(const TS &ts) const;
    TS samples_stddev() const;

    void normalize();

    TS lb() const;
    TS ub() const;
    TS avg() const;
    TS avg(std::vector<int> samples) const;

    std::vector<int> range_search(TS &query, ts_type dist_threshold);

    TS random_mix_pivot() const;
    TS random_mvp_pivot() const;

    void update_counters();

    int id;
    int m;
    int n;
    int class_label;

    std::vector<TS> samples;



};

#endif // UTS_H
