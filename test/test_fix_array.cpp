#include <iostream>

#include "common/data_struct/fix_array.h"

using namespace std;


void test_fix_array() {
    cout << "test basic fix array" << endl;
    ef::FixArray<int> arr(5);
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(0) << " size: "; cout << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(1) << " size: "; cout << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(2) << " size: "; cout << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(3) << " size: "; cout << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(4) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "Push # " << " ret: " << arr.Push() << " size: "; cout << arr.Size() << endl;
    cout << "Push # " << " ret: " << arr.Push() << " size: "; cout << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(0) << " size: "; cout << arr.Size() << endl;
    cout << "Push # " << " ret: " << arr.Push() << " size: "; cout << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: "; cout  << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(0) << " size: "; cout << arr.Size() << endl;
    cout << "pop  # " << " ret: " << arr.Pop(1) << " size: "; cout << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "push # " << " ret: " << arr.Push() << " size: " << arr.Size() << endl;
    cout << "test basic fix array done" << endl;
}

void test_fix_array_list() {
    cout << "test list fix array" << endl;
    int idx = 1;
    int head1, tail1;
    int head2, tail2;
    head1 = tail1 = -1;
    head2 = tail2 = -1;
    int ret = -1;

    ef::FixArray<int> arr(5);
    cout << "push # "  << " ret: " << (ret = arr.Push()) << " size: "; cout  << arr.Size() << endl;
    head1 = tail1 = ret;
    cout << "push # "  << " ret: " << (ret = arr.Push()) << " size: "; cout  << arr.Size() << endl;
    head2 = tail2 = ret;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
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

    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret > 0 ? ret : tail2;

    cout << "pop list1" << endl;
    arr.PopList(head1);

    cout << "push # "  << " ret: " << (ret = arr.Push(-1)) << " size: "; cout  << arr.Size() << endl;
    head1 = tail1 = ret;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;

    cout << "list1" << endl;
    t = head1;
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

    cout << "pop list1 and list2" << endl;
    arr.PopList(head1);
    arr.PopList(head2);

    cout << "push # "  << " ret: " << (ret = arr.Push(-1)) << " size: "; cout  << arr.Size() << endl;
    head1 = tail1 = ret;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail2)) << " size: "; cout  << arr.Size() << endl;
    tail2 = ret >= 0 ? ret : tail2;
    cout << "push # "  << " ret: " << (ret = arr.Push(tail1)) << " size: "; cout  << arr.Size() << endl;
    tail1 = ret >= 0 ? ret : tail1;
    cout << "test list fix array done" << endl;
}

int main(int argc, char *argv[])
{
    test_fix_array();
    test_fix_array_list();
    return 0;
}
