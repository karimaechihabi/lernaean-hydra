#ifndef BASELINE_H
#define BASELINE_H

#include "dataset.h"

typedef std::pair<int,ts_type> intts_type;

typedef std::vector< std::vector<ts_type> > Dists;


class Baseline
{

private:

    Dists eval_dists(const TS &query);
    Dists dists;


public:

    Baseline(const Dataset &dataset);
    std::vector< std::pair<int,ts_type> > eval(const TS &query, int K);

    ts_type PNN(const TS &query, int uts_i);
    std::vector< std::pair<int,ts_type> > PkNN(const TS &query, int K);

    const Dataset &dataset;


};

#endif // BASELINE_H
