#include <iostream>
#include <math.h>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

//#include <boost/filesystem.hpp>

#include "utils.hpp"

using namespace std;

void z_normalize(double *ts, int size) {
    double sum = 0;
    int i;
    for(i = 0; i < size; ++i)
        sum += ts[i];
    double mean = sum / size;

    double sq_diff_sum = 0;
    for(i = 0; i < size; ++i) {
       double diff = ts[i] - mean;
       sq_diff_sum += diff * diff;
    }
    double variance = sq_diff_sum / (size-1);
    double std = sqrt(variance);

    for (i = 0; i < size; i++)
	{ 
	    ts[i] = (ts[i] - mean) / std;
	}   
}

int dirExists(const char* path)
{
    struct stat info;

    if(stat( path, &info ) != 0){
      mkdir(path, S_IRWXU);
      return 0;
    }
    else if(info.st_mode & S_IFDIR){
      
      return 1;
    }
    else
        return 0;
}

