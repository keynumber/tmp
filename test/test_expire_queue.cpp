#include <iostream>

#include "common/data_struct/expire_queue.h"

using namespace std;

void PrintQueue(const ef::ExpireQueue & queue) {
    int next = queue.GetHead();
    cout << "queue next: ";
    while (true) {
        cout << next << " ";
        if (next < 0) {
            break;
        }
        next = queue.GetNext(next);
    }
    cout << endl;

    int pre = queue.GetTail();
    cout << "queue pre : ";
    while (true) {
        cout << pre << " ";
        if (pre < 0) {
            break;
        }
        pre = queue.GetPre(pre);
    }
    cout << endl;
    cout << endl;
}

void Activate(ef::ExpireQueue & queue, int idx) {
    cout << "Activate " << idx << endl;
    queue.Activate(idx);
    PrintQueue(queue);
}

void Erase(ef::ExpireQueue & queue, int idx) {
    cout << "Erase " << idx << endl;
    queue.Erase(idx);
    PrintQueue(queue);
}

void TestActivate() {
    cout << "Test Activate" << endl;
    ef::ExpireQueue queue(10);
    Activate(queue, 5);
    Activate(queue, 1);
    Activate(queue, 3);
    Activate(queue, 1);
    Activate(queue, 1);
    Activate(queue, 4);
    Activate(queue, 0);
    Activate(queue, 2);
    Activate(queue, 5);
    Activate(queue, 1);
    Activate(queue, 2);
    Activate(queue, 5);
}

void TestErase() {
    cout << "Test Erase" << endl;
    ef::ExpireQueue queue(10);
    queue.Activate(5);
    queue.Activate(4);
    queue.Activate(3);
    queue.Activate(2);
    queue.Activate(1);
    queue.Activate(0);
    queue.Activate(-1);
    PrintQueue(queue);
    Erase(queue, 7);
    Erase(queue, 4);
    Erase(queue, 0);
    Erase(queue, 3);
    Erase(queue, 2);
    Erase(queue, 5);
    Erase(queue, 5);
    Erase(queue, 1);
}

int main(int argc, char *argv[])
{
    TestActivate();
    TestErase();
    return 0;
}
