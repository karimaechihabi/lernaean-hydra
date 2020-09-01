#include "randomvariable.h"


RandomVariable::RandomVariable() {

}


RandomVariable::RandomVariable(int distribution, double groundtruth, double observation, double stddev) {
this->distribution = distribution;
this->groundtruth = groundtruth;
this->observation = observation;
this->stddev = stddev;
}
