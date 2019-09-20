#ifndef RANDOMVARIABLE_H
#define RANDOMVARIABLE_H

#include <iostream>
#include <ostream>
#include <string>

using namespace std;

#define RANDVAR_UNIFORM 1
#define RANDVAR_NORMAL 2
#define RANDVAR_EXP 3

class RandomVariable
{
public:
    RandomVariable();
    RandomVariable(int distribution, double groundtruth, double observation, double stddev);
    int distribution;
    double stddev;
    double observation;
    double groundtruth;

    operator double() { return observation; }

    RandomVariable & operator= (const double observation) {
        this->observation = observation;
        return *this;
    }

};



#endif // RANDOMVARIABLE_H
