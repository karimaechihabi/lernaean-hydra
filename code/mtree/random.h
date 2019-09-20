#ifndef RANDOM_H
#define RANDOM_H

#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/uniform.hpp>



class Random
{
public:
    Random();
    static int seed;
    static boost::rand48 rng;

    static boost::rand48 getInstance();
    static double getSample(double a, double b);
    static bool getTrue(double P);



};



#endif // RANDOM_H
