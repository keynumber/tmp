#include <iostream>

#include "common/data_struct/fix_queue.h"

using namespace std;

void TestPush()
{
    ef::FixQueue<int> q(5);

    cout << "empty ? " << q.empty() << endl;
    cout << "full? " << q.full() << endl;

    q.pop();
    q.pop();
    q.pop();
    q.pop();
    q.push(1);
    q.push(2);
    q.pop();
    q.push(3);
    q.push(4);
    q.pop();
    q.push(5);
    q.push(6);
    q.pop();
    q.push(7);
    q.pop();
    q.push(8);
    q.push(9);
    q.push(10);
    q.pop();
    q.push(11);

    cout << "empty ? " << q.empty() << endl;
    cout << "full? " << q.full() << endl;


    while(!q.empty()) {
        cout << q.front() << endl;
        q.pop();
    }
}

int main(int argc, char *argv[])
{
    TestPush();
    return 0;
}
