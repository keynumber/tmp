#include <sys/epoll.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <random>
#include <thread>
#include <algorithm>

#include "server/net_complete_func.h"

// #define USE_DEBUG
#ifdef USE_DEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

using namespace std;

const int max_thread_num = 200;
const unsigned short server_port = 12345;
const char * server_addr = "192.168.0.115";

const int request_num = 10;
const int content_len = 10;
char _content[request_num][content_len + 1];
const auto & content = _content;

void generator_content()
{
    memset(_content, 0, sizeof(_content));
    for (int i=0; i<request_num; ++i) {
        memset(_content[i], i+'a', content_len);
    }
}

int generator_sendbuf(char *buf, int len)
{
    memset(buf, 0, len);

    ef::PacketHeader * header = (ef::PacketHeader*)buf;
    random_device rd;
    int t = rd() % request_num;
    memcpy(header->payload, content[t], content_len);

    header->length = content_len;
    header->request_id = rd();
    return sizeof(ef::PacketHeader) + content_len;
}

int write_to_server(int fd, char *buf, int len) {
    int send_len = len;
    int total_send = 0;
    while (true) {
        int ret = write(fd, buf + total_send, send_len);
        if (ret < 0) {
            perror("write to server failed, errmsg: ");
            return -1;
        }
        total_send += send_len;
        if (total_send == send_len) {
            break;
        }
    }
    DEBUG("totoal send len %d\n", len);
    return 0;
}

int read_from_server(int clientfd, char *buf, int len)
{
    memset(buf, 0, len);

    int total_read = 0;
    while(true) {
        int ret = read(clientfd, buf+total_read, len-total_read);
        if (ret < 0) {
            perror("read from server failed, errmsg: ");
            return -1;
        }
        if (ret == 0) {
            perror("server close connetion");
            return -1;
        }

        total_read += ret;
        ret = ef::packet_len_func(buf, total_read);
        if (ret < 0) {
            DEBUG("packet_len_func error\n");
            break;
        } else if (ret >= total_read){
            return ret;
        }
    }
    return -1;
}

bool check_response(char *send, char * recv) {
    char * p = send;
    while (*p) {
        if (*p >= 'a' && *p <= 'z') {
            *p = *p - 'a' + 'A';
            p++;
        }
    }

    return strcmp(send, recv) == 0;
}

// return -1, request server failed
// return -2, response is not correct
// return 0, succeed
int short_conn_request_svr()
{
    struct sockaddr_in conn_addr;
    int len = sizeof(struct sockaddr_in);

    int conn = socket(AF_INET, SOCK_STREAM, 0);
    if (conn == -1) {
        perror("socket error: ");
        return -1;
    }

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(server_port);
    conn_addr.sin_addr.s_addr = inet_addr(server_addr);
    if (connect(conn, (struct sockaddr*)&conn_addr, len) < 0) {
        perror("connect error: ");
        close(conn);
        return -1;
    }

    const int buflen = 10240;
    char sendbuf[buflen];
    char recvbuf[buflen];
    ef::PacketHeader * send_header = (ef::PacketHeader*)sendbuf;

    len = generator_sendbuf(sendbuf, buflen);
    DEBUG("content_len: %d, request_id: %u, content: %s\n", send_header->length, send_header->request_id, send_header->payload);
    if (write_to_server(conn, sendbuf, len) < 0) {
        return -1;
    }
    if (read_from_server(conn, recvbuf, buflen) < 0) {
        return -1;
    }

    ef::PacketHeader * recv_header = (ef::PacketHeader*)recvbuf;
    DEBUG("read from server length %d\n", recv_header->length);
    DEBUG("content_len: %d, request_id: %u, content: %s\n", recv_header->length, recv_header->request_id, recv_header->payload);

    if (recv_header->request_id != send_header->request_id ||
        !check_response(send_header->payload, recv_header->payload)) {
        return -2;
    }

    close(conn);
    return 0;
}

int request_server_ntimes(int id, int n)
{
    int suc = 0;
    int fail = 0;
    int errrsp = 0;
    for (int i=0; i<n; ++i) {
        int ret = short_conn_request_svr();
        if (ret == -1) {
            fail++;
        } else if (ret == -2) {
            errrsp++;
        } else {
            suc ++;
        }
    }
    printf("thread id: %d, suc: %d, fail: %d, errrsp: %d\n", id, suc, fail, errrsp);
    return 0;
}

int main(int argc, char * argv[])
{
    if (argc < 3) {
        printf("usage %s <thread num> <query times per thread>\n", argv[0]);
        return -1;
    }

    int nthread = atoi(argv[1]);
    int ntimes = atoi(argv[2]);

    generator_content();

    vector<thread> vec;
    for (int i=0; i<nthread; ++i) {
        vec.push_back(thread(request_server_ntimes, i, ntimes));
    }

    for (auto &it : vec) {
        it.join();
    }
    return 0;
}
