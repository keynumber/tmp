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

using namespace std;

struct PacketHeader {
    int length;         // length为包请求内容大小,不包括包头大小
    unsigned int request_id;
};

#define MAX_EVENTS 1000
#define BUFFER_SIZE 1000
#define EPOLLEN 1018

uint32_t suc = 0;
uint32_t fail = 0;

int generator_sendbuf(char *buf, int len)
{
    const char * str[] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    int str_len = sizeof(str)/sizeof(char*);

    memset(buf, 0, len);
    PacketHeader * header = (PacketHeader*)buf;
    buf += sizeof(PacketHeader);
    random_device rd;
    for (int i=0; i< 3; i++) {
        int t = rd() % str_len;
        strcat(buf, str[t]);
        strcat(buf, ">");
    }

    int content_len = strlen(buf);
    header->length = content_len;
    header->request_id = rd();
    cout << "content_len: " << header->length << ", request_id: " << header->request_id << ", ";
    cout << "content: " << buf << endl;
    return sizeof(PacketHeader) + strlen(buf);
}

int write_to_server(int fd) {
    const int buflen = 10240;
    char buf[buflen];

    int idx = 0;
    for (int i=0; i<10; ++i) {
        idx += generator_sendbuf(buf + idx, buflen - idx);
    }

    int send_len = 0;
    int total_send = send_len;
    while (true) {
        cin >> send_len;
        if (send_len < 0)
            break;
        cout << "offset: " << total_send << endl;
        int ret = write(fd, buf + total_send, send_len);
        if (ret != send_len) {
            perror("write failed");
            return -1;
        }
        total_send += send_len;
    }
    return 0;
}

int visit_client()
{
    struct sockaddr_in conn_addr;
    int len = sizeof(struct sockaddr_in);

    int conn = socket(AF_INET, SOCK_STREAM, 0);
    if (conn == -1) {
        perror("socket error\n");
        return -1;
    }

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(12345);
    conn_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(conn, (struct sockaddr*)&conn_addr, len) < 0) {
        perror("connect error: ");
        close(conn);
        return -1;
    }
    int ret = write_to_server(conn);
    close(conn);
    return ret;
}

int main(int argc, char * argv[])
{
    int n = atoi(argv[1]);
    printf("will visit server %d times\n", n);
    for (int i=0; i<n; ++i) {
        if (visit_client() < 0) {
            fail++;
        } else {
            suc ++;
        }
    }
    printf("suc: %d\nfail: %d\n", suc, fail);
    return 0;
}
