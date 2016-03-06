#include <iostream>

#include "common/data_struct/pool_list.h"

using namespace std;

void test_pool_list() {
    int idx = 1;
    ef::FixArray<int> pool(5);
    ef::PoolList<int> list;
    list.AttachPool(&pool);

    int * p = list.PushBack(); if (p) *p = 1;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 2;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 3;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 4;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 5;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 6;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;

    cout << "list size: " << list.Size() << " content: ";
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        idx = list.GetNext(idx);
    }
    cout << endl << endl;

    cout << "pop and push" << endl;
    idx = 1;
    cout << "Pop# " << idx++  << " ret: " << (list.PopFront(), 0) << " size: "; cout << list.Size() << endl;
    cout << "Pop# " << idx++  << " ret: " << (list.PopFront(), 0) << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 7;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 8;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 9;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;

    cout << "list size: " << list.Size() << " content: ";
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        idx = list.GetNext(idx);
    }
    cout << endl << endl;

    cout << "visit and pop" << endl;
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        list.PopFront();
        idx = list.GetHead();
    }
    cout << endl << endl;

    cout << "after visit and pop" << endl;
    cout << "list size: " << list.Size() << " content: ";
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        idx = list.GetNext(idx);
    }
    cout << endl << endl;

    cout << "push and pop" << endl;
    p = list.PushBack(); if (p) *p = 10;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 11;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 12;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 13;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    cout << "pop# " << idx++  << " ret: " << (list.PopFront(), 0) << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 14;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    cout << "pop# " << idx++  << " ret: " << (list.PopFront(), 0) << " size: "; cout << list.Size() << endl;
    cout << "pop# " << idx++  << " ret: " << (list.PopFront(), 0) << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 15;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 16;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 17;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;
    p = list.PushBack(); if (p) *p = 18;
    cout << "push # " << idx++  << " ret: " << p << " size: "; cout << list.Size() << endl;

    cout << "list size: " << list.Size() << " content: ";
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        idx = list.GetNext(idx);
    }
    cout << endl << endl;

    cout << "clear" << endl;
    list.Clear();
    cout << "list size: " << list.Size() << " content: ";
    idx = list.GetHead();
    while (idx >= 0) {
        cout << list[idx] << " ";
        idx = list.GetNext(idx);
    }
    cout << endl << endl;

    cout << "pool size: " << pool.Size() << endl;
}

int main(int argc, char *argv[])
{
    test_pool_list();
    return 0;
}
