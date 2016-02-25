/*
 * Author: number
 * Date  : Feb 25, 2016
 */

#ifndef __COMMON_PERFORMANCE_TEST_H_H___
#define __COMMON_PERFORMANCE_TEST_H_H___

#include <sys/time.h>

namespace ef {

class PerformanceTest {
public:
    PerformanceTest(int times);
    virtual ~PerformanceTest();

private:
    timeval _begin_time;
    timeval _end_time;
    int _times;
};

} /* namespace ef */

#endif /* __COMMON_PERFORMANCE_TEST_H__ */
