#ifndef MAIN_H
#define MAIN_H

#include <set>


typedef struct
{

    int nRuns;
    std::set<int> experimentIds;
    int mspknn_func_C;
    int mspknn_func_R;
    bool generate_perturbed_datasets;
    char *datasetname;
    bool dust_gen_lookup_tables;
}
OPT;




#endif // MAIN_H
