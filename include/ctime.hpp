#ifndef CTIME_HPP
#define CTIME_HPP
#include <ctime>
#include "../src/inline_variables.hpp"
#include <time.h>
namespace backports{
    using ::timespec;
    //#undef timespec_get
    static inline int timespec_get(timespec*ts, int base) { return ::clock_gettime(base,ts);}
    #undef TIME_UTC
    #define TIME_UTC CLOCK_REALTIME
    enum:int{
    #ifdef CLOCK_PROCESS_CPUTIME_ID
    TIME_PROCESS=CLOCK_PROCESS_CPUTIME_ID,
    #endif
    #ifdef CLOCK_THREAD_CPUTIME_ID
    TIME_THREAD=CLOCK_THREAD_CPUTIME_ID,
    #endif
    #ifdef CLOCK_MONOTONIC
    TIME_MONOTONIC=CLOCK_MONOTONIC,
    #endif
    #ifdef CLOCK_BOOTTIME
    TIME_BOOT=CLOCK_BOOTTIME,
    #endif
    };
}// namespace backports
#endif // CTIME_HPP
