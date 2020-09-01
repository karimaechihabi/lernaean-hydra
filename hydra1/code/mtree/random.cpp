#include "random.h"
#include "common.h"

using namespace std;
using namespace boost;

int Random::seed = 0;
rand48 Random::rng;


Random::Random()
{
}


rand48 Random::getInstance() {

    if(seed == 0) {
        seed = time(0);
        seed = 123;
    } else {
        seed++;
    }

    return rand48(seed);
}



double Random::getSample(double a, double b) {
    rand48 rng = getInstance();

    uniform_real<> ur(a,b);
    variate_generator<rand48&,uniform_real<> > drawSample(rng, ur);

    return drawSample();
}


bool Random::getTrue(double P) {
    return getSample(0,1) <= P;
}
