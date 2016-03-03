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
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;


    idx = 1;
    cout << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(0) << " size: "; cout << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(1) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(2) << " size: "; cout << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(3) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop # " << idx++  << " ret: " << arr.Pop(4) << " size: "; cout << arr.Size() << endl;

    idx = 1;
    cout << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << idx++  << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
}

void test_fix_array_list() {
    int idx = 1;
    int head1, tail1;
    int head2, tail2;
    head1 = tail1 = -1;
    head2 = tail2 = -1;
    int ret = -1;

    ef::FixArray<int> arr(5);
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push()) << " size: "; cout  << arr.Size() << endl;
    head1 = tail1 = ret;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push()) << " size: "; cout  << arr.Size() << endl;
    head2 = tail2 = ret;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;

    cout << "list1" << endl;
    int t = head1;
    while(t >= 0) {
        cout << t << ", ";
        t = arr.GetNext(t);
    }
    cout << endl;

    t = head2;
    cout << "list2" << endl;
    while(t >= 0) {
        cout << t << ", ";
        t = arr.GetNext(t);
    }
    cout << endl;

    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret > 0 ? ret : tail2;
    arr.PopList(head1);
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # " << idx++  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;

    t = head2;
    cout << "list2" << endl;
    while(t >= 0) {
        cout << t << ", ";
        t = arr.GetNext(t);
    }
    cout << endl;
}

void test_obj()
{
    ef::FixArray<A> arr(10);
    cout << "push" << endl;
    arr.Push();
    cout << "pop" << endl;
    arr.Pop(0);
    cout << "done" << endl;
}

int main(int argc, char *argv[])
{
    test_fix_array();
    test_obj();
    test_fix_array_list();
    return 0;
}
