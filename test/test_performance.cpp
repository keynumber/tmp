#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <functional>

#include "common/performance_test.h"

using namespace std;

int inc(int i)
{
    return i+1;
}

void TestFuncLambda()
{
    auto lambda = [](int i) {return i+1;};
    function<int(int)> func = inc;

    int n = 1000*1000*1000;
    {
        cout << "test c function" << endl;
        ef::PerformanceTest test(n);
        for (int i=0; i<n; i++)
            inc(i);
    }

    {
        cout << "test c++ std::function" << endl;
        ef::PerformanceTest test(n);
        for (int i=0; i<n; i++)
            func(i);
    }

    {
        cout << "test c++ lambda" << endl;
        ef::PerformanceTest test(n);
        for (int i=0; i<n; i++)
            lambda(i);
    }
}

int main(int argc, char *argv[])
{
    TestFuncLambda();
    return 0;
}
