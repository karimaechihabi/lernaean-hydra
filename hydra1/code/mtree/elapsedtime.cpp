#include "elapsedtime.h"
#include "common.h"

using namespace std;
using namespace boost;


ElapsedTime::ElapsedTime()
{
    active = false;
}


void ElapsedTime::begin() {
    assert(active == false);
    getrusage(RUSAGE_SELF, &r);
}


unsigned long ElapsedTime::end() {
    return end(true);
}

unsigned long ElapsedTime::get() {
    return end(false);
}

unsigned long ElapsedTime::end(bool stop) {

    struct rusage r2;
    getrusage(RUSAGE_SELF, &r2);

    struct timeval elapsed0, elapsed1;

    TIMEVAL_SUB(&r2.ru_utime, &r.ru_utime, &elapsed0);/* user CPU time used */
    TIMEVAL_SUB(&r2.ru_stime, &r.ru_stime, &elapsed1);/* system CPU time used */

    // tv_sec seconds
    // tv_usec microseconds

    unsigned long t = elapsed0.tv_sec * 1000 + elapsed0.tv_usec / 1000 +
                elapsed1.tv_sec * 1000 + elapsed1.tv_usec / 1000; // tt milliseconds (ms)

    this->t = t;
    if(stop) {
        active = false;
        return t;
    } else {
        return t;
    }

}





