#include <iostream>
#include <thread>
#include <functional>
#include <vector>

#include "common/event_notifier.h"
using namespace std;

ef::EventNotifier ev_thread;

// 读写线程个数
const int nread = 20;
const int nwrite= 50;

// 一个线程读写次数
const int num = 1000;

// 线程读写结果
int read_res[nread][num] = {{0}};
int read_suc_num[nread] = {0};

int write_res[nwrite][num] = {{ 0 }};
int write_suc_num[nwrite] = {0};

void thread_func(int id, bool read) {
    for (int i=0; i<num; i++) {
        if (read) {
            read_res[id][i] = ev_thread.GetEvent();
            if (read_res[id][i] > 0) {
                read_suc_num[id] += read_res[id][i];
            }
        } else {
            write_res[id][i] = ev_thread.Notify(1);
            if (write_res[id][i])
                ++ write_suc_num[id];
        }
    }
}

// 测试一读一写两个线程同时操作是否冲突
void TestReadWriteConflicate() {
    ev_thread.Initialize(false, false);

    vector<thread*> r, w;

    for (int i = 0; i < nread; ++i) {
        r.push_back(new thread(bind(thread_func, i, true)));
    }

    for (int i = 0; i < nwrite; ++i) {
        w.push_back(new thread(bind(thread_func, i, false)));
    }

    for (int i = 0; i < nread; ++i) {
        r[i]->join();
        delete r[i];
    }

    for (int i = 0; i < nwrite; ++i) {
        w[i]->join();
        delete w[i];
    }

    // output res
    int write_sum = 0;
    for (int i = 0; i < nwrite; ++i) {
        cout << "write thread " << i << " write " << write_suc_num[i] << " event " << endl;
        write_sum += write_suc_num[i];
    }

    int read_sum = 0;
    for (int i = 0; i < nread; ++i) {
        cout << "read thread " << i << " read " << read_suc_num[i] << " event " << endl;
        read_sum += read_suc_num[i];
    }

    int n = 0;
    while (ev_thread.GetEvent() >= 0) ++n;
    cout << "thread run finish, read " << n << " notify event" << endl;

    cout << endl;
    cout << "thread read total:\t" << read_sum << endl;
    cout << "thread not read total:\t" << n << endl;
    cout << "thread write total:\t" << write_sum << endl;
    if (read_sum + n != write_sum) {
        cout << "read and write conflicate~!!!!!! " << endl;;
    } else {
        cout << "read and write done $$$$" << endl;;
    }
    cout << endl << endl;
}

void TestEventNotifier(bool blocked, bool edge_trigger) {
    ef::EventNotifier ev;
    ev.Initialize(blocked, edge_trigger);
    cout << "ev with block " << blocked << " and level trigger " << edge_trigger << " test " << endl;
    ev.Notify(12);

    for (int i = 0; i < 15; ++i) {
        cout << "loop " << i << " get events num " <<ev.GetEvent() << endl;
    }
    cout << endl << endl << endl;
}

int main(int argc, char *argv[])
{
    TestReadWriteConflicate();
    TestEventNotifier(false, false);
    TestEventNotifier(false, true);
    TestEventNotifier(true, false);
    return 0;
}
