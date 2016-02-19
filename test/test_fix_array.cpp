#include <iostream>

#include "common/data_struct/fix_array.h"

using namespace std;

class A
{
public:
    A () {cout << "construct" << endl;}
    virtual ~A () {cout << "destroy" << endl;}
};


void test_fix_array() {
    int idx = 1;
    ef::FixArray<int> arr(5);
    cout << "push # " << idx++  << " ret: " << arr.Push(1) << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(2) << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(3) << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(4) << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(5) << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(6) << " size: "; cout  << arr.Size() << endl;


    idx = 1;
    cout << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(4) << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(0) << " size: "; cout << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(1) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(4) << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(2) << " size: "; cout << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(3) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(4) << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(4) << " size: "; cout << arr.Size() << endl;

    idx = 1;
    cout << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(4) << " size: " << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(5) << " size: " << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push(6) << " size: " << arr.Size() << endl;
}

void test_obj()
{
    ef::FixArray<A> arr(10);
    cout << "push" << endl;
    arr.Push(A());
    cout << "pop" << endl;
    arr.Pop(0);
    cout << "done" << endl;
}

int main(int argc, char *argv[])
{
    test_fix_array();
    test_obj();
    return 0;
}
