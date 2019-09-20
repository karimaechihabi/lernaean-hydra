#ifndef TS_H
#define TS_H

#include "common.h"
#include <vector>


class TS;

class TS
{
public:
    TS();
    TS(int n);
    TS(const char *pathname);

    friend std::ostream& operator << (std::ostream&, const TS&);

    ts_type *ts_types();
    void loadFromFile(std::ifstream &f, int n);
    void normalize();
    void loadFromString(std::string &s);
    void dumpToFile(std::ofstream &ofs) const;
    ts_type dist(const TS &ts) const;
    ts_type dist(const TS &ts, ts_type early_abandoning_threshold) const;
    ts_type dist2(const TS &ts, ts_type early_abandoning_threshold) const;
    ts_type dist2cut(const TS &ts, ts_type early_abandoning_threshold, int s) const;
    std::vector<ts_type> dists(const std::vector<TS> &tss) const;

    TS avg(const TS &ts) const;
    TS reversed() const;
    TS lb(const TS &ts) const;
    TS ub(const TS &ts) const;

    std::vector<ts_type> values;


};

#endif // TS_H
