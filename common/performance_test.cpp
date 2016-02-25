/*
 * Author: number
 * Date  : Feb 25, 2016
 */

#include "performance_test.h"

#include <stdio.h>

namespace ef {

PerformanceTest::PerformanceTest(int times)
{
    _times = times;
    gettimeofday(&_begin_time, nullptr);
}

PerformanceTest::~PerformanceTest()
{
    gettimeofday(&_end_time, nullptr);
    long long int  sec = _end_time.tv_sec - _begin_time.tv_sec;
    long long int  usec = _end_time.tv_usec - _begin_time.tv_usec;
    if (usec < 0) {
        --sec;
        usec += 1000*1000;
    }
    long long int  msec = usec / 1000;
    usec %= 1000;
    printf("execute %d times use total time %lld sec, %lld msec, %lld usec\n",
           _times, sec, msec, usec);

    long long int  average = ((_end_time.tv_sec - _begin_time.tv_sec)*1000*1000*1000 +
            (_end_time.tv_usec- _begin_time.tv_usec)*1000) / _times;
    printf("execute one times use everage time %lld usec, %lld msec, %lld usec, %lld nsec\n",
           average/(1000*1000*1000), (average%(1000*1000*1000))/(1000*1000),
           (average/1000)%1000, average%1000);
}

} /* namespace ef */
