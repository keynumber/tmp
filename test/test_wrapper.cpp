#include <iostream>

#include "common/wrapper.h"

using namespace std;

int main(int argc, char *argv[])
{
    std::string str = ef::safe_strerror(EINVAL);
    cout << str << endl;
    return 0;
}
