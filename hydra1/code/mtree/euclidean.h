#ifndef EUCLIDEAN_H
#define EUCLIDEAN_H

#include "dataset.h"




class Euclidean
{

private:

    const Dataset &dataset; // dataset reference


public:

    Euclidean(const Dataset &dataset);
    int classify(const TS &query);
    int classify_bad(const TS &query);
    std::vector< std::pair<int, double> > eval(const TS &query, int K);
    int classify_uema(const TS &query);

};

#endif // EUCLIDEAN_H
