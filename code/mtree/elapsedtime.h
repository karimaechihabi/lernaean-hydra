#ifndef ELAPSEDTIME_H
#define ELAPSEDTIME_H

#include <sys/time.h>
#include <sys/resource.h>

#ifndef TIMEVAL_SUB
#define TIMEVAL_SUB(a, b, result) \
do { \
 (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;          \
 (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;       \
 if ((result)->tv_usec < 0) {                           \
     --(result)->tv_sec;                                \
  (result)->tv_usec += 1000000;                         \
     }                                                  \
} while (0)
#endif //TIMEVAL_SUB

class ElapsedTime
{
public:
    ElapsedTime();

    void begin();
    unsigned long end(bool stop);
    unsigned long end();
    unsigned long get();

    unsigned long t;
    struct rusage r;

    bool active;

};

#endif // ELAPSEDTIME_H
