/**
 ****************************************************************************
 ** Copyright (C) 2012 Michele Dallachiesa
 ** All rights reserved.
 **
 ** Warning: This program is protected by copyright law and international
 ** treaties. Unauthorized reproduction and distribution of this program,
 ** or any portion of it, may result in severe civil and criminal penalties,
 ** and will be persecuted to the maximum extent possible under the law.
 ****************************************************************************
 ** Author: Michele Dallachiesa <dallachiesa@disi.unitn.it>
 ****************************************************************************
 **/

#ifndef COMMON_H
#define COMMON_H


#include <iostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <limits>

#define RANDOM_SEED ++random_seed // 2 //time(0)
extern int random_seed;
typedef float ts_type;

#define OVERLAP(min1, max1, min2, max2) MAX(0, MIN(max1, max2) - MAX(min1, min2))

#define WITHIN(x,a,b) (a <= x && x <= b)
#define WITHIN_EXCLUDED(x,a,b) (a < x && x < b)

#define FATAL(x) do {cerr << "Fatal error at " << __FILE__ << ":" << __FUNCTION__ <<":" << __LINE__ <<": "<< x << "; exit forced.\n"; exit(1);}while(0)
#define SAFE_FREE(x) do { if (x) { free(x); x = NULL; }}while(0)

#define MAX3(x,y,z) MAX(x,MAX(y,z))

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#define POW2(x) ((x)*(x)) // pow(x, 2.0) // ((x)*(x))

#define ABS(x) abs(x) // ((x) < 0 ? (-x) : (x))

extern int lineOutput;

//#define LOG std::cout << "LOG" << lineOutput++  << ": "
#define LOG std::cout << "% "
#define LOGN std::cout << "\n"
#define LOGL std::cout

//(static_cast<float>(clock()) / CLOCKS_PER_SEC)

namespace std{

template <typename A, typename B>

ostream& operator << (ostream& out, const pair<A, B>& p) {
  return out << "(" << p.first << ", " << p.second << ")";
}

template<typename _Tp, typename _Alloc >
std::ostream& operator << (ostream& out, const vector<_Tp, _Alloc>& vec) {
  out << '[';
  for (typename vector<_Tp, _Alloc>::const_iterator i = vec.begin();
      i != vec.end(); i++) {
    if (i != vec.begin())
      out << ", ";
    out << *i;
  }
  out << ']';
  return out;
}

}

typedef std::pair<int,double> intdouble;
typedef std::pair<double,double> doubledouble;
typedef std::pair<int,int> intint;


#endif
