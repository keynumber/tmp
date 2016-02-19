#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <thread>

#include <unistd.h>

#include "common/rc_buf.h"

using namespace std;

const int buflen = 1024;

ef::RcBuf thread_queue[buflen];
int size = 0;

void reciever() {
    while(size == 0) {
        cout << "receiver: wait message" << endl;
        sleep(1);
    }

    ef::RcBuf rc = thread_queue[0];

    cout << "receiver: thread get message offset: " << rc.offset << endl;
    cout << "receiver: thread get message length: " << rc.len << endl;
    cout << "receiver: thread get message: " << rc.buf + rc.offset << endl;

    rc.Release();
}

void sender() {
    ef::RcBuf rc(buflen);
    memset(rc.buf, 0, rc.len);
    const char * role = "sender: ";
    const char * message = "this is message from sender";
    strcpy(rc.buf, role);
    strcat(rc.buf, message);

    cout << "sender: send message offset: " << strlen(role) << endl;
    cout << "sender: send message length: " << strlen(message) << endl;
    cout << "sender: send message: " << message << endl;
    rc.offset = strlen(role);
    rc.len = strlen(message);
    thread_queue[0] = rc;
    size = 1;

    sleep(2);

    rc.Release();
}

int main(int argc, char *argv[])
{
    thread t1(reciever);
    sleep(2);
    thread t2(sender);

    t1.join();
    t2.join();
    // std::list<ef::RcBuf> buf_list;
    // cout << "create" << endl;
    // ef::RcBuf rcbuf(10);
    // cout << "push_back" << endl;
    // buf_list.push_back(rcbuf);
    // cout << "release" << endl;
    // rcbuf.Release();
    // cout << "get out" << endl;
    // ef::RcBuf tmp = *buf_list.begin();
    // cout << "clear" << endl;
    // buf_list.clear();
    // cout << "done" << endl;
    return 0;
}
